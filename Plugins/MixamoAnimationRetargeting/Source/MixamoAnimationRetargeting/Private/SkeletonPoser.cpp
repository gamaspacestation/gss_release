// Copyright 2022 UNAmedia. All Rights Reserved.

#include "SkeletonPoser.h"
#include "MixamoToolkitPrivatePCH.h"

#include "Engine/SkeletalMesh.h"
#include "Animation/Skeleton.h"
#include "Animation/Rig.h"

#include <RetargetEditor/IKRetargeterController.h>



// Define it to check mathematical equivalences used by the algorithm.
// NOTE: leave it undefined on redistribution, as ordinary small numerical errors could halt the editor otherwise.
//#define FSKELETONPOSER_CHECK_NUMERIC_CODE_

#ifdef FSKELETONPOSER_CHECK_NUMERIC_CODE_
	#define FSKELETONPOSER_CHECK_(X,M)	checkf(X, M)
	#define FSKELETONPOSER_CHECK_FTRANSFORM_EQUALS_(A,B,M)	checkf((A).Equals(B), M)
	#pragma message ("MIXAMOANIMATIONRETARGETING_CHECK_NUMERIC_CODE_ symbol is defined. Undefine it for redistribution.")
#else
	#define FSKELETONPOSER_CHECK_(X,M)
	#define FSKELETONPOSER_CHECK_FTRANSFORM_EQUALS_(A,B,M)
#endif



#define LOCTEXT_NAMESPACE "FMixamoAnimationRetargetingModule"



int32 FRigConfigurationBoneMapper::MapBoneIndex(int32 BoneIndex) const
{
	const FName SourceBoneName = Source->GetBoneName(BoneIndex);
	const FName RigNodeName = SourceSkeleton->GetRigNodeNameFromBoneName(SourceBoneName);
	const FName DestinationBoneName = DestinationSkeleton->GetRigBoneMapping(RigNodeName);
	const int32 DestinationBoneIndex = Destination->FindBoneIndex(DestinationBoneName);
	return DestinationBoneIndex;
}



int32 FEqualNameBoneMapper::MapBoneIndex(int32 BoneIndex) const
{
	const FName SourceBoneName = Source->GetBoneName(BoneIndex);
	const int32 DestinationBoneIndex = Destination->FindBoneIndex(SourceBoneName);
	return DestinationBoneIndex;
}



FNameTranslationBoneMapper::FNameTranslationBoneMapper(
	const FReferenceSkeleton* ASource,
	const FReferenceSkeleton* ADestination,
	const FStaticNamesMapper & Mapper
)
	: FBoneMapper(ASource, ADestination),
	  NamesMapper(Mapper)
{
}



int32 FNameTranslationBoneMapper::MapBoneIndex(int32 BoneIndex) const
{
	const FName SourceBoneName = Source->GetBoneName(BoneIndex);
	const FName TargetBoneName = MapBoneName(SourceBoneName);
	if (!TargetBoneName.IsNone())
	{
		const int32 DestinationBoneIndex = Destination->FindBoneIndex(TargetBoneName);
		return DestinationBoneIndex;
	}
	return INDEX_NONE;
}



FName FNameTranslationBoneMapper::MapBoneName(FName BoneName) const
{
	return NamesMapper.MapName(BoneName);
}



FSkeletonPoser::FSkeletonPoser(const USkeleton * Reference, const TArray<FTransform> & ReferenceBonePose)
	: ReferenceSkeleton(Reference)
{
	check(ReferenceSkeleton != nullptr);
	checkf(ReferenceBonePose.Num() == ReferenceSkeleton->GetReferenceSkeleton().GetNum(), TEXT("Length of bone pose must match the one of the reference skeleton."));
	BoneSpaceToComponentSpaceTransforms(ReferenceSkeleton->GetReferenceSkeleton(), ReferenceBonePose, ReferenceCSBonePoses);
}



void FSkeletonPoser::Pose(
	const USkeletalMesh * Mesh,
	const FBoneMapper & BoneMapper,
	const TArray<FName> & PreserveCSBonesNames,
	const TArray<TPair<FName, FName>>& ParentChildBoneNamesToBypassOneChildConstraint,
	TArray<FTransform> & MeshBonePose
) const
{
	check(Mesh != nullptr);

	// Convert bone names to bone indices.
	TSet<int32> PreserveCSBonesIndices;
	if (PreserveCSBonesNames.Num() > 0)
	{
		PreserveCSBonesIndices.Reserve(PreserveCSBonesNames.Num());
		for (const FName & BoneName : PreserveCSBonesNames)
		{
			const int32 BoneIndex = Mesh->GetRefSkeleton().FindBoneIndex(BoneName);
			if (BoneIndex != INDEX_NONE)
			{
				PreserveCSBonesIndices.Add(BoneIndex);
			}
		}
	}

	TSet<TPair<int32, int32>> ParentChildBoneIndicesToBypassOneChildConstraint;
	ParentChildBoneIndicesToBypassOneChildConstraint.Reserve(ParentChildBoneNamesToBypassOneChildConstraint.Num());
	for (const auto& ParentChildNames : ParentChildBoneNamesToBypassOneChildConstraint)
	{
		const int32 ParentBoneIndex = Mesh->GetRefSkeleton().FindBoneIndex(ParentChildNames.Get<0>());
		const int32 ChildBoneIndex = Mesh->GetRefSkeleton().FindBoneIndex(ParentChildNames.Get<1>());
		if (ParentBoneIndex != INDEX_NONE && ChildBoneIndex != INDEX_NONE)
		{
			check(ParentBoneIndex != ChildBoneIndex);
			ParentChildBoneIndicesToBypassOneChildConstraint.Add(MakeTuple(ParentBoneIndex, ChildBoneIndex));
		}
	}

	UE_LOG(LogMixamoToolkit, Verbose, TEXT("BEGIN: %s -> %s"), *ReferenceSkeleton->GetName(), *Mesh->GetName());
	// NOTE: the RefSkeleton of the Skeletal Mesh counts for its mesh proportions.
	Pose(Mesh->GetRefSkeleton(), BoneMapper, PreserveCSBonesIndices, ParentChildBoneIndicesToBypassOneChildConstraint, MeshBonePose);
	UE_LOG(LogMixamoToolkit, Verbose, TEXT("END: %s -> %s"), *ReferenceSkeleton->GetName(), *Mesh->GetName());
}



void FSkeletonPoser::PoseBasedOnRigConfiguration(
	const USkeletalMesh * Mesh, 
	const TArray<FName> & PreserveCSBonesNames, 
	const TArray<TPair<FName, FName>>& ParentChildBoneNamesToBypassOneChildConstraint,
	TArray<FTransform> & MeshBonePose) const
{
	check(Mesh != nullptr);
	Pose(Mesh, FRigConfigurationBoneMapper(& Mesh->GetRefSkeleton(), Mesh->GetSkeleton(), & ReferenceSkeleton->GetReferenceSkeleton(), ReferenceSkeleton), PreserveCSBonesNames, ParentChildBoneNamesToBypassOneChildConstraint, MeshBonePose);
}



void FSkeletonPoser::PoseBasedOnCommonBoneNames(
	const USkeletalMesh * Mesh,
	const TArray<FName> & PreserveCSBonesNames,
	const TArray<TPair<FName, FName>>& ParentChildBoneNamesToBypassOneChildConstraint,
	TArray<FTransform> & MeshBonePose) const
{
	check(Mesh != nullptr);
	Pose(Mesh, FEqualNameBoneMapper(& Mesh->GetRefSkeleton(), & ReferenceSkeleton->GetReferenceSkeleton()), PreserveCSBonesNames, ParentChildBoneNamesToBypassOneChildConstraint, MeshBonePose);
}



void FSkeletonPoser::PoseBasedOnMappedBoneNames(
	const USkeletalMesh* Mesh,
	const TArray<FName>& PreserveCSBonesNames,
	const FStaticNamesMapper & SourceToDest_BonesNameMapping,
	const TArray<TPair<FName, FName>>& ParentChildBoneNamesToBypassOneChildConstraint,
	TArray<FTransform> & MeshBonePose
) const
{
	check(Mesh != nullptr);
	Pose(Mesh, FNameTranslationBoneMapper(& Mesh->GetRefSkeleton(), & ReferenceSkeleton->GetReferenceSkeleton(), SourceToDest_BonesNameMapping), PreserveCSBonesNames, ParentChildBoneNamesToBypassOneChildConstraint, MeshBonePose);
}


TArray<int32> GetBreadthFirstSortedBones(const FReferenceSkeleton& Skeleton)
{
	const int32 NumBones = Skeleton.GetNum();

	TArray<int32> SortedIndices;
	SortedIndices.Reserve(NumBones);

	for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
	{
		SortedIndices.Add(BoneIndex);
	}

	SortedIndices.Sort([&](int32 IndexA, int32 IndexB) {
		return Skeleton.GetDepthBetweenBones(IndexA, 0) < Skeleton.GetDepthBetweenBones(IndexB, 0);
	});

	return SortedIndices;
}


void FSkeletonPoser::Pose(
	const FReferenceSkeleton & EditRefSkeleton,
	const FBoneMapper & BoneMapper,
	const TSet<int32> & PreserveCSBonesIndices,
	const TSet<TPair<int32, int32>>& ParentChildBoneIndicesToBypassOneChildConstraint,
	TArray<FTransform> & EditBonePoses
) const
{
	check(ReferenceSkeleton != nullptr);
	// NOTE: ReferenceSkeleton is used only to get hierarchical infos.
	const FReferenceSkeleton & ReferenceRefSkeleton = ReferenceSkeleton->GetReferenceSkeleton();

	const int32 NumBones = EditRefSkeleton.GetNum();
	EditBonePoses = EditRefSkeleton.GetRefBonePose();
	check(EditBonePoses.Num() == NumBones);

	TArray<int> EditChildrens;
	NumOfChildren(EditRefSkeleton, EditChildrens);
	TArray<FTransform> OriginalEditCSBonePoses;
	if (PreserveCSBonesIndices.Num () > 0)
	{
		BoneSpaceToComponentSpaceTransforms(EditRefSkeleton, EditRefSkeleton.GetRefBonePose(), OriginalEditCSBonePoses);
	}

	//UE_LOG(LogMixamoToolkit, Verbose, TEXT("Initial pose"));
	//LogReferenceSkeleton(EditRefSkeleton, EditRefSkeleton.GetRefBonePose());
	auto SortedIndices = GetBreadthFirstSortedBones(EditRefSkeleton);
	for (int32 EditBoneIndex : SortedIndices)
	{
		UE_LOG(LogMixamoToolkit, Verbose, TEXT("Processing bone %d (%s)"), EditBoneIndex, *EditRefSkeleton.GetBoneName(EditBoneIndex).ToString());

		FVector ReferenceCSBoneOrientation;
		if (PreserveCSBonesIndices.Contains(EditBoneIndex))
		{
			UE_LOG(LogMixamoToolkit, Verbose, TEXT("  Preserving its Component-Space orientation"));

			// Compute orientation of reference bone, considering the original CS bone poses as reference.
			const int32 ReferenceBoneParentIndex = EditRefSkeleton.GetParentIndex(EditBoneIndex);
			check(ReferenceBoneParentIndex < EditBoneIndex && "Parent bone must have lower index");
			const FTransform & ReferenceCSParentTransform = (ReferenceBoneParentIndex != INDEX_NONE ? OriginalEditCSBonePoses[ReferenceBoneParentIndex] : FTransform::Identity);
			const FTransform & ReferenceCSTransform = OriginalEditCSBonePoses[EditBoneIndex];
			ReferenceCSBoneOrientation = (ReferenceCSTransform.GetLocation() - ReferenceCSParentTransform.GetLocation()).GetSafeNormal();
		}
		else
		{
			UE_LOG(LogMixamoToolkit, Verbose, TEXT("  Re-posing it"));

			// Get the retarget bone on the reference skeleton.
			const int32 ReferenceBoneIndex = BoneMapper.MapBoneIndex(EditBoneIndex);
			if (ReferenceBoneIndex == INDEX_NONE)
			{
				// Bone not retargeted, skip.
				UE_LOG(LogMixamoToolkit, Verbose, TEXT("  Skipped: not found the corresponding bone in the reference skeleton"));
				continue;
			}
			UE_LOG(LogMixamoToolkit, Verbose, TEXT("  Corresponding bone in the reference skeleton: %d (%s)"), ReferenceBoneIndex, *ReferenceRefSkeleton.GetBoneName(ReferenceBoneIndex).ToString());

			// Compute orientation of reference bone.
			const int32 ReferenceBoneParentIndex = ReferenceRefSkeleton.GetParentIndex(ReferenceBoneIndex);
			check(ReferenceBoneParentIndex < ReferenceBoneIndex && "Parent bone must have lower index");
			UE_LOG(LogMixamoToolkit, Verbose, TEXT("    Parent bone: %d (%s)"), ReferenceBoneParentIndex, ReferenceBoneParentIndex != INDEX_NONE ? *ReferenceRefSkeleton.GetBoneName(ReferenceBoneParentIndex).ToString() : TEXT("-"));
			const FTransform & ReferenceCSParentTransform = (ReferenceBoneParentIndex != INDEX_NONE ? ReferenceCSBonePoses[ReferenceBoneParentIndex] : FTransform::Identity);
			const FTransform & ReferenceCSTransform = ReferenceCSBonePoses[ReferenceBoneIndex];
			ReferenceCSBoneOrientation = (ReferenceCSTransform.GetLocation() - ReferenceCSParentTransform.GetLocation()).GetSafeNormal();
			// Skip degenerated bones.
			if (ReferenceCSBoneOrientation.IsNearlyZero())
			{
				UE_LOG(LogMixamoToolkit, Verbose, TEXT("  Skipped: degenerate bone orientation in the reference skeleton"));
				continue;
			}
		}

		// Compute current orientation of the bone to retarget (skeleton).
		const int32 EditBoneParentIndex = EditRefSkeleton.GetParentIndex(EditBoneIndex);
		check(EditBoneParentIndex < EditBoneIndex && "Parent bone must have been already retargeted");
		if (EditBoneParentIndex == INDEX_NONE)
		{
			// We must rotate the parent bone, but it doesn't exist. Skip.
			UE_LOG(LogMixamoToolkit, Verbose, TEXT("  Skipped: no parent bone"));
			continue;
		}
		UE_LOG(LogMixamoToolkit, Verbose, TEXT("  Parent bone: %d (%s)"), EditBoneParentIndex, *EditRefSkeleton.GetBoneName(EditBoneParentIndex).ToString());

		if (EditChildrens[EditBoneParentIndex] > 1 &&
			!ParentChildBoneIndicesToBypassOneChildConstraint.Contains(MakeTuple(EditBoneParentIndex, EditBoneIndex)))
		{
			// If parent bone has multiple children, modifying it here would ruin the sibling bones. Skip. [NOTE: this bone will differ from the expected result!]
			UE_LOG(LogMixamoToolkit, Verbose, TEXT("  Skipped: bone %d (%s) not re-oriented because its parent bone (%d - %s) controls also other bones"), EditBoneIndex, *EditRefSkeleton.GetBoneName(EditBoneIndex).ToString(), EditBoneParentIndex, *EditRefSkeleton.GetBoneName(EditBoneParentIndex).ToString());
			continue;
		}
		// Compute the transforms on the up-to-date skeleton (they cant' be cached).
		const FTransform EditCSParentTransform = ComputeComponentSpaceTransform(EditRefSkeleton, EditBonePoses, EditBoneParentIndex);
		const FTransform EditCSTransform = EditBonePoses[EditBoneIndex] * EditCSParentTransform;
		const FVector EditCSBoneOrientation = (EditCSTransform.GetLocation() - EditCSParentTransform.GetLocation()).GetSafeNormal();

		// Skip degenerated or already-aligned bones.
		if (EditCSBoneOrientation.IsNearlyZero() || ReferenceCSBoneOrientation.Equals(EditCSBoneOrientation))
		{
			UE_LOG(LogMixamoToolkit, Verbose, TEXT("  Skipped: degenerate or already-aligned bone"));
			continue;
		}

		// Delta rotation (in Component Space) to make the skeleton bone aligned to the reference one.
		const FQuat EditToReferenceCSRotation = FQuat::FindBetweenVectors(EditCSBoneOrientation, ReferenceCSBoneOrientation);
		FSKELETONPOSER_CHECK_(
			EditToReferenceCSRotation.RotateVector(EditCSBoneOrientation).Equals(ReferenceCSBoneOrientation),
			TEXT("The rotation applied to the Edited Bone orientation must match the Reference one, in Component Space")
		);
		// Convert from Component Space to skeleton Bone Space
		const FQuat EditToReferenceBSRotation = EditCSParentTransform.GetRotation().Inverse() * EditToReferenceCSRotation * EditCSParentTransform.GetRotation();

#if defined(FSKELETONPOSER_CHECK_NUMERIC_CODE_) && DO_CHECK
		const FTransform & EditParentRefBonePose = EditRefSkeleton.GetRefBonePose()[EditBoneParentIndex];
#endif
		FSKELETONPOSER_CHECK_FTRANSFORM_EQUALS_(
			EditBonePoses[EditBoneParentIndex],
			EditParentRefBonePose,
			TEXT("Bone pose transform is still the same as the original one")
		);
		
		// Apply the rotation to the *parent* bone (yep!!!)
		EditBonePoses[EditBoneParentIndex].ConcatenateRotation(EditToReferenceBSRotation);

#if defined(FSKELETONPOSER_CHECK_NUMERIC_CODE_) && DO_CHECK
		{
			const FTransform NewSkeletonCSParentTransform = ComputeComponentSpaceTransform(EditRefSkeleton, EditBonePoses, EditBoneParentIndex);
			// For some reasons, check on thumbs need a much higher tollerance (thumb_02_l, thumb_03_l, thumb_02_r, thumb_03_r).
			FSKELETONPOSER_CHECK_(
				((EditBonePoses[EditBoneIndex] * NewSkeletonCSParentTransform).GetLocation() - NewSkeletonCSParentTransform.GetLocation()).GetSafeNormal().Equals(ReferenceCSBoneOrientation, 1e-3),
				TEXT("The new Bone pose results now in the same orientation as the reference one")
			);
		}
#endif
		FSKELETONPOSER_CHECK_FTRANSFORM_EQUALS_(
			EditBonePoses[EditBoneParentIndex],
			FTransform(EditToReferenceBSRotation) * EditParentRefBonePose,
			TEXT("Using ConcatenateRotation() is the same as pre-multiplying with the delta rotation")
		);
		UE_LOG(LogMixamoToolkit, Verbose, TEXT("  Done: changed parent bone %d (%s): %s"), EditBoneParentIndex, *EditRefSkeleton.GetBoneName(EditBoneParentIndex).ToString(), *EditBonePoses[EditBoneParentIndex].ToString());

		// Notes.
		//
		// FTransform uses the VQS notation: S->Q->V (where S = scale, Q = rotation, V = translation; considering each of them as affine transforms: S * Q * V).
		//
		// FQuat multiples in the opposite order, i.e. Q*Q' corresponds to q'*q.
		//
		// SetRotation() - used by FIKRetargetPose - changes Q, in the middle of the S*Q*V.
		// Let's consider Q and Q' the FTransform corresponding to the quaternions q and q',
		// after SetRotation(Q*Q') the corresponding FTransform is:
		//
		//     S * (Q * Q') * V = S * Q * V * V(-1) * Q' * V
		//
		// i.e. it's equal to the original FTransform S*Q*V post-multiplied by V(-1)*Q'*V.
	}
	//UE_LOG(LogMixamoToolkit, Verbose, TEXT("Retargeted pose"));
	//LogReferenceSkeleton(EditRefSkeleton, EditBonePoses);
}



void FSkeletonPoser::ApplyPoseToRetargetBasePose(USkeletalMesh* Mesh, const TArray<FTransform>& MeshBonePose)
{
	checkf(Mesh->GetRetargetBasePose().Num() == MeshBonePose.Num(), TEXT("Computed pose must have the same number of transforms as the target retarget base pose"));
	// We'll change RetargetBasePose.
	Mesh->Modify();
	// Transforms computed by FSkeletonPoser::Pose() are already compatible with RetargetBasePose, a simple assignment is enough.
	Mesh->GetRetargetBasePose() = MeshBonePose;
}



void FSkeletonPoser::ApplyPoseToIKRetargetPose(USkeletalMesh* Mesh, UIKRetargeterController* Controller, const TArray<FTransform>& MeshBonePose)
{
	check(Controller != nullptr);
	
	// NOTE: using the FIKRigSkeleton::CurrentPoseLocal (Controller->GetAsset()->GetTargetIKRig()->Skeleton) is wrong
	// as it reflects only the Skeletal Mesh used to create the IK Rig asset, and not current input Mesh.
	//
	// UIKRetargetProcessor::Initialize() calls FRetargetSkeleton::Initialize() that calls FRetargetSkeleton::GenerateRetargetPose(),
	// and they re-generate the RetargetLocalPose (corresponding to FIKRigSkeleton::CurrentPoseLocal)
	// from SkeletalMesh->GetRefSkeleton().GetRefBonePose() [where SkeletalMesh is the target skeletal mesh]
	//
	// So we do it also here.
	const FReferenceSkeleton& MeshRefSkeleton = Mesh->GetRefSkeleton();
	const TArray<FTransform> & TargetBonePose = Mesh->GetRefSkeleton().GetRefBonePose();

	const int32 NumBones = MeshBonePose.Num();
	checkf(TargetBonePose.Num() == NumBones, TEXT("Computed pose must have the same number of transforms as in the target IK Rig Skeleton"));
	for (int32 EditBoneIndex = 0; EditBoneIndex < NumBones; ++EditBoneIndex)
	{
		/**
		See also the comments in FSkeletonPoser::Pose().

		FTransform follows the VQS notation: T=S*Q*V.

		IKRigSkeleton.CurrentPoseLocal[EditBoneIndex] is the base pose (=S*R*V) used by IKRigSkeleton
		to compute the final pose when considering the Rotation Offset (call it Q).
		
		MeshBonePose[EditBoneIndex] contains the resulting pose with the added Rotation Offset Q (= S*R'*V = S*(R*Q)*V).

		R' = R * Q
		R(-1) * R' = Q

		as quaternions, and considering that FQuat applies multiplications in reverse order:

		r' * r(-1) = q
		*/
		FQuat Q = MeshBonePose[EditBoneIndex].GetRotation() * TargetBonePose[EditBoneIndex].GetRotation().Inverse();
		Controller->SetRotationOffsetForRetargetPoseBone(MeshRefSkeleton.GetBoneName(EditBoneIndex), Q);

#if defined(FSKELETONPOSER_CHECK_NUMERIC_CODE_) && DO_CHECK
		{
			// This is how the FIKRetargetPose computes the final bone poses (calling SetRotation()).
			FTransform IKRes = TargetBonePose[EditBoneIndex];
			IKRes.SetRotation(Q * IKRes.GetRotation());
			FSKELETONPOSER_CHECK_FTRANSFORM_EQUALS_(
				MeshBonePose[EditBoneIndex],
				IKRes,
				TEXT("The computed FIKRetargetPose pose must match the computed mesh bone pose")
			);
		}
#endif
	}
}



FTransform FSkeletonPoser::ComputeComponentSpaceTransform(const FReferenceSkeleton & RefSkeleton, const TArray<FTransform> & RelTransforms, int32 BoneIndex)
{
	if (BoneIndex == INDEX_NONE)
	{
		return FTransform::Identity;
	}

	FTransform T = RelTransforms[BoneIndex];
	int32 i = RefSkeleton.GetParentIndex(BoneIndex);
	while (i != INDEX_NONE)
	{
		checkf(i < BoneIndex, TEXT("Parent bone must have been already retargeted"));
		T *= RelTransforms[i];
		i = RefSkeleton.GetParentIndex(i);
	}

	return T;
}



void FSkeletonPoser::BoneSpaceToComponentSpaceTransforms(const FReferenceSkeleton & RefSkeleton, const TArray<FTransform> & BSTransforms, TArray<FTransform> & CSTransforms)
{
	check(RefSkeleton.GetNum() == BSTransforms.Num());
	const int32 NumBones = RefSkeleton.GetNum();
	CSTransforms.Empty(NumBones);
	CSTransforms.AddUninitialized(NumBones);
	for (int32 iBone = 0; iBone < NumBones; ++iBone)
	{
		CSTransforms[iBone] = BSTransforms[iBone];
		const int32 iParent = RefSkeleton.GetParentIndex(iBone);
		check(iParent < iBone);
		if (iParent != INDEX_NONE)
		{
			CSTransforms[iBone] *= CSTransforms[iParent];
		}
	}
}



void FSkeletonPoser::NumOfChildren(const FReferenceSkeleton & RefSkeleton, TArray<int> & children)
{
	const int32 NumBones = RefSkeleton.GetNum();
	children.Empty(NumBones);
	children.AddUninitialized(NumBones);
	for (int32 iBone = 0; iBone < NumBones; ++iBone)
	{
		children[iBone] = 0;
		const int32 iParent = RefSkeleton.GetParentIndex(iBone);
		check(iParent < iBone);
		if (iParent != INDEX_NONE)
		{
			++children[iParent];
		}
	}
}



void FSkeletonPoser::LogReferenceSkeleton (const FReferenceSkeleton & RefSkeleton, const TArray<FTransform> & Poses, int BoneIndex, int Deep)
{
	FString Indent;
	for (int i = 0; i < Deep; ++i)
	{
		Indent.Append(TEXT("  "));
	}

	UE_LOG(LogMixamoToolkit, Verbose, TEXT("%s[%d - %s]: %s"), * Indent, BoneIndex, * RefSkeleton.GetBoneName(BoneIndex).ToString(), * Poses[BoneIndex].ToString ());

	for (int i = BoneIndex + 1; i < Poses.Num(); ++i)
	{
		if (RefSkeleton.GetParentIndex(i) == BoneIndex)
		{
			LogReferenceSkeleton(RefSkeleton, Poses, i, Deep + 1);
		}
	}
}



#undef LOCTEXT_NAMESPACE
