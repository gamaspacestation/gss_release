// Copyright 2022 UNAmedia. All Rights Reserved.

#include "MixamoToolkitCommands.h"
#include "MixamoToolkitPrivatePCH.h"

#include "MixamoToolkitStyle.h"

#define LOCTEXT_NAMESPACE "FMixamoAnimationRetargetingModule"



FMixamoToolkitCommands::FMixamoToolkitCommands()
	: TCommands<FMixamoToolkitCommands>(
		TEXT("MixamoAnimationRetargeting"),		// Context name for fast lookup
		NSLOCTEXT(LOCTEXT_NAMESPACE, "MixamoAnimationRetargetingCommands", "Mixamo Animation Retargeting Plugin"),
		NAME_None,		// Parent
		FMixamoToolkitStyle::GetStyleSetName()		// Icon Style Set
	  )
{
}



void FMixamoToolkitCommands::RegisterCommands()
{
	//UI_COMMAND(OpenBatchConverterWindow, "Mixamo batch helper", "Open the batch helper for Mixamo assets", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(RetargetMixamoSkeleton, "Retarget Mixamo Skeleton Asset", "Retarget Mixamo Skeleton Assets", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ExtractRootMotion, "Generate Root Motion Animations", "Generate Root Motion Animations", EUserInterfaceActionType::Button, FInputChord());
}



#undef LOCTEXT_NAMESPACE
