// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "SMGraphK2Node_RuntimeNodeContainer.h"
#include "Graph/Nodes/Helpers/SMGraphK2Node_FunctionNodes_TransitionInstance.h"

#include "SMGraphK2Node_TransitionEnteredNode.generated.h"

UCLASS(MinimalAPI)
class USMGraphK2Node_TransitionEnteredNode : public USMGraphK2Node_RuntimeNodeReference
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual void AllocateDefaultPins() override;
	virtual void PostPlacedNewNode() override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	/** Required to show up in BP right click context menu. */
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	/** Limit blueprints this shows up in. */
	virtual bool IsActionFilteredOut(class FBlueprintActionFilter const& Filter) override;
	virtual bool IsCompatibleWithGraph(UEdGraph const* Graph) const override;
	/** User can replace node. */
	virtual bool CanUserDeleteNode() const override { return true; }
	// ~UEdGraphNode

	// USMGraphK2Node_RuntimeNode_Base
	virtual bool IsCompatibleWithInstanceGraphNodeClass(TSubclassOf<USMGraphK2Node_FunctionNode_NodeInstance> InGraphNodeClass) const override
	{
		return InGraphNodeClass == USMGraphK2Node_TransitionInstance_OnTransitionTaken::StaticClass() ||
			InGraphNodeClass == USMGraphK2Node_ConduitInstance_OnConduitEntered::StaticClass();
	}
	virtual bool IsConsideredForEntryConnection() const override { return true; }
	// ~USMGraphK2Node_RuntimeNode_Base
};
