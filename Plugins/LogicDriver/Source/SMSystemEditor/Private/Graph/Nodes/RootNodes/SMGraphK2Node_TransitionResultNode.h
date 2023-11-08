// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "SMTransition.h"
#include "SMGraphK2Node_RuntimeNodeContainer.h"
#include "Graph/Nodes/Helpers/SMGraphK2Node_FunctionNodes_TransitionInstance.h"

#include "CoreMinimal.h"

#include "SMGraphK2Node_TransitionResultNode.generated.h"

UCLASS()
class SMSYSTEMEDITOR_API USMGraphK2Node_TransitionResultNode : public USMGraphK2Node_RuntimeNodeContainer
{
	GENERATED_UCLASS_BODY()

	static const FName EvalPinName;
	
	UPROPERTY(EditAnywhere, Category = "State Machines")
	FSMTransition TransitionNode;

	// UEdGraphNode
	virtual void AllocateDefaultPins() override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;

	virtual bool IsNodePure() const override { return true; }
	virtual bool IsCompatibleWithGraph(UEdGraph const* Graph) const override;
	// ~UEdGraphNode

	// USMGraphK2Node_RootNode
	virtual bool IsCompatibleWithInstanceGraphNodeClass(TSubclassOf<USMGraphK2Node_FunctionNode_NodeInstance> InGraphNodeClass) const override
	{
		return InGraphNodeClass == USMGraphK2Node_TransitionInstance_CanEnterTransition::StaticClass() ||
			USMGraphK2Node_ConduitInstance_CanEnterTransition::StaticClass();
	}
	virtual UEdGraphPin* GetCorrectEntryPin() const override;
	virtual UEdGraphPin* GetCorrectNodeInstanceOutputPin(USMGraphK2Node_FunctionNode_NodeInstance* InInstance) const override;
	virtual bool IsConsideredForEntryConnection() const override { return true; }
	// ~USMGraphK2Node_RootNode
	
	virtual FSMNode_Base* GetRunTimeNode() override { return &TransitionNode; }
	UEdGraphPin* GetTransitionEvaluationPin() const;
};
