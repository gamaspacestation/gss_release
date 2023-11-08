// Copyright 2022 UNAmedia. All Rights Reserved.

#include "MixamoAnimationRootMotionSolver.h"

#include "MixamoToolkitPrivatePCH.h"

#include "MixamoToolkitPrivate.h"

#include "Editor.h"
#include "SMixamoToolkitWidget.h"
#include "Misc/MessageDialog.h"

#include "Animation/AnimSequence.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "FMixamoAnimationRetargetingModule"

void FMixamoAnimationRootMotionSolver::LaunchProcedureFlow(USkeleton* Skeleton)
{
    checkf(Skeleton != nullptr, TEXT("A reference skeleton must be specified."));
    checkf(CanExecuteProcedure(Skeleton), TEXT("Incompatible skeleton."));

    TSharedRef<SWindow> WidgetWindow = SNew(SWindow)
        .Title(LOCTEXT("FMixamoAnimationRootMotionSolver_AskUserForAnimations_WindowTitle", "Select animations"))
        .ClientSize(FVector2D(1000, 600))
        .SupportsMinimize(false)
        .SupportsMaximize(false)
        .HasCloseButton(false);

    TSharedRef<SRootMotionExtractionWidget> RootMotionExtractionWidget = SNew(SRootMotionExtractionWidget)
        .ReferenceSkeleton(Skeleton);

    WidgetWindow->SetContent(RootMotionExtractionWidget);

    GEditor->EditorAddModalWindow(WidgetWindow);

    UAnimSequence* SelectedAnimation = RootMotionExtractionWidget->GetSelectedAnimation();
    UAnimSequence* SelectedInPlaceAnimation = RootMotionExtractionWidget->GetSelectedInPlaceAnimation();

    if (!SelectedAnimation || !SelectedInPlaceAnimation)
        return;

    // check, with an heuristic, that the user has selected the right "IN PLACE" animation otherwise prompt a message box as warning
    const UAnimSequence* EstimatedInPlaceAnim = EstimateInPlaceAnimation(SelectedAnimation, SelectedInPlaceAnimation);
    if (EstimatedInPlaceAnim != SelectedInPlaceAnimation)
    {
        FText WarningText = LOCTEXT("SRootMotionExtractionWidget_InPlaceAnimWarning", "Warning: are you sure to have choose the right IN PLACE animation?");
        if (FMessageDialog::Open(EAppMsgType::YesNo, WarningText) == EAppReturnType::No)
            return;
    }

    static const FName NAME_AssetTools = "AssetTools";
    IAssetTools* AssetTools = &FModuleManager::GetModuleChecked<FAssetToolsModule>(NAME_AssetTools).Get();

    const FString ResultAnimationName = SelectedAnimation->GetName() + "_rootmotion";
    const FString PackagePath = FAssetData(SelectedAnimation).PackagePath.ToString();
    UAnimSequence* ResultAnimation = Cast<UAnimSequence>(AssetTools->DuplicateAsset(ResultAnimationName, PackagePath, SelectedAnimation));
    if (!ResultAnimation)
    {
        FMessageLog("LogMixamoToolkit").Error(FText::FromString(TEXT("Aborted: failed to duplicate the animation sequence.")));
        return;
    }

    if (ExecuteExtraction(ResultAnimation, SelectedInPlaceAnimation))
    {
        ResultAnimation->bEnableRootMotion = true;

        // focus the content browser on the new animation
        FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
        TArray<UObject*> SyncObjects;
        SyncObjects.Add(ResultAnimation);
        ContentBrowserModule.Get().SyncBrowserToAssets(SyncObjects);
    }
    else 
    {
        FText WarningText = LOCTEXT("SRootMotionExtractionWidget_ExtractionFailedMsg", "Root motion extraction has failed, please double check the input animation sequences (ordinary and inplace). See console for additional details.");
        FMessageDialog::Open(EAppMsgType::Ok, WarningText);

        ResultAnimation->MarkAsGarbage();
    }
}



bool FMixamoAnimationRootMotionSolver::CanExecuteProcedure(const USkeleton* Skeleton) const
{
    // Check the asset content.
    // NOTE: this will load the asset if needed.
    if (!FMixamoAnimationRetargetingModule::Get().GetMixamoSkeletonRetargeter()->IsMixamoSkeleton(Skeleton))
    {
        return false;
    }

    // Check that the skeleton was processed with our retargeter !
    int32 RootBoneIndex = Skeleton->GetReferenceSkeleton().FindBoneIndex(TEXT("root"));
    if (RootBoneIndex == INDEX_NONE)
    {
        return false;
    }

    return true;
}



bool FMixamoAnimationRootMotionSolver::ExecuteExtraction(UAnimSequence* AnimSequence, const UAnimSequence* InPlaceAnimSequence)
{
    UAnimDataModel* AnimDataModel = AnimSequence->GetDataModel();
    UAnimDataModel* InPlaceAnimDataModel = InPlaceAnimSequence->GetDataModel();
    
    // take the hips bone track data from both animation sequences
    const FBoneAnimationTrack* HipsBoneTrack = AnimDataModel->FindBoneTrackByName(FName("Hips"));
    if (!HipsBoneTrack)
    {
        FMessageLog("LogMixamoToolkit").Error(FText::FromString(TEXT("Hips bone not found in the ordinary animation sequence.")));
        return false;
    }

    const FBoneAnimationTrack* InPlaceHipsBoneTrack = InPlaceAnimDataModel->FindBoneTrackByName(FName("Hips"));
    if (!InPlaceHipsBoneTrack)
    {
        FMessageLog("LogMixamoToolkit").Error(FText::FromString(TEXT("Hips bone not found in the inplace animation sequence.")));
        return false;
    }
    
    auto& HipsTrackData = HipsBoneTrack->InternalTrackData;
    auto& InPlaceHipsTrackData = InPlaceHipsBoneTrack->InternalTrackData;

    // nummber of keys should match between the two animations.
    if (HipsTrackData.PosKeys.Num() != InPlaceHipsTrackData.PosKeys.Num())
    {
        FMessageLog("LogMixamoToolkit").Error(FText::FromString(TEXT("Track data keys number mismatch between ordinary and inplace animation sequences.")));
        return false;
    }

    // PosKeys, RotKeys and ScaleKeys should have the same size
    if (FMath::Max3(HipsTrackData.PosKeys.Num(), HipsTrackData.RotKeys.Num(), HipsTrackData.ScaleKeys.Num())
        != FMath::Min3(HipsTrackData.PosKeys.Num(), HipsTrackData.RotKeys.Num(), HipsTrackData.ScaleKeys.Num()))
    {
        FMessageLog("LogMixamoToolkit").Error(FText::FromString(TEXT("Invalid track key data on ordinary animation sequence, expected uniform data.")));
        return false;
    }

    // PosKeys, RotKeys and ScaleKeys should have the same size
    if (FMath::Max3(InPlaceHipsTrackData.PosKeys.Num(), InPlaceHipsTrackData.RotKeys.Num(), InPlaceHipsTrackData.ScaleKeys.Num())
        != FMath::Min3(InPlaceHipsTrackData.PosKeys.Num(), InPlaceHipsTrackData.RotKeys.Num(), InPlaceHipsTrackData.ScaleKeys.Num()))
    {
        FMessageLog("LogMixamoToolkit").Error(FText::FromString(TEXT("Invalid track key data on inplace animation sequence, expected uniform data.")));
        return false;
    }

    // make a new track for the root bone
    // the keys num is equal to the hips keys num
    FRawAnimSequenceTrack RootBoneTrack;
    const int32 NumOfKeys = HipsTrackData.PosKeys.Num();
    RootBoneTrack.PosKeys.SetNum(NumOfKeys);
    RootBoneTrack.RotKeys.SetNum(NumOfKeys);
    RootBoneTrack.ScaleKeys.SetNum(NumOfKeys);

    // HipsBoneTrack = Root + Hips
    // InPlaceHipsBoneTrack = Hips
    // we want to extract the Root value and set to the new root track so:
    // Root = HipsBoneTrack - InPlaceHipsBoneTrack = (Root + Hips) - Hips = Root
    for (int i = 0; i < NumOfKeys; ++i)
    {
        RootBoneTrack.PosKeys[i] = HipsTrackData.PosKeys[i] - InPlaceHipsTrackData.PosKeys[i];
        RootBoneTrack.RotKeys[i] = HipsTrackData.RotKeys[i] * InPlaceHipsTrackData.RotKeys[i].Inverse();
        RootBoneTrack.ScaleKeys[i] = FVector3f(1);
    }

    IAnimationDataController& Controller = AnimSequence->GetController();
    constexpr bool bShouldTransact = false;
    // NOTE: modifications MUST be done inside a "bracket", otherwise each modification will fire a re-build of the animation.
    // After adding the "root" track, the re-build will fail since its track keys are missing.
    // Worst: there's a bug in UE5.0 (https://github.com/EpicGames/UnrealEngine/blob/05ce24e3038cb1994a7c71d4d0058dbdb112f52b/Engine/Source/Runtime/Engine/Private/Animation/AnimSequenceHelpers.cpp#L593)
    // where when no keys are present, element at index -1 is removed from an array, causing a random memory overriding.
    Controller.OpenBracket(LOCTEXT("FMixamoAnimationRootMotionSolver_ExecuteExtraction_AnimEdit", "Animation editing"), bShouldTransact);

    // now we can replace the HipsBoneTrack with InPlaceHipsBoneTrack
    Controller.SetBoneTrackKeys(FName("Hips"), InPlaceHipsTrackData.PosKeys, InPlaceHipsTrackData.RotKeys, InPlaceHipsTrackData.ScaleKeys, bShouldTransact);

    // add the new root track (now as the first item)
    ensure(Controller.InsertBoneTrack(FName("root"), 0, bShouldTransact) == 0);
    Controller.SetBoneTrackKeys(FName("root"), RootBoneTrack.PosKeys, RootBoneTrack.RotKeys, RootBoneTrack.ScaleKeys, bShouldTransact);

    // Apply all the changes at once.
    Controller.CloseBracket(bShouldTransact);

    return true;
}



float FMixamoAnimationRootMotionSolver::GetMaxBoneDisplacement(const UAnimSequence* AnimSequence, const FName& BoneName)
{
    UAnimDataModel* AnimDataModel = AnimSequence->GetDataModel();
    const FBoneAnimationTrack* HipsBoneTrack = AnimDataModel->FindBoneTrackByName(BoneName);
    if (!HipsBoneTrack)
        return 0;

    float MaxSize = 0;

    for (int i = 0; i < HipsBoneTrack->InternalTrackData.PosKeys.Num(); ++i)
    {
        float Size = HipsBoneTrack->InternalTrackData.PosKeys[i].Size();
        if (Size > MaxSize)
            MaxSize = Size;
    }

    return MaxSize;
}



const UAnimSequence* FMixamoAnimationRootMotionSolver::EstimateInPlaceAnimation(const UAnimSequence* AnimationA, const UAnimSequence* AnimationB)
{
    FName RefBoneName(TEXT("Hips"));

    /*
    Find the "in place" animation sequence. To do that we compare the two hips bone displacements.
    The animation sequence with the lower value is the "in place" one.
    @TODO: is this checks always reliable ?
    */
    float dA = GetMaxBoneDisplacement(AnimationA, RefBoneName);
    float dB = GetMaxBoneDisplacement(AnimationB, RefBoneName);

    const UAnimSequence* NormalAnimSequence = (dA < dB) ? AnimationB : AnimationA;
    const UAnimSequence* InPlaceAnimSequence = (dA < dB) ? AnimationA : AnimationB;

    return InPlaceAnimSequence;
}
