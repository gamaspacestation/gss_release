// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "Graph/Nodes/PropertyNodes/SMGraphK2Node_PropertyNode.h"

#include "SMGraphK2Node_GraphPropertyNode.generated.h"

UCLASS(MinimalAPI)
class USMGraphK2Node_GraphPropertyNode : public USMGraphK2Node_PropertyNode_Base
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY()
	FSMGraphProperty_Runtime RuntimeGraphProperty;
	
	UPROPERTY(EditAnywhere, Category = "Graph Property")
	FSMGraphProperty GraphProperty;
	
	// UEdGraphNode
	virtual void AllocateDefaultPins() override;
	// ~UEdGraphNode
	
	// USMGraphK2Node_PropertyNode
	virtual void ConfigureRuntimePropertyNode() override;
	virtual FSMGraphProperty_Base_Runtime* GetRuntimePropertyNode() override;
	virtual FSMGraphProperty_Base* GetPropertyNode() override { return &GraphProperty; }
	virtual void SetPropertyNode(FSMGraphProperty_Base* NewNode) override { GraphProperty = *(FSMGraphProperty*)NewNode; }
	virtual TSharedPtr<SSMGraphProperty_Base> GetGraphNodeWidget() const override;
	virtual bool IsConsideredForDefaultProperty() const override { return GraphProperty.WidgetInfo.bConsiderForDefaultWidget; }
	virtual void DefaultPropertyActionWhenPlaced(TSharedPtr<SWidget> Widget) override;
	// ~USMGraphK2Node_PropertyNode
};