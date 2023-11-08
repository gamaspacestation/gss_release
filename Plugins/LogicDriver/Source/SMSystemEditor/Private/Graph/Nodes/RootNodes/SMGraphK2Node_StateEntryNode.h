// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "SMGraphK2Node_RuntimeNodeContainer.h"
#include "Graph/Nodes/Helpers/SMGraphK2Node_FunctionNodes_NodeInstance.h"

#include "SMState.h"

#include "SMGraphK2Node_StateEntryNode.generated.h"

UCLASS(MinimalAPI)
class USMGraphK2Node_StateEntryNode : public USMGraphK2Node_RuntimeNodeContainer
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category = "State Machines")
	FSMState StateNode;

	// UEdGraphNode
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual bool IsCompatibleWithGraph(UEdGraph const* Graph) const override;
	// ~UEdGraphNode
	
	// USMGraphK2Node_RuntimeNode_Base
    virtual bool IsCompatibleWithInstanceGraphNodeClass(TSubclassOf<USMGraphK2Node_FunctionNode_NodeInstance> InGraphNodeClass) const override
    {
	    return InGraphNodeClass == USMGraphK2Node_StateInstance_Begin::StaticClass();
    }
    // ~USMGraphK2Node_RuntimeNode_Base
	
	// USMGraphK2Node_RuntimeNodeContainer
	virtual FSMNode_Base* GetRunTimeNode() override { return &StateNode; }
	// ~USMGraphK2Node_RuntimeNodeContainer
};
