// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "SMGraphNode_StateMachineEntryNode.h"
#include "Graph/Schema/SMGraphSchema.h"

#include "States/SMStateInstance.h"

#include "EdGraph/EdGraph.h"

#define LOCTEXT_NAMESPACE "SMStateMachineEntryNode"

USMGraphNode_StateMachineEntryNode::USMGraphNode_StateMachineEntryNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), bAllowParallelEntryStates(false)
{
	bCanRenameNode = false;
	StateMachineNode.GenerateNewNodeGuidIfNotSet();
}

void USMGraphNode_StateMachineEntryNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Entry"));
}

FText USMGraphNode_StateMachineEntryNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	UEdGraph* Graph = GetGraph();
	return FText::FromString(Graph->GetName());
}

FText USMGraphNode_StateMachineEntryNode::GetTooltipText() const
{
	return LOCTEXT("StateMachineEntryNodeTooltip", "Entry point for state machine");
}

void USMGraphNode_StateMachineEntryNode::PostPasteNode()
{
	Super::PostPasteNode();
	StateMachineNode.GenerateNewNodeGuid();
}

void USMGraphNode_StateMachineEntryNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(USMGraphNode_StateMachineEntryNode, bAllowParallelEntryStates))
	{
		// Break all except the first connection if we are turning off multiple state entry points.
		if (!bAllowParallelEntryStates)
		{
			const int32 Idx = 1;
			while (Idx < GetOutputPin()->LinkedTo.Num())
			{
				GetSchema()->BreakPinLinks(*GetOutputPin()->LinkedTo[Idx], true);
			}
		}
	}
}

UClass* USMGraphNode_StateMachineEntryNode::GetNodeClass() const
{
	return USMEntryStateInstance::StaticClass();
}

void USMGraphK2Node_StateMachineEntryNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
}

USMGraphK2Node_StateMachineEntryNode::USMGraphK2Node_StateMachineEntryNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCanRenameNode = false;
	StateMachineNode.GenerateNewNodeGuidIfNotSet();
}

#undef LOCTEXT_NAMESPACE
