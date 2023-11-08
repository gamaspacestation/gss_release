// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "SMGraphK2.h"
#include "Nodes/RootNodes/SMGraphK2Node_StateEntryNode.h"
#include "Nodes/SMGraphNode_StateNode.h"

#include "SMStateGraph.generated.h"

class USMGraphNode_StateNode;

UCLASS(MinimalAPI)
class USMStateGraph : public USMGraphK2
{
	GENERATED_UCLASS_BODY()

public:
	virtual USMGraphNode_StateNodeBase* GetOwningStateNode() const { return Cast<USMGraphNode_StateNodeBase>(GetOuter()); }

	// USMGraphK2
	virtual FSMNode_Base* GetRuntimeNode() const override { return EntryNode ? EntryNode->GetRunTimeNode() : nullptr; }
	// ~USMGraphK2
public:
	UPROPERTY()
	class USMGraphK2Node_StateEntryNode* EntryNode;
};
