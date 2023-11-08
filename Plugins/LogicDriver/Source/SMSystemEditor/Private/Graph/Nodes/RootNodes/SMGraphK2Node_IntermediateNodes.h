// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "Graph/Nodes/SMGraphNode_StateMachineEntryNode.h"
#include "Graph/Nodes/Helpers/SMGraphK2Node_FunctionNodes_NodeInstance.h"

#include "SMGraphK2Node_IntermediateNodes.generated.h"

/**
 * State Start override for intermediate graphs.
 */
UCLASS(MinimalAPI)
class USMGraphK2Node_IntermediateEntryNode : public USMGraphK2Node_StateMachineEntryNode
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual bool IsCompatibleWithGraph(UEdGraph const* Graph) const override;
	// ~UEdGraphNode
};

/**
 * This blueprint's root State machine start entry point
 */
UCLASS(MinimalAPI)
class USMGraphK2Node_IntermediateStateMachineStartNode : public USMGraphK2Node_RuntimeNodeReference
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual bool CanUserDeleteNode() const override { return true; }
	virtual void AllocateDefaultPins() override;
	virtual ERedirectType DoPinsMatchForReconstruction(const UEdGraphPin* NewPin, int32 NewPinIndex, const UEdGraphPin* OldPin, int32 OldPinIndex) const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetMenuCategory() const override;
	virtual bool IsCompatibleWithGraph(UEdGraph const* Graph) const override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual bool IsActionFilteredOut(class FBlueprintActionFilter const& Filter) override;
	virtual void PostPlacedNewNode() override;
	// ~UEdGraphNode

	// USMGraphK2Node_RuntimeNode_Base
	virtual bool IsCompatibleWithInstanceGraphNodeClass(TSubclassOf<USMGraphK2Node_FunctionNode_NodeInstance> InGraphNodeClass) const override
	{
		return InGraphNodeClass == USMGraphK2Node_StateInstance_StateMachineStart::StaticClass();
	}
	virtual bool IsConsideredForEntryConnection() const override { return true; }
	// ~USMGraphK2Node_RuntimeNode_Base
};

/**
 * When the blueprint's root state machine stops.
 */
UCLASS(MinimalAPI)
class USMGraphK2Node_IntermediateStateMachineStopNode : public USMGraphK2Node_RuntimeNodeReference
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual bool CanUserDeleteNode() const override { return true; }
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetMenuCategory() const override;
	virtual bool IsCompatibleWithGraph(UEdGraph const* Graph) const override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual bool IsActionFilteredOut(class FBlueprintActionFilter const& Filter) override;
	virtual void PostPlacedNewNode() override;
	// ~UEdGraphNode

	// USMGraphK2Node_RuntimeNode_Base
	virtual bool IsCompatibleWithInstanceGraphNodeClass(TSubclassOf<USMGraphK2Node_FunctionNode_NodeInstance> InGraphNodeClass) const override
	{
		return InGraphNodeClass == USMGraphK2Node_StateInstance_StateMachineStop::StaticClass();
	}
	virtual bool IsConsideredForEntryConnection() const override { return true; }
	// ~USMGraphK2Node_RuntimeNode_Base
};