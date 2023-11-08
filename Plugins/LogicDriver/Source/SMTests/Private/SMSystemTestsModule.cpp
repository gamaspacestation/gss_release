// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "SMTests/Public/SMSystemTestsModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "SMSystemTests"

void FSMSystemTestsModule::StartupModule()
{
}


void FSMSystemTestsModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSMSystemTestsModule, SMSystemTests);