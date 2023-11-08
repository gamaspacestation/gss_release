// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "SSMGraphProperty.h"

#include "Configuration/SMEditorSettings.h"
#include "Configuration/SMEditorStyle.h"
#include "Graph/Nodes/PropertyNodes/SMGraphK2Node_GraphPropertyNode.h"
#include "Graph/Nodes/PropertyNodes/SMGraphK2Node_PropertyNode.h"
#include "Helpers/SMDragDropHelpers.h"
#include "Utilities/SMBlueprintEditorUtils.h"

#include "BPVariableDragDropAction.h"
#include "EditorStyleSet.h"
#include "NodeFactory.h"
#include "SMUnrealTypeDefs.h"
#include "Editor/Kismet/Private/BPFunctionDragDropAction.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"

#define LOCTEXT_NAMESPACE "SSMGraphProperty"

FMargin SSMGraphProperty_Base::NotifyPadding = FMargin(3.f, 0.f, 0.f, 0.f);

SSMGraphProperty_Base::SSMGraphProperty_Base(): GraphNode(nullptr)
{
}

void SSMGraphProperty_Base::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);
	if (USMGraphK2Node_PropertyNode_Base* Node = Cast<USMGraphK2Node_PropertyNode_Base>(GraphNode))
	{
		Node->bMouseOverNodeProperty = true;
	}
}

void SSMGraphProperty_Base::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SCompoundWidget::OnMouseLeave(MouseEvent);

	if (!MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		if (USMGraphK2Node_PropertyNode_Base* Node = Cast<USMGraphK2Node_PropertyNode_Base>(GraphNode))
		{
			Node->bMouseOverNodeProperty = false;
		}
	}
}

FReply SSMGraphProperty_Base::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	if (USMGraphK2Node_PropertyNode_Base* Node = Cast<USMGraphK2Node_PropertyNode_Base>(GraphNode))
	{
		Node->JumpToPropertyGraph();
		return FReply::Handled();
	}

	return SCompoundWidget::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

TSharedPtr<SGraphNode> SSMGraphProperty_Base::FindParentGraphNode() const
{
	for (TSharedPtr<SWidget> Parent = GetParentWidget(); Parent.IsValid(); Parent = Parent->GetParentWidget())
	{
		FString Type = Parent->GetType().ToString();
		if (Type.Contains("SGraphNode"))
		{
			return StaticCastSharedPtr<SGraphNode>(Parent);
		}
	}

	return nullptr;
}

UEdGraphPin* SSMGraphProperty_Base::FindResultPin() const
{
	if (USMGraphK2Node_PropertyNode_Base* Node = Cast<USMGraphK2Node_PropertyNode_Base>(GraphNode))
	{
		return Node->GetResultPin();
	}

	return nullptr;
}

TSharedRef<SBorder> SSMGraphProperty_Base::MakeHighlightBorder()
{
	return SNew(SBorder)
	.BorderImage(FSMEditorStyle::Get()->GetBrush("BoxHighlight"))
	.BorderBackgroundColor(this, &SSMGraphProperty::GetHighlightColor)
	.Visibility(this, &SSMGraphProperty::GetHighlightVisibility);
}

TSharedRef<SWidget> SSMGraphProperty_Base::MakeNotifyIconWidget()
{
	return SNew(SImage)
	.Image(this, &SSMGraphProperty_Base::GetNotifyIconBrush)
	.ToolTipText(this, &SSMGraphProperty_Base::GetNotifyIconTooltip)
	.Visibility(this, &SSMGraphProperty_Base::GetNotifyVisibility);
}

EVisibility SSMGraphProperty_Base::GetHighlightVisibility() const
{
	bool bVisible = false;
	if (const USMGraphK2Node_PropertyNode_Base* Node = Cast<USMGraphK2Node_PropertyNode_Base>(GraphNode))
	{
		const USMGraphK2Node_PropertyNode_Base::FHighlightArgs& Args = Node->GetHighlightArgs();
		bVisible = Args.bEnable;
	}

	return bVisible ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

FSlateColor SSMGraphProperty_Base::GetHighlightColor() const
{
	if (const USMGraphK2Node_PropertyNode_Base* Node = Cast<USMGraphK2Node_PropertyNode_Base>(GraphNode))
	{
		const USMGraphK2Node_PropertyNode_Base::FHighlightArgs& Args = Node->GetHighlightArgs();
		if (Args.bEnable)
		{
			return Node->GetHighlightArgs().Color;
		}
	}

	return FLinearColor::Transparent;
}

EVisibility SSMGraphProperty_Base::GetNotifyVisibility() const
{
	bool bVisible = false;

	if (const USMGraphK2Node_PropertyNode_Base* Node = Cast<USMGraphK2Node_PropertyNode_Base>(GraphNode))
	{
		const USMGraphK2Node_PropertyNode_Base::FNotifyArgs& Args = Node->GetNotifyArgs();
		bVisible = Args.bEnable;
	}

	return bVisible ? EVisibility::Visible : EVisibility::Collapsed;
}

const FSlateBrush* SSMGraphProperty_Base::GetNotifyIconBrush() const
{
	if (const USMGraphK2Node_PropertyNode_Base* Node = Cast<USMGraphK2Node_PropertyNode_Base>(GraphNode))
	{
		const USMGraphK2Node_PropertyNode_Base::FNotifyArgs& Args = Node->GetNotifyArgs();
		switch (Args.LogType)
		{
		case ESMLogType::Note:
			{
				return FSMUnrealAppStyle::Get().GetBrush(TEXT("Icons.Info"));
			}
		case ESMLogType::Warning:
			{
				return FSMUnrealAppStyle::Get().GetBrush(TEXT("Icons.Warning"));
			}
		case ESMLogType::Error:
			{
				return FSMUnrealAppStyle::Get().GetBrush(TEXT("Icons.Error"));
			}
		}
	}

	return FStyleDefaults::GetNoBrush();
}

FText SSMGraphProperty_Base::GetNotifyIconTooltip() const
{
	if (const USMGraphK2Node_PropertyNode_Base* Node = Cast<USMGraphK2Node_PropertyNode_Base>(GraphNode))
	{
		const USMGraphK2Node_PropertyNode_Base::FNotifyArgs& Args = Node->GetNotifyArgs();
		return FText::FromString(Args.Message);
	}

	return FText::GetEmpty();
}

SSMGraphProperty::SSMGraphProperty() : SSMGraphProperty_Base(), bIsValidDragDrop(false)
{
}

SSMGraphProperty::~SSMGraphProperty()
{
	if (USMGraphK2Node_GraphPropertyNode* Node = Cast<USMGraphK2Node_GraphPropertyNode>(GraphNode))
	{
		Node->ForceVisualRefreshEvent.RemoveAll(this);
	}
}

void SSMGraphProperty::Construct(const FArguments& InArgs)
{
	GraphNode = InArgs._GraphNode;
	WidgetInfo = InArgs._WidgetInfo ? *InArgs._WidgetInfo : FSMTextDisplayWidgetInfo();
	
	FText DefaultText = InArgs._WidgetInfo->DefaultText;

	UEdGraphPin* ResultPin = nullptr;
	if (USMGraphK2Node_GraphPropertyNode* Node = Cast<USMGraphK2Node_GraphPropertyNode>(GraphNode))
	{
		Node->ForceVisualRefreshEvent.AddSP(this, &SSMGraphProperty::Refresh);
		
		if (DefaultText.IsEmpty())
		{
			DefaultText = Node->GetPropertyNode()->GetDisplayName();
		}

		if (Node->GetPropertyNode()->bIsInArray)
		{
			DefaultText = FText::FromString(FString::Printf(TEXT("%s %s"), *DefaultText.ToString(),
					*FString::FromInt(Node->GetPropertyNode()->ArrayIndex)));
		}

		ResultPin = Node->GetResultPin();

		if (FSMGraphProperty_Base* Prop = Node->GetPropertyNode())
		{
			if (UBlueprint* Blueprint = FSMBlueprintEditorUtils::FindBlueprintForNode(Node))
			{
				if (const FProperty* Property = Prop->MemberReference.ResolveMember<FProperty>(Blueprint))
				{
					const FText Description = Property->GetToolTipText();
					if (!Description.IsEmpty())
					{
						SetToolTipText(Description);
					}
				}
			}
		}
	}
	
	ChildSlot
	[
		SNew(SOverlay)
		+SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FSMUnrealAppStyle::Get().GetBrush("Graph.StateNode.ColorSpill"))
			.BorderBackgroundColor(this, &SSMGraphProperty::GetBackgroundColor)
			.Padding(1.f)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(NotifyPadding)
				[
					MakeNotifyIconWidget()
				]
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				[
					SNew(SBox)
					.MinDesiredWidth(WidgetInfo.MinWidth)
					.MaxDesiredWidth(WidgetInfo.MaxWidth)
					.MinDesiredHeight(WidgetInfo.MinHeight)
					.MaxDesiredHeight(WidgetInfo.MaxHeight)
					.Clipping(WidgetInfo.Clipping)
					.Padding(1.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.VAlign(VAlign_Center)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							[
								// Default text.
								SNew(STextBlock)
								.Text(DefaultText)
								.TextStyle(&WidgetInfo.DefaultTextStyle)
								.Margin(FMargin(1.f))
							]
							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							.AutoWidth()
							[
								// Linear expression.
								SAssignNew(ExpressionWidget, SKismetLinearExpression, ResultPin)
								.Clipping(EWidgetClipping::ClipToBounds)
								.IsEditable(false)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.HAlign(HAlign_Left)
							.VAlign(VAlign_Center)
							.Padding(2.f) // Padding needed to help with zoom resize issues.
							[
								SAssignNew(InputPinContainer, SBox)
							]
						]
					]
				]
			]
		]
		// Optional highlight border.
		+SOverlay::Slot()
		.Padding(HighlightPadding)
		[
			MakeHighlightBorder()
		]
	];

	HandleExpressionChange(ResultPin);
}

void SSMGraphProperty::Finalize()
{
	UEdGraphPin* ResultPin = FindResultPin();
	TSharedPtr<SGraphNode> ParentNode = FindParentGraphNode();
	TSharedPtr<SGraphPin> InputPin = (ParentNode.IsValid() && ResultPin) ? FNodeFactory::CreatePinWidget(ResultPin) : nullptr;
	// Don't display if invalid or another connection is present. We only want this to display / edit the default value.
	if (InputPin.IsValid() && !InputPin->IsConnected())
	{
		InputPin->SetOwner(ParentNode.ToSharedRef());
		InputPin->SetOnlyShowDefaultValue(true);
		InputPin->SetShowLabel(false);
		InputPin->SetPinColorModifier(FSMBlueprintEditorUtils::GetEditorSettings()->PropertyPinColorModifier); // Without this the color can wash out the text.
		if (USMGraphK2Node_GraphPropertyNode* Node = Cast<USMGraphK2Node_GraphPropertyNode>(GraphNode))
		{
			if (const FSMGraphProperty_Base* Prop = Node->GetPropertyNode())
			{
				InputPin->SetIsEditable(!Prop->IsVariableReadOnly());
			}
		}
		
		TWeakPtr<SHorizontalBox> Row = InputPin->GetFullPinHorizontalRowWidget();
		// We want to hide the k2 selection pin as this is only for defaults.
		if (Row.Pin().IsValid())
		{
			if (FChildren* Children = Row.Pin()->GetChildren())
			{
				// The first child should be the pin.
				if (Children->Num() > 1)
				{
					Children->GetChildAt(0)->SetVisibility(EVisibility::Collapsed);
				}
			}
		}

		check(InputPinContainer.IsValid());
		InputPinContainer->SetContent(InputPin.ToSharedRef());
	}

	InputPinPtr = InputPin;
}

FReply SSMGraphProperty::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if (IsDragDropValid(DragDropEvent))
	{
		bIsValidDragDrop = true;
		SetCursor(EMouseCursor::GrabHand);

		// Tooltip message.
		FSMDragDropHelpers::SetDragDropMessage(DragDropEvent);
		
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void SSMGraphProperty::OnDragLeave(const FDragDropEvent& DragDropEvent)
{
	SetCursor(EMouseCursor::CardinalCross);
	bIsValidDragDrop = false;
	SCompoundWidget::OnDragLeave(DragDropEvent);
}

FReply SSMGraphProperty::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if (IsDragDropValid(DragDropEvent))
	{
		UEdGraphPin* ResultPin = nullptr;
		if (USMGraphK2Node_GraphPropertyNode* Node = Cast<USMGraphK2Node_GraphPropertyNode>(GraphNode))
		{
			USMPropertyGraph* Graph = CastChecked<USMPropertyGraph>(Node->GetPropertyGraph());

			TSharedPtr<FKismetVariableDragDropAction> VariableDragDrop = DragDropEvent.GetOperationAs<FKismetVariableDragDropAction>();
			if (VariableDragDrop.IsValid())
			{
				FProperty* Property = VariableDragDrop->GetVariableProperty();
				Graph->SetPropertyOnGraph(Property);
			}
			TSharedPtr<FKismetFunctionDragDropAction> FunctionDragDrop = DragDropEvent.GetOperationAs<FKismetFunctionDragDropAction>();
			if (FunctionDragDrop.IsValid())
			{
				UFunction const* Function = FSMDragDropAction_Function::GetFunction(FunctionDragDrop.Get());
				Graph->SetFunctionOnGraph(const_cast<UFunction*>(Function));
			}

			ResultPin = Node->GetResultPin();
		}

		SetCursor(EMouseCursor::CardinalCross);

		bIsValidDragDrop = false;
		ExpressionWidget->SetExpressionRoot(ResultPin);
		HandleExpressionChange(ResultPin);

		return FReply::Handled();
	}

	return SCompoundWidget::OnDrop(MyGeometry, DragDropEvent);
}

void SSMGraphProperty::Refresh()
{
	HandleExpressionChange(FindResultPin());
}

bool SSMGraphProperty::IsDragDropValid(const FDragDropEvent& DragDropEvent) const
{
	return FSMDragDropHelpers::IsDragDropValidForPropertyNode(Cast<USMGraphK2Node_PropertyNode_Base>(GraphNode), DragDropEvent, true);
}

void SSMGraphProperty::HandleExpressionChange(UEdGraphPin* ResultPin)
{
	if (ResultPin && ResultPin->LinkedTo.Num())
	{
		// Display normal object evaluation.
		ExpressionWidget->SetVisibility(EVisibility::HitTestInvisible);
		ExpressionWidget->SetExpressionRoot(ResultPin);

		if (InputPinPtr.IsValid())
		{
			InputPinPtr.Pin()->SetVisibility(EVisibility::Collapsed);
		}
	}
	else
	{
		// Display default text only.
		ExpressionWidget->SetVisibility(EVisibility::Collapsed);
		if (InputPinPtr.IsValid())
		{
			InputPinPtr.Pin()->SetVisibility(EVisibility::HitTestInvisible);
		}
	}
}

FSlateColor SSMGraphProperty::GetBackgroundColor() const
{
	return bIsValidDragDrop ? WidgetInfo.OnDropBackgroundColor : WidgetInfo.BackgroundColor;
}

#undef LOCTEXT_NAMESPACE
