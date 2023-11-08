// Copyright 2022 UNAmedia. All Rights Reserved.

#pragma once

#include "Engine/EngineTypes.h"
#include "NamesMapper.h"



class USkeleton;
class USkeletalMesh;
struct FReferenceSkeleton;



/**
Class to map bones from a reference skeleton to another.

@attention Skeletal meshes can share the same USkeleton asset, but they can have different FReferenceSkeleton data
	(with more or less data).
	The effective valid bone data (indexes and names) used by a skeletal mesh are the ones stored in its FReferenceSkeleton object.

@attention To be used within a single method's stack space.
*/
class FBoneMapper
{
public:
	FBoneMapper(const FReferenceSkeleton * ASource, const FReferenceSkeleton * ADestination)
		: Source(ASource),
		  Destination(ADestination)
	{}

	virtual
	~FBoneMapper()
	{}

	/**
	Map a bone index from the source skeleton to a bone index into the destination skeleton.

	Returns INDEX_NONE if the bone can't be mapped.
	*/
	virtual
	int32 MapBoneIndex(int32 BoneIndex) const = 0;

protected:
	const FReferenceSkeleton * Source;
	const FReferenceSkeleton * Destination;
};



/**
A bone mapper based on FRigConfiguration.

A bone in the source skeleton is mapped to the bone in the destination skeleton sharing the same "rig node name",
as defined by the FRigConfiguration of the two skeletons (that must be compatible).

@attention "rig node name" is not the same as "bone name".
*/
class FRigConfigurationBoneMapper : public FBoneMapper
{
public:
	FRigConfigurationBoneMapper(const FReferenceSkeleton * ASource, const USkeleton * ASourceSkeleton, const FReferenceSkeleton * ADestination, const USkeleton * ADestinationSkeleton)
		: FBoneMapper(ASource, ADestination),
		  SourceSkeleton(ASourceSkeleton),
		  DestinationSkeleton(ADestinationSkeleton)
	{}

	virtual
	int32 MapBoneIndex(int32 BoneIndex) const override;

protected:
	const USkeleton * SourceSkeleton;
	const USkeleton * DestinationSkeleton;
};



/**
A bone mapper mapping bones by matching name.

A bone in the source skeleton is mapped to the bone in the destination skeleton having the same "bone name".
*/
class FEqualNameBoneMapper : public FBoneMapper
{
public:
	FEqualNameBoneMapper(const FReferenceSkeleton * ASource, const FReferenceSkeleton * ADestination)
		: FBoneMapper(ASource, ADestination)
	{}

	virtual
	int32 MapBoneIndex(int32 BoneIndex) const override;
};



/**
A bone mapper mapping bones by matching "translated" name.

A bone in the source skeleton is mapped to the bone in the destination skeleton having the same "translated bone name",
i.e. the source bone name is translated accordingly to a translation map and the resulting bone name is looked for in the destination skeleton.
*/
class FNameTranslationBoneMapper : public FBoneMapper
{
public:
	/**
	@param SourceToDest_BonesNameMapping Array of strings where the 2*i-th string is a bone name of the source skeleton and
		(2*i+1)-th string is the translated name in the destination skeleton.
	@param SourceToDest_BonesNameMappingNum Length of the array SourceToDest_BonesNameMapping.
	@param Reverse If the mapping table must be applied in reverse, i.e. mapping from (2*i+1) to 2*i (instead of the default 2*i -> 2*i+1).
		Set to true if SourceToDest_BonesNameMapping is mapping bones from the Destination skeleton to the Source skeleton, instead of the expected "Source to Destination".
	*/
	FNameTranslationBoneMapper(const FReferenceSkeleton* ASource, const FReferenceSkeleton* ADestination, const FStaticNamesMapper & Mapper);

	virtual
	int32 MapBoneIndex(int32 BoneIndex) const override;

	FName MapBoneName(FName BoneName) const;

private:
	FStaticNamesMapper NamesMapper;
};



/**
Class to compute a matching pose from one skeleton to another, distinct one.

@attention To be used within a single method's stack space.
*/
class FSkeletonPoser
{
public:
	/**
	@param Reference the Reference Skeleton used by the poser, i.e. the skeleton that we want to "reproduce"
	@param ReferenceBonePose the pose that we want to reproduce in other skeletons.
		This is an array of transforms in Bone Space, following the order and hierarchy as in Reference->GetReferenceSkeleton().
	*/
	FSkeletonPoser(const USkeleton * Reference, const TArray<FTransform> & ReferenceBonePose);
	/**
	Compute the matching pose for a given Skeletal Mesh.

	@param Mesh The skeletal mesh for which compute a pose, matching the Reference Pose configured in the constructor.
	@param BoneMapper A bone mapper converting bones used by Mesh into bones of the Reference skeleton.
	@param PreserveCSBonesNames A set of bone names of Mesh for which the Component Space transform (relative to the parent) must be preserved.
	@param ParentChildBoneNamesToBypassOneChildConstraint A set of parent-child bone names of Mesh that must be forcefully oriented regardless of
			the children number of the parent bone.
	@param MeshBonePose In output it will contain the resulting computed matching pose for Mesh.
		This is an array of transforms in Bone Space, following the order and hierarchy as in Mesh->GetRefSkeleton().
	*/
	void Pose(const USkeletalMesh * Mesh, const FBoneMapper & BoneMapper, const TArray<FName> & PreserveCSBonesNames, const TArray<TPair<FName, FName>> & ParentChildBoneNamesToBypassOneChildConstraint, TArray<FTransform> & MeshBonePose) const;

	// Utility methods.
	void PoseBasedOnRigConfiguration(const USkeletalMesh * Mesh, const TArray<FName> & PreserveCSBonesNames, const TArray<TPair<FName, FName>>& ParentChildBoneNamesToBypassOneChildConstraint, TArray<FTransform> & MeshBonePose) const;
	void PoseBasedOnCommonBoneNames(const USkeletalMesh * Mesh, const TArray<FName> & PreserveCSBonesNames, const TArray<TPair<FName, FName>>& ParentChildBoneNamesToBypassOneChildConstraint, TArray<FTransform> & MeshBonePose) const;
	void PoseBasedOnMappedBoneNames(const USkeletalMesh * Mesh, const TArray<FName> & PreserveCSBonesNames, const FStaticNamesMapper & SourceToDest_BonesNameMapping, const TArray<TPair<FName, FName>>& ParentChildBoneNamesToBypassOneChildConstraint, TArray<FTransform> & MeshBonePose) const;

	// Utility methods
	static void ApplyPoseToRetargetBasePose(USkeletalMesh* Mesh, const TArray<FTransform>& MeshBonePose);
	static void ApplyPoseToIKRetargetPose(USkeletalMesh* Mesh, UIKRetargeterController* Controller, const TArray<FTransform>& MeshBonePose);

private:
	const USkeleton * ReferenceSkeleton;
	TArray<FTransform> ReferenceCSBonePoses;

private:
	void Pose(const FReferenceSkeleton & EditRefSkeleton, const FBoneMapper & BoneMapper, const TSet<int32> & PreserveCSBonesIndices, const TSet<TPair<int32, int32>>& ParentChildBoneIndicesToBypassOneChildConstraint, TArray<FTransform> & MeshBonePoses) const;

	static
	FTransform ComputeComponentSpaceTransform(const FReferenceSkeleton & RefSkeleton, const TArray<FTransform> & RelTransforms, int32 BoneIndex);
	static
	void BoneSpaceToComponentSpaceTransforms(const FReferenceSkeleton & RefSkeleton, const TArray<FTransform> & BSTransforms, TArray<FTransform> & CSTransforms);
	static
	void NumOfChildren(const FReferenceSkeleton & RefSkeleton, TArray<int> & children);

	static
	void LogReferenceSkeleton(const FReferenceSkeleton & RefSkeleton, const TArray<FTransform> & Poses, int BoneIndex = 0, int Deep = 0);
};
