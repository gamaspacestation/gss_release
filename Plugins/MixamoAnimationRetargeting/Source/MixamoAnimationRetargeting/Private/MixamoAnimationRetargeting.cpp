// Copyright 2022 UNAmedia. All Rights Reserved.

#include "MixamoAnimationRetargeting.h"
#include "MixamoToolkitPrivatePCH.h"

#include "MixamoToolkitPrivate.h"

#include "MixamoToolkitStyle.h"
#include "MixamoToolkitCommands.h"
#include "MixamoToolkitEditorIntegration.h"

#include "MixamoSkeletonRetargeter.h"
#include "MixamoAnimationRootMotionSolver.h"

#include "MessageLogModule.h"


#define LOCTEXT_NAMESPACE "FMixamoAnimationRetargetingModule"



DEFINE_LOG_CATEGORY(LogMixamoToolkit)



FMixamoAnimationRetargetingModule & FMixamoAnimationRetargetingModule::Get()
{
	static FName MixamoToolkitModuleName("MixamoAnimationRetargeting");
	return FModuleManager::Get().LoadModuleChecked<FMixamoAnimationRetargetingModule>(MixamoToolkitModuleName);
}



void FMixamoAnimationRetargetingModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	MixamoSkeletonRetargeter = MakeShareable(new FMixamoSkeletonRetargeter());
	MixamoAnimationRootMotionSolver = MakeShareable(new FMixamoAnimationRootMotionSolver());

	// Register Slate style ovverides
	FMixamoToolkitStyle::Initialize();
	FMixamoToolkitStyle::ReloadTextures();
	
	// === Register commands.
	FMixamoToolkitCommands::Register();

	EditorIntegration = MakeShareable(new FMixamoToolkitEditorIntegration());
	EditorIntegration->Register();

	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	MessageLogModule.RegisterLogListing(FName("LogMixamoToolkit"), LOCTEXT("MixamoRetargeting", "Mixamo Retargeting Log"));
}



void FMixamoAnimationRetargetingModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	EditorIntegration->Unregister();
	EditorIntegration.Reset();

	FMixamoToolkitCommands::Unregister();
	FMixamoToolkitStyle::Shutdown();

	MixamoAnimationRootMotionSolver.Reset ();
	MixamoSkeletonRetargeter.Reset ();
}



TSharedRef<FMixamoSkeletonRetargeter> FMixamoAnimationRetargetingModule::GetMixamoSkeletonRetargeter()
{
	return MixamoSkeletonRetargeter.ToSharedRef();
}



TSharedRef<FMixamoAnimationRootMotionSolver> FMixamoAnimationRetargetingModule::GetMixamoAnimationRootMotionSolver()
{
    return MixamoAnimationRootMotionSolver.ToSharedRef();
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMixamoAnimationRetargetingModule, MixamoAnimationRetargeting)