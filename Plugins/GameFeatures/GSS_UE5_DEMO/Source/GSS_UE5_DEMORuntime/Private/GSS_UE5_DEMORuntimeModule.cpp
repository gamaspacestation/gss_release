// Copyright Epic Games, Inc. All Rights Reserved.

#include "GSS_UE5_DEMORuntimeModule.h"

#define LOCTEXT_NAMESPACE "FGSS_UE5_DEMORuntimeModule"

void FGSS_UE5_DEMORuntimeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory;
	// the exact timing is specified in the .uplugin file per-module
}

void FGSS_UE5_DEMORuntimeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.
	// For modules that support dynamic reloading, we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGSS_UE5_DEMORuntimeModule, GSS_UE5_DEMORuntime)
