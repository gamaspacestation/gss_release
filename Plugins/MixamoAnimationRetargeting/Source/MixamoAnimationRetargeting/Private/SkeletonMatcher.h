// Copyright 2022 UNAmedia. All Rights Reserved.

#pragma once

#include "Engine/EngineTypes.h"



class USkeleton;



/**
Class to check if a skeleton is matching a desired hierarchy.

@attention To be used within a single method's stack space.
*/
class FSkeletonMatcher
{
public:
	/**
	@param InBoneNames The expected bone names.
	@param InMinimumMatchingPerc A skeleton is matching if it has at least X% of the expected bones. The value is in [0, 1].
	*/
	FSkeletonMatcher(const TArray<FName> & InBoneNames, float InMinimumMatchingPerc);
	
	bool IsMatching(const USkeleton * Skeleton) const;

private:
	const TArray<FName> BoneNames;
	const float MinimumMatchingPerc;
};
