// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Graph/Nodes/SMGraphK2Node_Base.h"

#include "SMGraphK2Node_RootNode.generated.h"

UCLASS()
class SMSYSTEMEDITOR_API USMGraphK2Node_RootNode : public USMGraphK2Node_Base
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual bool CanUserDeleteNode() const override { return false; }
	virtual bool CanDuplicateNode() const override { return false; }
	virtual void PostPasteNode() override;
	virtual void DestroyNode() override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	// ~UEdGraphNode

	// USMGraphK2Node_Base
	virtual bool CanCollapseNode() const override { return true; }
	virtual bool CanCollapseToFunctionOrMacro() const override { return false; }
	// ~USMGraphK2Node_Base

protected:
	// If this node is in the process of being destroyed.
	bool bIsBeingDestroyed;

	// If this node can be placed more than once on the same graph.
	bool bAllowMoreThanOneNode;
};
