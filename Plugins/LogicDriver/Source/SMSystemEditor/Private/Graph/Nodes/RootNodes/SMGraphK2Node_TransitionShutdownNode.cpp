// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "SMGraphK2Node_TransitionShutdownNode.h"
#include "Graph/Schema/SMGraphK2Schema.h"
#include "Graph/SMTransitionGraph.h"
#include "Graph/SMConduitGraph.h"
#include "Graph/SMStateGraph.h"
#include "Utilities/SMBlueprintEditorUtils.h"

#include "Blueprints/SMBlueprint.h"

#include "EdGraph/EdGraph.h"
#include "BlueprintNodeSpawner.h"
#include "BlueprintActionDatabaseRegistrar.h"

#define LOCTEXT_NAMESPACE "SMTransitionShutdownNode"

USMGraphK2Node_TransitionShutdownNode::USMGraphK2Node_TransitionShutdownNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAllowMoreThanOneNode = true;
}

void USMGraphK2Node_TransitionShutdownNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, USMGraphK2Schema::PC_Exec, UEdGraphSchema_K2::PN_Then);
}

void USMGraphK2Node_TransitionShutdownNode::PostPlacedNewNode()
{
	RuntimeNodeGuid = GetRuntimeContainerChecked()->GetRunTimeNodeChecked()->GetNodeGuid();
}

FText USMGraphK2Node_TransitionShutdownNode::GetMenuCategory() const
{
	return FText::FromString(STATE_MACHINE_HELPER_CATEGORY);
}

FText USMGraphK2Node_TransitionShutdownNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UEdGraph* TopLevelGraph = FSMBlueprintEditorUtils::FindTopLevelOwningGraph(GetGraph());
	const bool bIsTransition = Cast<USMTransitionGraph>(TopLevelGraph) != nullptr;
	const bool bIsConduit = !bIsTransition && Cast<USMConduitGraph>(TopLevelGraph) != nullptr;
	
	if ((TitleType == ENodeTitleType::MenuTitle || TitleType == ENodeTitleType::ListView))
	{
		return LOCTEXT("AddNodeShutdownEvent", "Add Event On Node Shutdown");
	}

	return FText::FromString(bIsTransition ? TEXT("On Transition Shutdown") : bIsConduit ? TEXT("On Conduit Shutdown") : TEXT("On State Shutdown"));
}

FText USMGraphK2Node_TransitionShutdownNode::GetTooltipText() const
{
	return LOCTEXT("TransitionShutdownNodeTooltip", "For transitions and conduits: Called after the state leading to this node has run OnStateEnd but before it has called its shutdown sequence.\
\nFor states: Called after OnStateEnd and after transitions are shutdown.");
}

void USMGraphK2Node_TransitionShutdownNode::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();

	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

bool USMGraphK2Node_TransitionShutdownNode::IsActionFilteredOut(FBlueprintActionFilter const& Filter)
{
	for (UBlueprint* Blueprint : Filter.Context.Blueprints)
	{
		if (!Cast<USMBlueprint>(Blueprint))
		{
			return true;
		}
	}

	for (UEdGraph* Graph : Filter.Context.Graphs)
	{
		// Only works on transition and conduit graphs.
		if (!Graph->IsA<USMTransitionGraph>() && !Graph->IsA<USMConduitGraph>() && !Graph->IsA<USMStateGraph>())
		{
			return true;
		}
	}

	return false;
}

bool USMGraphK2Node_TransitionShutdownNode::IsCompatibleWithGraph(UEdGraph const* Graph) const
{
	return Graph->IsA<USMTransitionGraph>() || Graph->IsA<USMConduitGraph>() || Graph->IsA<USMStateGraph>();
}

#undef LOCTEXT_NAMESPACE
