// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "SMTestHelpers.h"
#include "SMTestContext.h"
#include "Helpers/SMTestBoilerplate.h"

#include "Blueprints/SMBlueprint.h"

#include "Blueprints/SMBlueprintFactory.h"
#include "Utilities/SMBlueprintEditorUtils.h"
#include "Graph/SMGraph.h"
#include "Graph/SMStateGraph.h"
#include "Graph/Nodes/SMGraphK2Node_StateMachineNode.h"
#include "Graph/Nodes/SMGraphNode_StateNode.h"
#include "Graph/Nodes/SMGraphNode_StateMachineEntryNode.h"
#include "Graph/Nodes/SMGraphNode_TransitionEdge.h"
#include "Graph/Nodes/SMGraphNode_StateMachineStateNode.h"
#include "Graph/Nodes/SMGraphNode_ConduitNode.h"

#include "Kismet2/KismetEditorUtilities.h"

#if WITH_DEV_AUTOMATION_TESTS

#if PLATFORM_DESKTOP

/**
 * Test methods to retrieve or set states by qualified name.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStateMachineInstanceMethodsTest, "LogicDriver.StateMachineNodeClass.Methods", EAutomationTestFlags::ApplicationContextMask |
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FStateMachineInstanceMethodsTest::RunTest(const FString& Parameters)
{
	SETUP_NEW_STATE_MACHINE_FOR_TEST(2)
	
	UEdGraphPin* LastStatePin = nullptr;
	TestHelpers::BuildLinearStateMachine(this, StateMachineGraph, TotalStates, &LastStatePin);

	const int32 NestedStateCount = 3;
	const USMGraphNode_StateMachineStateNode* NestedFSMNode = TestHelpers::BuildNestedStateMachine(this, StateMachineGraph, NestedStateCount, &LastStatePin, nullptr);

	LastStatePin = NestedFSMNode->GetOutputPin();
	USMGraphNode_StateMachineStateNode* NestedFSMRefNode = TestHelpers::BuildNestedStateMachine(this, StateMachineGraph, NestedStateCount, &LastStatePin, nullptr);

	USMBlueprint* NewReferencedBlueprint = FSMBlueprintEditorUtils::ConvertStateMachineToReference(NestedFSMRefNode, false, nullptr, nullptr);
	FAssetHandler ReferencedAsset = TestHelpers::CreateAssetFromBlueprint(NewReferencedBlueprint);
	FKismetEditorUtilities::CompileBlueprint(NewReferencedBlueprint);
	
	const USMInstance* Instance = TestHelpers::CompileAndCreateStateMachineInstanceFromBP(NewBP);

	TArray<USMStateInstance_Base*> EntryStates;
	Instance->GetRootStateMachineNodeInstance()->GetEntryStates(EntryStates);
	check(EntryStates.Num() == 1);

	TArray<USMStateInstance_Base*> AllStatesToCheck;
	{
		USMStateInstance_Base* EntryState = EntryStates[0];
		USMStateInstance_Base* SecondState = EntryState->GetNextStateByTransitionIndex(0);
		USMStateMachineInstance* NestedStateMachine = CastChecked<USMStateMachineInstance>(SecondState->GetNextStateByTransitionIndex(0));
		USMStateMachineInstance* NestedStateMachineRef = CastChecked<USMStateMachineInstance>(NestedStateMachine->GetNextStateByTransitionIndex(0));

		TArray<USMStateInstance_Base*> NestedEntryStates;
		NestedStateMachine->GetEntryStates(NestedEntryStates);
		check(NestedEntryStates.Num() == 1);
	
		USMStateInstance_Base* NestedEntryState = NestedEntryStates[0];
		USMStateInstance_Base* SecondNestedState = NestedEntryState->GetNextStateByTransitionIndex(0);
		USMStateInstance_Base* ThirdNestedState = SecondNestedState->GetNextStateByTransitionIndex(0);
		check(ThirdNestedState);

		TArray<USMStateInstance_Base*> NestedRefEntryStates;
		NestedStateMachineRef->GetEntryStates(NestedRefEntryStates);
		check(NestedRefEntryStates.Num() == 1);
	
		USMStateInstance_Base* NestedRefEntryState = NestedRefEntryStates[0];
		USMStateInstance_Base* SecondRefNestedState = NestedRefEntryState->GetNextStateByTransitionIndex(0);
		USMStateInstance_Base* ThirdRefNestedState = SecondRefNestedState->GetNextStateByTransitionIndex(0);
		check(ThirdRefNestedState);
		
		AllStatesToCheck.Append({EntryState, SecondState,
			NestedStateMachine, NestedEntryState, SecondNestedState, ThirdNestedState,
			NestedStateMachineRef, NestedRefEntryState,  SecondRefNestedState, ThirdRefNestedState});
	}

	auto TestStateMachineInstance = [&](const USMStateMachineInstance* InStateMachineNodeInstance, const int32 InExpectedStateCount)
	{
		TArray<USMStateInstance_Base*> AllStateInstances;
		InStateMachineNodeInstance->GetAllStateInstances(AllStateInstances);

		TestEqual("Top level states found", AllStateInstances.Num(), InExpectedStateCount);

		for (const USMStateInstance_Base* State : AllStateInstances)
		{
			USMStateInstance_Base* FoundState = InStateMachineNodeInstance->GetContainedStateByName(State->GetNodeName());
			TestNotNull("State found by name", FoundState);
		}

		TArray<USMStateInstance_Base*> FoundEntryStates;
		InStateMachineNodeInstance->GetEntryStates(FoundEntryStates);
		TestEqual("Entry state found", FoundEntryStates.Num(), 1);
	};
	
	TestStateMachineInstance(Instance->GetRootStateMachineNodeInstance(), 4);

	for (USMStateInstance_Base* State : AllStatesToCheck)
	{
		if (const USMStateMachineInstance* StateMachineNodeInstance = Cast<USMStateMachineInstance>(State))
		{
			TestStateMachineInstance(StateMachineNodeInstance, NestedStateCount);
		}
	}
	
	ReferencedAsset.DeleteAsset(this);
	return NewAsset.DeleteAsset(this);
}

#endif

#endif //WITH_DEV_AUTOMATION_TESTS