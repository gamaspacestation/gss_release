// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#if WITH_EDITORONLY_DATA
#include "Styling/SlateTypes.h"
#include "Layout/Clipping.h"
#endif

#include "SMNodeWidgetInfo.generated.h"

/**
 * Info used in determining widget display on a node.
 */
USTRUCT()
struct SMSYSTEM_API FSMNodeWidgetInfo
{
public:
	GENERATED_USTRUCT_BODY()

	FSMNodeWidgetInfo();

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, Category = "Size")
	int32 MinWidth;
	
	UPROPERTY(EditDefaultsOnly, Category = "Size")
	int32 MaxWidth;

	UPROPERTY(EditDefaultsOnly, Category = "Size")
	int32 MinHeight;
	
	UPROPERTY(EditDefaultsOnly, Category = "Size")
	int32 MaxHeight;

	/*
	* @deprecated Use DisplayOrder metadata for native properties, or adjust the blueprint variable order in blueprints. 
	*/
	UPROPERTY()
	int32 DisplayOrder_DEPRECATED;

	UPROPERTY(EditDefaultsOnly, Category = "Display")
	FLinearColor BackgroundColor;

	UPROPERTY(EditDefaultsOnly, Category = "Display")
	EWidgetClipping Clipping;
	
	/** When placing a new node the sgraph node widget will consider this node for editing text. */
	UPROPERTY(EditDefaultsOnly, Category = "Display")
	bool bConsiderForDefaultWidget;

#endif
};

/**
 * Info used in determining text based widget display on a node.
 */
USTRUCT()
struct SMSYSTEM_API FSMTextDisplayWidgetInfo : public FSMNodeWidgetInfo
{
public:
	GENERATED_USTRUCT_BODY()

	FSMTextDisplayWidgetInfo();
	
#if WITH_EDITORONLY_DATA
	/** Default text to display in place of the property name. */
	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FText DefaultText;

	/** Style of default text. */
	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FTextBlockStyle DefaultTextStyle;
	
	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FLinearColor OnDropBackgroundColor;
#endif
};
