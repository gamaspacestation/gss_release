// Copyright 2022 UNAmedia. All Rights Reserved.

#pragma once

#include "Engine/EngineTypes.h"

#include "NamesMapper.h"



class UIKRigDefinition;
class UIKRetargeter;
class USkeleton;
class USkeletalMesh;
struct FReferenceSkeleton;



/**
Type of the target skeleton when retargeting from a Mixamo skeleton.

At the moment we assume that ST_UE5_MANNEQUIN can be used also for the MetaHuman skeleton,
if needed we'll add a distinct ST_METAHUMAN value in future.
*/
enum class ETargetSkeletonType
{
	ST_UNKNOWN = 0,
	ST_UE4_MANNEQUIN,
	ST_UE5_MANNEQUIN,

	ST_SIZE
};



/**
Manage the retargeting of a Mixamo skeleton.

Further info:
- https://docs.unrealengine.com/latest/INT/Engine/Animation/Skeleton/
- https://docs.unrealengine.com/latest/INT/Engine/Animation/AnimationRetargeting/index.html
- https://docs.unrealengine.com/latest/INT/Engine/Animation/AnimHowTo/Retargeting/index.html
- https://docs.unrealengine.com/latest/INT/Engine/Animation/RetargetingDifferentSkeletons/
*/
class FMixamoSkeletonRetargeter
{
public:
	FMixamoSkeletonRetargeter();

	void RetargetToUE4Mannequin(TArray<USkeleton *> Skeletons) const;
	bool IsMixamoSkeleton(const USkeleton * Skeleton) const;

private:
	bool OnShouldFilterNonUEMannequinSkeletonAsset(const FAssetData& AssetData) const;
	ETargetSkeletonType GetTargetSkeletonType(const USkeleton* Skeleton) const;
	bool IsUEMannequinSkeleton(const USkeleton * Skeleton) const;
	void Retarget(USkeleton* Skeleton, const USkeleton * ReferenceSkeleton, ETargetSkeletonType ReferenceSkeletonType) const;
	bool HasFakeRootBone(const USkeleton* Skeleton) const;
	void AddRootBone(USkeleton * Skeleton, TArray<USkeletalMesh *> SkeletalMeshes) const;
	void AddRootBone(const USkeleton * Skeleton, FReferenceSkeleton * RefSkeleton) const;
	void SetupTranslationRetargetingModes(USkeleton* Skeleton) const;
	void RetargetBasePose(
		TArray<USkeletalMesh *> SkeletalMeshes,
		const USkeleton * ReferenceSkeleton,
		const TArray<FName>& PreserveCSBonesNames,
		const FStaticNamesMapper & EditToReference_BoneNamesMapping,
		const TArray<TPair<FName, FName>>& ParentChildBoneNamesToBypassOneChildConstraint,
		bool bApplyPoseToRetargetBasePose,
		class UIKRetargeterController* Controller
	) const;
	USkeleton * AskUserForTargetSkeleton() const;
	bool AskUserOverridingAssetsConfirmation(const TArray<UObject*>& AssetsToOverwrite) const;
	/// Valid within a single method's stack space.
	void GetAllSkeletalMeshesUsingSkeleton(const USkeleton * Skeleton, TArray<FAssetData> & SkeletalMeshes) const;
	void GetAllSkeletalMeshesUsingSkeleton(const USkeleton * Skeleton, TArray<USkeletalMesh *> & SkeletalMeshes) const;
	void SetPreviewMesh(USkeleton * Skeleton, USkeletalMesh * PreviewMesh) const;

	void EnumerateAssetsToOverwrite(const USkeleton* Skeleton, const USkeleton* ReferenceSkeleton, TArray<UObject*>& AssetsToOverride) const;

	UIKRigDefinition* CreateIKRig(
		const FString & PackagePath,
		const FString & AssetName,
		const USkeleton* Skeleton
	) const;
	UIKRigDefinition* CreateMixamoIKRig(const USkeleton* Skeleton) const;
	UIKRigDefinition* CreateUEMannequinIKRig(const USkeleton* Skeleton, ETargetSkeletonType SkeletonType) const;
	UIKRetargeter* CreateIKRetargeter(
		const FString & PackagePath,
		const FString & AssetName,
		UIKRigDefinition* SourceRig,
		UIKRigDefinition* TargetRig,
		const FStaticNamesMapper & TargetToSource_ChainNamesMapping,
		const TArray<FName> & TargetBoneChainsToSkip,
		const TArray<FName> & TargetBoneChainsDriveIKGoal,
		const TArray<FName>& TargetBoneChainsOneToOneRotationMode
	) const;

private:
	// UE4 Mannequin to Mixamo data.
	const FStaticNamesMapper UE4MannequinToMixamo_BoneNamesMapping;
	const FStaticNamesMapper UE4MannequinToMixamo_ChainNamesMapping;
	// UE5/MetaHuman to Mixamo data.
	const FStaticNamesMapper UE5MannequinToMixamo_BoneNamesMapping;
	const FStaticNamesMapper UE5MannequinToMixamo_ChainNamesMapping;
};
