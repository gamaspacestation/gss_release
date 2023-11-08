// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "SMGraphK2Node_StateUpdateNode.h"
#include "Graph/Schema/SMGraphK2Schema.h"
#include "Graph/SMStateGraph.h"

#include "EdGraph/EdGraph.h"

#define LOCTEXT_NAMESPACE "SMStateUpdateNode"

USMGraphK2Node_StateUpdateNode::USMGraphK2Node_StateUpdateNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USMGraphK2Node_StateUpdateNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, USMGraphK2Schema::PC_Exec, UEdGraphSchema_K2::PN_Then);
	CreatePin(EGPD_Output, USMGraphK2Schema::PC_Real, USMGraphK2Schema::PC_Float, TEXT("DeltaSeconds"));
}

FText USMGraphK2Node_StateUpdateNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("On State Update"));
}

FText USMGraphK2Node_StateUpdateNode::GetTooltipText() const
{
	return LOCTEXT("StateUpdateNodeTooltip", "Called when the state updates.");
}

bool USMGraphK2Node_StateUpdateNode::IsCompatibleWithGraph(UEdGraph const* Graph) const
{
	return Graph->IsA<USMStateGraph>();
}

#undef LOCTEXT_NAMESPACE
