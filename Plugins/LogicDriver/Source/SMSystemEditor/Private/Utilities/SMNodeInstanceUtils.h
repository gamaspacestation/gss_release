// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "SMNodeInstance.h"

#include "Kismet2/BlueprintEditorUtils.h"
#include "Graph/SMGraph.h"

class ISinglePropertyView;
class USMGraphNode_Base;

// Helpers for managing node instances and related objects.
class SMSYSTEMEDITOR_API FSMNodeInstanceUtils
{
public:
	enum class ENodeStackType : uint8
	{
		None,
		StateStack,
		TransitionStack
	};

	/** @return The display name for this node, accounting for editor name sanitization and class data. */
	static FString GetNodeDisplayName(const USMNodeInstance* InNodeInstance);

	/** @return The description, accounting for overrides or class data. */
	static FText GetNodeDescriptionText(const USMNodeInstance* InNodeInstance);

	/** @return The category of the node for use in context menus. */
	static FText GetNodeCategory(const USMNodeInstance* InNodeInstance);

	/** Recursively checks a child to see if it belongs to a parent. */
	static bool IsWidgetChildOf(TSharedPtr<SWidget> Parent, TSharedPtr<SWidget> PossibleChild);

	/** Create formatted text to summarize the node class. */
	static FText CreateNodeClassTextSummary(const USMNodeInstance* NodeInstance);

	/** Create a widget to display node class information. */
	static TSharedPtr<SWidget> CreateNodeClassWidgetDisplay(const USMNodeInstance* NodeInstance);
	
	/** Sets all related internal properties. Returns the guid used. */
	static const FGuid& SetGraphPropertyFromProperty(FSMGraphProperty_Base& GraphProperty, FProperty* Property,
		USMNodeInstance* NodeInstance, int32 Index = 0, bool bSetGuid = true, bool bUseTemplateInGuid = true,
		bool bUseTempNativeGuid = false);

	/** Checks appropriate flags on a property to see if it should be exposed. */
	static bool IsPropertyExposedToGraphNode(const FProperty* Property);

	/** Checks if the property handle is a supported container and exposed. */
	static bool IsPropertyHandleExposedContainer(const TSharedPtr<IPropertyHandle>& InHandle);
	
	/** Checks if this handle should be displayed in a node stack instance template. */
	static bool ShouldHideNodeStackPropertyFromDetails(const FProperty* InProperty);

	/** Recursively check if a handle has no properties and hide the handle. */
	static bool HideEmptyCategoryHandles(const TSharedPtr<IPropertyHandle>& InHandle, ENodeStackType NodeStackType);
	
	/** Returns the struct property if this property is a graph property. */
	static FStructProperty* GetGraphPropertyFromProperty(FProperty* Property);

	/** Checks if the property is considered a graph property. */
	static bool IsPropertyGraphProperty(const FProperty* Property);

	/**
	 * Checks if the node might have user defined construction scripts.
	 *
	 * @param NodeClass Node class construction script graph to check.
	 * @param ExecutionType Editor or game construction scripts.
	 */
	static bool DoesNodeClassPossiblyHaveConstructionScripts(TSubclassOf<USMNodeInstance> NodeClass, ESMExecutionEnvironment ExecutionType);

	/**
	 * Return an existing override. O(n).
	 * @param InNodeInstance The node instance to modify. Providing a CDO will propagate values to instances.
	 * @param VariableName The property name to override.
	 * @param OutPropView A single property view which owns the handle. If this goes out of scope the return value will become stale.
	 * @return The property handle if one exists.
	 */
	static TSharedPtr<IPropertyHandle> FindExposedPropertyOverrideByName(USMNodeInstance* InNodeInstance, const FName& VariableName,
		TSharedPtr<ISinglePropertyView>& OutPropView);
	
	/**
	 * Return an existing override or add a new one. O(n).
	 * @param InNodeInstance The node instance to modify. Providing a CDO will propagate values to instances.
	 * @param VariableName The property name to override.
	 * @param OutPropView A single property view which owns the handle. If this goes out of scope the return value will become stale.
	 * @return The existing property handle or a new one.
	 */
	static TSharedPtr<IPropertyHandle> FindOrAddExposedPropertyOverrideByName(USMNodeInstance* InNodeInstance, const FName& VariableName,
		TSharedPtr<ISinglePropertyView>& OutPropView);

	/**
	 * Update an exposed property's VariableName. O(n).
	 * @return True if successful.
	 */
	static bool UpdateExposedPropertyOverrideName(USMNodeInstance* InNodeInstance, const FName& OldVarName, const FName& NewVarName);

	/**
	 * Remove an exposed property override by name. O(n).
	 * @return Number of elements removed.
	 */
	static uint32 RemoveExposedPropertyOverrideByName(USMNodeInstance* InNodeInstance, const FName& VariableName);
};

