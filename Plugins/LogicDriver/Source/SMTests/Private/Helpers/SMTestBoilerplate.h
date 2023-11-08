// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "SMTestHelpers.h"

#include "Graph/SMGraph.h"
#include "Graph/Nodes/SMGraphK2Node_StateMachineNode.h"
#include "Utilities/SMBlueprintEditorUtils.h"

#include "Blueprints/SMBlueprint.h"

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

static bool SetupTest(FAutomationTestBase* InTest, FAssetHandler& OutNewAsset, USMBlueprint** OutNewBP, USMGraphK2Node_StateMachineNode** OutRootStateMachineNode, USMGraph** OutStateMachineGraph)
{
	if (!TestHelpers::TryCreateNewStateMachineAsset(InTest, OutNewAsset, false))
	{
		return false;
	}
	*OutNewBP = OutNewAsset.GetObjectAs<USMBlueprint>();
	*OutRootStateMachineNode = FSMBlueprintEditorUtils::GetRootStateMachineNode(*OutNewBP);
	*OutStateMachineGraph = (*OutRootStateMachineNode)->GetStateMachineGraph();
	return true;
}

/** Run for duration of a test. */
struct FSMTestScopeHelper
{
	FSMTestScopeHelper()
	{
		FSMNode_Base::bValidateGuids = true;
	}

	~FSMTestScopeHelper()
	{
		FSMNode_Base::bValidateGuids = false;
	}
};

#define SETUP_NEW_STATE_MACHINE_FOR_TEST_NO_STATES() \
FAssetHandler NewAsset; \
USMBlueprint* NewBP = nullptr; \
USMGraphK2Node_StateMachineNode* RootStateMachineNode = nullptr; \
USMGraph* StateMachineGraph = nullptr; \
if (!SetupTest(this, NewAsset, &NewBP, &RootStateMachineNode, &StateMachineGraph)) \
{ \
	return false; \
} \

#define SETUP_NEW_STATE_MACHINE_FOR_TEST(num_states) \
SETUP_NEW_STATE_MACHINE_FOR_TEST_NO_STATES() \
int32 TotalStates = num_states; \
FSMTestScopeHelper SMTestScopeHelper; \

#endif