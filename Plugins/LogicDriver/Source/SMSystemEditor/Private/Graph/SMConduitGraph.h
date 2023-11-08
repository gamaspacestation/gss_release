// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "SMGraphK2.h"
#include "Nodes/RootNodes/SMGraphK2Node_ConduitResultNode.h"

#include "SMConduitGraph.generated.h"

UCLASS(MinimalAPI)
class USMConduitGraph : public USMGraphK2
{
	GENERATED_UCLASS_BODY()

public:
	// USMGraphK2
	virtual bool HasAnyLogicConnections() const override;
	virtual FSMNode_Base* GetRuntimeNode() const override { return ResultNode ? ResultNode->GetRunTimeNode() : nullptr; }
	// ~USMGraphK2
	
	/** Determine if the graph should be evaluated at runtime or can be statically known. */
	SMSYSTEMEDITOR_API ESMConditionalEvaluationType GetConditionalEvaluationType() const;

public:
	UPROPERTY()
	USMGraphK2Node_ConduitResultNode* ResultNode;
};
