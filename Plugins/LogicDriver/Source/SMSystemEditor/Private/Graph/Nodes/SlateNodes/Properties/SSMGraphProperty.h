// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "SMNodeWidgetInfo.h"

#include "SGraphPin.h"
#include "SKismetLinearExpression.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBox.h"

/**
 * Base representation of an exposed graph property. Extend this to implement custom graph properties.
 */
class SMSYSTEMEDITOR_API SSMGraphProperty_Base : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SSMGraphProperty_Base)
		: _GraphNode(nullptr)
	{
	}

	/** Graph node containing the property. */
	SLATE_ARGUMENT(class UEdGraphNode*, GraphNode)
	SLATE_END_ARGS()

	SSMGraphProperty_Base();

	virtual void Finalize() {}
	virtual void Refresh() {}
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;

	TWeakObjectPtr<UEdGraphNode> GetGraphNode() const { return GraphNode; }
	TSharedPtr<SGraphNode> FindParentGraphNode() const;
	UEdGraphPin* FindResultPin() const;

protected:
	TSharedRef<SBorder> MakeHighlightBorder();
	TSharedRef<SWidget> MakeNotifyIconWidget();

	EVisibility GetHighlightVisibility() const;
	FSlateColor GetHighlightColor() const;

	EVisibility GetNotifyVisibility() const;
	const FSlateBrush* GetNotifyIconBrush() const;
	FText GetNotifyIconTooltip() const;

protected:
	TWeakObjectPtr<UEdGraphNode> GraphNode;
	static constexpr float HighlightPadding = -6.f;
	static FMargin NotifyPadding;
};

/**
 * Visual representation of an exposed graph property.
 */
class SSMGraphProperty : public SSMGraphProperty_Base
{
	SLATE_BEGIN_ARGS(SSMGraphProperty)
		: _GraphNode(nullptr)
		, _WidgetInfo(nullptr)
	{
	}

	/** Graph node containing the property. */
	SLATE_ARGUMENT(class UEdGraphNode*, GraphNode)
	SLATE_ARGUMENT(const FSMTextDisplayWidgetInfo*, WidgetInfo)
	SLATE_END_ARGS()
	
	SSMGraphProperty();
	virtual ~SSMGraphProperty() override;
	
	void Construct(const FArguments& InArgs);
	virtual void Finalize() override;

	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;

	virtual void Refresh() override;
	
protected:
	/** Validates that the drag drop event is allowed for this class. */
	bool IsDragDropValid(const FDragDropEvent& DragDropEvent) const;
	void HandleExpressionChange(class UEdGraphPin* ResultPin);
	FSlateColor GetBackgroundColor() const;

protected:
	TSharedPtr<SKismetLinearExpression> ExpressionWidget;
	TSharedPtr<SBox> InputPinContainer;
	TWeakPtr<SGraphPin> InputPinPtr;
	
	FSMTextDisplayWidgetInfo WidgetInfo;
	bool bIsValidDragDrop;
};
