// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "SMEditorInstance.h"

#include "SMUtils.h"

#define LOCTEXT_NAMESPACE "SMEditorInstance"

void USMEditorInstance::Initialize(UObject* Context)
{
	const TSet<FStructProperty*> Properties;
	USMUtils::GenerateStateMachine(this, GetRootStateMachine(), Properties);

	TMap<FString, int32> Paths;
	RootStateMachine.CalculatePathGuid(Paths);

	BuildStateMachineMap(&RootStateMachine);

	bInitialized = true;
}

void USMEditorInstance::Shutdown()
{
	Super::Shutdown();
	RootStateMachine = FSMStateMachine();
}

#undef LOCTEXT_NAMESPACE
