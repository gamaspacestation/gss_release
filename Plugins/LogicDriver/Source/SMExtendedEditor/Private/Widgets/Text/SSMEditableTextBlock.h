// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "SlateGlobals.h"
#include "Styling/SlateColor.h"
#include "Fonts/SlateFontInfo.h"
#include "Input/Reply.h"
#include "Styling/SlateWidgetStyleAsset.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "Framework/SlateDelegates.h"
#include "Framework/Text/TextLayout.h"
#include "UObject/StrongObjectPtr.h"

class USMGraphNode_StateNode;
class FActiveTimerHandle;
class IBreakIterator;
class SEditableTextBox;
class SHorizontalBox;
class SSMEditableTextBox;
class STextBlock;
class UDataTable;
class FSlateStyleSet;
class URichTextBlockDecorator;
class ITextDecorator;

DECLARE_DELEGATE_RetVal_TwoParams(bool, FOnVerifyTextChanged, const FText&, FText&)
DECLARE_DELEGATE_OneParam(FOnBeginTextEdit, const FText&)

/**
 * This is a recreation of SInlineEditableTextBlock. It's almost entirely the same except we inject our own SSMEditableTextBox as the Multiline Text box.
 * We also perform custom handling of various events such as mouse drag.
 */
class SSMEditableTextBlock : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SSMEditableTextBlock)
		: _RichText()
		  , _PlainText()
		  , _GraphNode(nullptr)
		  , _Style(&FCoreStyle::Get().GetWidgetStyle<FInlineEditableTextBlockStyle>("InlineEditableTextBlockStyle"))
		  , _Font()
		  , _ColorAndOpacity()
		  , _ShadowOffset()
		  , _ShadowColorAndOpacity()
		  , _HighlightText()
		  , _WrapTextAt(0.0f)
		  , _Justification(ETextJustify::Left)
		  , _LineBreakPolicy()
		  , _IsReadOnly(false)
		  , _MultiLine(false)
		  , _RichTextStyleDataTable(nullptr)
		  , _RichTextStyleDecoratorInstances()
	      , _ModiferKeyForNewLine(EModifierKey::None)
	{
	}

	/** The text displayed in this text block */
	SLATE_ATTRIBUTE(FText, RichText)

	SLATE_ATTRIBUTE(FText, PlainText)

	/** The default text if no other text is passed in. */
	SLATE_ATTRIBUTE(FText, DefaultText)

	SLATE_ATTRIBUTE(FTextBlockStyle, DefaultTextStyle);
	
	SLATE_ARGUMENT(const class UEdGraphNode*, GraphNode)

	/** Pointer to a style of the inline editable text block, which dictates the font, color, and shadow options. */
	SLATE_STYLE_ARGUMENT(FInlineEditableTextBlockStyle, Style)

	/** Sets the font used to draw the text (overrides style) */
	SLATE_ATTRIBUTE(FSlateFontInfo, Font)

	/** Text color and opacity (overrides style) */
	SLATE_ATTRIBUTE(FSlateColor, ColorAndOpacity)

	/** Drop shadow offset in pixels (overrides style) */
	SLATE_ATTRIBUTE(FVector2D, ShadowOffset)

	/** Shadow color and opacity (overrides style) */
	SLATE_ATTRIBUTE(FLinearColor, ShadowColorAndOpacity)

	/** Highlight this text in the text block */
	SLATE_ATTRIBUTE(FText, HighlightText)

	/** Whether text wraps onto a new line when it's length exceeds this width; if this value is zero or negative, no wrapping occurs. */
	SLATE_ATTRIBUTE(float, WrapTextAt)

	/** How the text should be aligned with the margin. */
	SLATE_ATTRIBUTE(ETextJustify::Type, Justification)

	/** The iterator to use to detect appropriate soft-wrapping points for lines (or null to use the default) */
	SLATE_ARGUMENT(TSharedPtr<IBreakIterator>, LineBreakPolicy)

	/** True if the editable text block is read-only. It will not be able to enter edit mode if read-only */
	SLATE_ATTRIBUTE(bool, IsReadOnly)

	/** True if the editable text block is multi-line */
	SLATE_ARGUMENT(bool, MultiLine)

	/** Optional data table to use for rich text style */
	SLATE_ARGUMENT(UDataTable*, RichTextStyleDataTable)

	/** Optional decorators use for rich text style */
	SLATE_ARGUMENT(TArray<TSharedRef<ITextDecorator>>, RichTextStyleDecoratorInstances)

	/** The optional modifier key necessary to create a newline when typing into the editor. */
	SLATE_ARGUMENT(EModifierKey::Type, ModiferKeyForNewLine)

	/** Callback when the text starts to be edited */
	SLATE_EVENT(FOnBeginTextEdit, OnBeginTextEdit)

	/** Callback when the text is committed. */
	SLATE_EVENT(FOnTextCommitted, OnTextCommitted)

	/** Callback when the text editing begins. */
	SLATE_EVENT(FSimpleDelegate, OnEnterEditingMode)

	/** Callback when the text editing ends. */
	SLATE_EVENT(FSimpleDelegate, OnExitEditingMode)

	/** Callback to check if the widget is selected, should only be hooked up if parent widget is handling selection or focus. */
	SLATE_EVENT(FIsSelected, IsSelected)

	/** Called whenever the text is changed interactively by the user */
	SLATE_EVENT(FOnVerifyTextChanged, OnVerifyTextChanged)
	SLATE_END_ARGS()

	virtual ~SSMEditableTextBlock() override;

	/**
	 * Construct this widget.  Called by the SNew() Slate macro.
	 *
	 * @param	InArgs				Declaration used by the SNew() macro to construct this widget
	 */
	void Construct(const FArguments& InArgs);

	/**
	 * See SWidget::SupportsKeyboardFocus().
	 *
	 * @return  This widget can gain focus if IsSelected is not bound.
	 */
	virtual bool SupportsKeyboardFocus() const override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;

	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;

	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;

	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;

	//virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;

	/** Switches the widget to editing mode */
	void EnterEditingMode();

	/** Switches the widget to label mode. */
	void ExitEditingMode();

	/** Checks if the widget is in edit mode */
	bool IsInEditMode() const;

	void SetReadOnly(bool bInIsReadOnly);

	void SetText(const TAttribute< FText >& InText);
	void SetText(const FString& InText);

	/** Sets the wrap text at attribute.  See WrapTextAt attribute */
	void SetWrapTextAt(const TAttribute<float>& InWrapTextAt);

	/** Create the appropriate run for the text editor representing this property. */
	void InsertProperty(FProperty* Property);
	void InsertFunction(UFunction* Function);
protected:
	/** Callback for the text box's OnTextChanged event */
	void OnTextChanged(const FText& InText);

	/** Callback when the text box is committed, switches back to label mode. */
	void OnTextBoxCommitted(const FText& InText, ETextCommit::Type InCommitType);

	/** Cancels the edit mode and switches back to label mode */
	void CancelEditMode();

	/** Validates that the drag drop event is allowed for this class. */
	bool IsDragDropValid(const FDragDropEvent& DragDropEvent) const;

	//////////////
	// Rich Style
	//////////////
public:
	TSharedPtr<SSMEditableTextBox> GetEditableRichTextBlock() const { return PlainTextBox; }
	TSharedPtr<SRichTextBlock> GetReadOnlyRichTextBlock() const { return RichTextBlock; }

protected:
	void BuildRichTextStyleSet();

private:
	/** The widget used when in read only mode */
	TSharedPtr<SRichTextBlock> RichTextBlock;

	/** The widget used when in editing mode */
	TSharedPtr<SSMEditableTextBox> PlainTextBox;

	/** The rich style data table if one is set. */
	UDataTable* RichTextStyleSet = nullptr;

	/** The instance for managing rich style. */
	TSharedPtr<FSlateStyleSet> RichStyleInstance;

	/** Either a generic style provided by the text graph, or a default from a rich data table. */
	FTextBlockStyle DefaultRichTextStyle;

	//////////////
	// ~Rich Style
	//////////////

protected:
	const UEdGraphNode* GraphNode = nullptr;

	FSimpleDelegate OnEnterEditingMode;
	FSimpleDelegate OnExitEditingMode;

	/** Delegate to execute when the text starts to be edited */
	FOnBeginTextEdit OnBeginTextEditDelegate;

	/** Delegate to execute when editing mode text is committed. */
	FOnTextCommitted OnTextCommittedDelegate;

	/** Delegate to execute to check the status of if the widget is selected or not
		Only needs to be hooked up if an external widget is managing selection, such
		as a SListView or STreeView. */
	FIsSelected IsSelected;

	/** Main horizontal box, used to dynamically add and remove the editable slot. */
	TSharedPtr< SHorizontalBox > HorizontalBox;

	/** Callback to verify text when changed. Will return an error message to denote problems. */
	FOnVerifyTextChanged OnVerifyTextChanged;

	/** Attribute for the text to use for the widget */
	TAttribute<FText> RichText;

	/** Attribute for the text to use when editing the widget */
	TAttribute<FText> PlainText;

	/** Attribute to look up if the widget is read-only */
	TAttribute< bool > bIsReadOnly;

	/** Widget to focus when we finish editing */
	TWeakPtr<SWidget> WidgetToFocus;

	/** The editable plain text needs focus. */
	mutable bool bNeedsFocus = false;
private:

	/** Get editable text widget */
	TSharedPtr<SWidget> GetEditableTextWidget() const;

	/** Set editable text */
	void SetEditableText(const TAttribute< FText >& InNewText);

	/** Set textbox error */
	void SetTextBoxError(const FText& ErrorText);

	/** Active timer to trigger entry into edit mode after a delay */
	EActiveTimerReturnType TriggerEditMode(double InCurrentTime, float InDeltaTime);

	/** The handle to the active timer */
	TWeakPtr<FActiveTimerHandle> ActiveTimerHandle;

protected:
	/** When selection of widget is managed by another widget, this delays the "double select" from
	occurring immediately, offering a chance for double clicking to take action. */
	float DoubleSelectDelay;

	/** Attribute to look up if the widget is multiline */
	bool bIsMultiLine;

	bool bIsDefaultValue;

};
