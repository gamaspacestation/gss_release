// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "SMGraphK2Node_StateEndNode.h"
#include "Graph/Schema/SMGraphSchema.h"
#include "Graph/SMStateGraph.h"

#include "EdGraph/EdGraph.h"

#define LOCTEXT_NAMESPACE "SMStateEndNode"

USMGraphK2Node_StateEndNode::USMGraphK2Node_StateEndNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USMGraphK2Node_StateEndNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
}

FText USMGraphK2Node_StateEndNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("On State End"));
}

FText USMGraphK2Node_StateEndNode::GetTooltipText() const
{
	return LOCTEXT("StateEndNodeTooltip", "Called when the state completes. It is not advised to switch states during this event.\
\nThe state machine will already be in the process of switching states.");
}

bool USMGraphK2Node_StateEndNode::IsCompatibleWithGraph(UEdGraph const* Graph) const
{
	return Graph->IsA<USMStateGraph>();
}

#undef LOCTEXT_NAMESPACE
