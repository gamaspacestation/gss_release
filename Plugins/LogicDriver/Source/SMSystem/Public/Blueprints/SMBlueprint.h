// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "Engine/Blueprint.h"

#include "SMBlueprint.generated.h"

/**
 * State Machine Blueprints allow you to assemble a finite state machine which is capable of running normal Blueprint logic.
 * Any UObject instance may be passed as a context for the state machine.
 */
UCLASS(BlueprintType, meta = (DisplayName = "State Machine Blueprint"))
class SMSYSTEM_API USMBlueprint : public UBlueprint
{
	GENERATED_UCLASS_BODY()

public:

#if WITH_EDITOR
	// UBlueprint
	virtual bool SupportedByDefaultBlueprintFactory() const override { return false; }
	virtual UClass* GetBlueprintClass() const override;
	virtual void GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const override;
	virtual void NotifyGraphRenamed(UEdGraph* Graph, FName OldName, FName NewName) override;
	virtual bool SupportsInputEvents() const override;
	// ~UBlueprint

	class USMBlueprintGeneratedClass* GetGeneratedClass() const;

	USMBlueprint* FindOldestParentBlueprint() const;

	DECLARE_MULTICAST_DELEGATE_FourParams(FOnRenameGraph, UBlueprint* /*Blueprint*/, UEdGraph* /*VariableClass*/, const FName& /*OldVarName*/, const FName& /*NewVarName*/);
	/** Event fired when a graph in a state machine blueprint is renamed. */
	static FOnRenameGraph OnRenameGraphEvent;

	/** Prevents SMBlueprintEditorUtils conditional compile, useful for preventing compile on bulk operations. */
	bool bPreventConditionalCompile = false;

	/** Prevents SMBlueprintEditorUtils from clearing the cache, useful for bulk operations. */
	bool bPreventCacheInvalidation = false;
#endif

#if WITH_EDITORONLY_DATA
public:
	/** Get or instantiate the preview object. */
	class USMPreviewObject* GetPreviewObject(bool bCreateIfNeeded = true);
	/** Recreates the preview object only if it already exists. */
	void RecreatePreviewObject();
private:
	UPROPERTY(Export)
	UObject* PreviewObject;

public:
	/**
	 * Allow editor construction scripts to run within this blueprint, dependent on Logic Driver Project Editor settings.
	 */
	UPROPERTY(EditAnywhere, Category = LogicDriverOptions)
	uint8 bAllowEditorConstructionScripts: 1;

	/**
	 * When enabled 'OnPreCompileValidate()' will be called on each node instance in this blueprint during compile.
	 * This can allow custom validation prevent this blueprint from compiling.
	 */
	UPROPERTY(EditAnywhere, Category = LogicDriverOptions)
	uint8 bEnableNodeValidation: 1;

	/**
	 * Run 'OnPreCompileValidate()' on all nodes within any references in this blueprint.
	 * This can allow a node contained in another blueprint to prevent this blueprint from compiling.
	 */
	UPROPERTY(EditAnywhere, Category = LogicDriverOptions, meta = (EditCondition = "bEnableNodeValidation"))
	uint8 bEnableReferenceNodeValidation: 1;

#endif
	
public:

	/** The specific asset version of this blueprint. This only increases on significant changes that requires a recompile. */
	UPROPERTY(AssetRegistrySearchable)
	int32 AssetVersion;

	/** The plugin version used to save this asset. */
	UPROPERTY(AssetRegistrySearchable)
	int32 PluginVersion;
};


/**
 * [Logic Driver] Node Class Blueprints allow you to define custom classes for nodes within your State Machine Blueprint. Reusable logic can be encapsulated
 * here as well as the ability to take more direct control over state machines by gaining access to other node class instances. Additionally these classes
 * can be extended using C++.
 */
UCLASS(BlueprintType, meta = (DisplayName = "Node Class Blueprint"))
class SMSYSTEM_API USMNodeBlueprint : public UBlueprint
{
	GENERATED_UCLASS_BODY()

public:

#if WITH_EDITOR
	// UBlueprint
	virtual bool SupportedByDefaultBlueprintFactory() const override { return false; }
	virtual UClass* GetBlueprintClass() const override;
	virtual void GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const override;
	virtual bool SupportsInputEvents() const override;
	// ~UBlueprint

#endif
	
	class USMNodeBlueprintGeneratedClass* GetGeneratedClass() const;

public:
	/** The specific asset version of this blueprint. This only increases on significant changes that requires a recompile. */
	UPROPERTY(AssetRegistrySearchable)
	int32 AssetVersion;

	/** The plugin version used to save this asset. */
	UPROPERTY(AssetRegistrySearchable)
	int32 PluginVersion;
};
