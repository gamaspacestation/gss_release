// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "SMGraphK2Node_StateEntryNode.h"
#include "Graph/Schema/SMGraphSchema.h"
#include "Graph/SMStateGraph.h"

#include "EdGraph/EdGraph.h"

#define LOCTEXT_NAMESPACE "SMStateEntryNode"

USMGraphK2Node_StateEntryNode::USMGraphK2Node_StateEntryNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StateNode.GenerateNewNodeGuidIfNotSet();
}

void USMGraphK2Node_StateEntryNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
}

FText USMGraphK2Node_StateEntryNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("On State Begin"));
}

FText USMGraphK2Node_StateEntryNode::GetTooltipText() const
{
	return LOCTEXT("StateEntryNodeTooltip", "Entry point for state.");
}

bool USMGraphK2Node_StateEntryNode::IsCompatibleWithGraph(UEdGraph const* Graph) const
{
	return Graph->IsA<USMStateGraph>();
}

#undef LOCTEXT_NAMESPACE
