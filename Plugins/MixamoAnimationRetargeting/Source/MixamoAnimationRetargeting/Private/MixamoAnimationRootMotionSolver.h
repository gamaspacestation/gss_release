// Copyright 2022 UNAmedia. All Rights Reserved.

#pragma once

#include "Engine/EngineTypes.h"



class USkeleton;
class UAnimSequence;



class FMixamoAnimationRootMotionSolver
{
public:

    void LaunchProcedureFlow(USkeleton* Skeleton);

    bool CanExecuteProcedure(const USkeleton* Skeleton) const;

private:

    bool ExecuteExtraction(UAnimSequence* AnimSequence, const UAnimSequence* InPlaceAnimSequence);

    static float GetMaxBoneDisplacement(const UAnimSequence* AnimSequence, const FName& BoneName);
    static const UAnimSequence* EstimateInPlaceAnimation(const UAnimSequence* AnimationA, const UAnimSequence* AnimationB);

};
