// Copyright 2022 UNAmedia. All Rights Reserved.

#include "SkeletonMatcher.h"

#include <Animation/Skeleton.h>



#define LOCTEXT_NAMESPACE "FMixamoAnimationRetargetingModule"



FSkeletonMatcher::FSkeletonMatcher(
	const TArray<FName>& InBoneNames,
	float InMinimumMatchingPerc
)
	: BoneNames(InBoneNames),
	  MinimumMatchingPerc(InMinimumMatchingPerc)
{}



bool FSkeletonMatcher::IsMatching(const USkeleton* Skeleton) const
{
	// No Skeleton, No matching...
	if (Skeleton == nullptr)
	{
		return false;
	}

	const int32 NumExpectedBones = BoneNames.Num();
	int32 nMatchingBones = 0;
	const FReferenceSkeleton & SkeletonRefSkeleton = Skeleton->GetReferenceSkeleton();
	for (int32 i = 0; i < NumExpectedBones; ++i)
	{
		const int32 BoneIndex = SkeletonRefSkeleton.FindBoneIndex(BoneNames[i]);
		if (BoneIndex != INDEX_NONE)
		{
			++nMatchingBones;
		}
	}
	const float MatchedPercentage = float(nMatchingBones) / float(NumExpectedBones);

	return MatchedPercentage >= MinimumMatchingPerc;
}



#undef LOCTEXT_NAMESPACE
