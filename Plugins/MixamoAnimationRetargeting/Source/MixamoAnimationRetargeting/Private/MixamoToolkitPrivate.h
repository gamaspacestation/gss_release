// Copyright 2022 UNAmedia. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"


DECLARE_LOG_CATEGORY_EXTERN(LogMixamoToolkit, Warning, All)



class FMixamoAnimationRetargetingModule :
	public IModuleInterface,
	public TSharedFromThis<FMixamoAnimationRetargetingModule>
{
public:
	static
	FMixamoAnimationRetargetingModule & Get();

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedRef<class FMixamoSkeletonRetargeter> GetMixamoSkeletonRetargeter();
	TSharedRef<class FMixamoAnimationRootMotionSolver> GetMixamoAnimationRootMotionSolver();

private:
	TSharedPtr<class FMixamoSkeletonRetargeter> MixamoSkeletonRetargeter;
	TSharedPtr<class FMixamoAnimationRootMotionSolver> MixamoAnimationRootMotionSolver;
	TSharedPtr<class FMixamoToolkitEditorIntegration> EditorIntegration;
};