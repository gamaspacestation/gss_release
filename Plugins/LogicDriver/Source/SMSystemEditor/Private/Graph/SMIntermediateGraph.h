// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "Nodes/RootNodes/SMGraphK2Node_IntermediateNodes.h"
#include "SMStateGraph.h"

#include "CoreMinimal.h"

#include "SMIntermediateGraph.generated.h"

class USMGraphNode_StateNode;

UCLASS(MinimalAPI)
class USMIntermediateGraph : public USMStateGraph
{
	GENERATED_UCLASS_BODY()

	// USMGraphK2
	virtual FSMNode_Base* GetRuntimeNode() const override { return IntermediateEntryNode ? IntermediateEntryNode->GetRunTimeNode() : nullptr; }
	// ~USMGraphK2
public:
	UPROPERTY()
	class USMGraphK2Node_IntermediateEntryNode* IntermediateEntryNode;
};
