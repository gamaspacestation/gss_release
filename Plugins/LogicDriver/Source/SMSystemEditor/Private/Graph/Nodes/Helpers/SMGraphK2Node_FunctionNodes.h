// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "Graph/Nodes/RootNodes/SMGraphK2Node_RuntimeNodeContainer.h"

#include "SMGraphK2Node_FunctionNodes.generated.h"

UCLASS(MinimalAPI)
class USMGraphK2Node_FunctionNode : public USMGraphK2Node_RuntimeNodeReference
{
public:
	GENERATED_UCLASS_BODY()
	
	// UEdGraphNode
	virtual void PostPlacedNewNode() override;
	virtual void PostPasteNode() override;
	virtual bool IsCompatibleWithGraph(UEdGraph const* Graph) const override;
	virtual bool CanUserDeleteNode() const override { return true; }
	virtual bool CanDuplicateNode() const override { return true; }
	virtual bool IsNodePure() const override { return false; }
	virtual FText GetMenuCategory() const override;
	virtual bool IsActionFilteredOut(class FBlueprintActionFilter const& Filter) override;
	//~ UEdGraphNode

	// USMGraphK2Node_Base
	virtual bool CanCollapseNode() const override { return true; }
	virtual bool CanCollapseToFunctionOrMacro() const override { return true; }
	virtual UEdGraphPin* GetInputPin() const override;
	// ~UEdGraphNode

	virtual bool ExpandAndWireStandardFunction(UFunction* Function, UEdGraphPin* SelfPin, FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty);
};

UCLASS(MinimalAPI)
class USMGraphK2Node_FunctionNode_StateMachineRef : public USMGraphK2Node_FunctionNode
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual bool IsCompatibleWithGraph(UEdGraph const* Graph) const override;
	virtual FText GetMenuCategory() const override;
	//~ UEdGraphNode

	// USMGraphK2Node_RuntimeNodeReference
	virtual bool HandlesOwnExpansion() const override { return true; }
	virtual void CustomExpandNode(FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_RuntimeNodeReference
};

UCLASS(MinimalAPI)
class USMGraphK2Node_StateMachineRef_Start : public USMGraphK2Node_FunctionNode_StateMachineRef
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	//~ UEdGraphNode

	// USMGraphK2Node_RuntimeNodeReference
	virtual void CustomExpandNode(FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_RuntimeNodeReference
};

UCLASS(MinimalAPI)
class USMGraphK2Node_StateMachineRef_Update : public USMGraphK2Node_FunctionNode_StateMachineRef
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	//~ UEdGraphNode

	// USMGraphK2Node_RuntimeNodeReference
	virtual void CustomExpandNode(FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_RuntimeNodeReference
};

UCLASS(MinimalAPI)
class USMGraphK2Node_StateMachineRef_Stop : public USMGraphK2Node_FunctionNode_StateMachineRef
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	//~ UEdGraphNode

	// USMGraphK2Node_RuntimeNodeReference
	virtual void CustomExpandNode(FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_RuntimeNodeReference
};

UENUM(BlueprintType)
enum ESMDelegateOwner
{
	/** This state machine instance. */
	SMDO_This			UMETA(DisplayName = "This"),
	/**
	 * The context object for this state machine.
	 * The class is not known until run-time and needs to be chosen manually. */
	SMDO_Context		UMETA(DisplayName = "Context"),
	/** The previous state instance. The class is determined by the state. */
	SMDO_PreviousState	UMETA(DisplayName = "Previous State")
};

UCLASS(MinimalAPI)
class USMGraphK2Node_FunctionNode_TransitionEvent : public USMGraphK2Node_FunctionNode
{
	GENERATED_UCLASS_BODY()
	
	UPROPERTY()
	FName DelegatePropertyName;
	
	UPROPERTY()
	TSubclassOf<UObject> DelegateOwnerClass;

	UPROPERTY()
	TEnumAsByte<ESMDelegateOwner> DelegateOwnerInstance;
	
	UPROPERTY()
	FMemberReference EventReference;

	/** Transition class of the transition edge. */
	UPROPERTY()
	TSubclassOf<class USMTransitionInstance> TransitionClass;
	
	// UEdGraphNode
	virtual void AllocateDefaultPins() override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual bool IsCompatibleWithGraph(UEdGraph const* Graph) const override;
	virtual bool IsActionFilteredOut(FBlueprintActionFilter const& Filter) override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool CanUserDeleteNode() const override { return false; }
	virtual bool CanDuplicateNode() const override { return false; }
	virtual bool HasExternalDependencies(TArray<UStruct*>* OptionalOutput) const override;
	virtual void ReconstructNode() override;
	//~ UEdGraphNode

	// USMGraphK2Node_Base
	virtual void PostCompileValidate(FCompilerResultsLog& MessageLog) override;
	// ~USMGraphK2Node_Base
	
	// USMGraphK2Node_RuntimeNodeReference
	virtual bool HandlesOwnExpansion() const override { return true; }
	virtual void CustomExpandNode(FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_RuntimeNodeReference

	void SetEventReferenceFromDelegate(FMulticastDelegateProperty* Delegate, ESMDelegateOwner InstanceType);
	
	UFunction* GetDelegateFunction() const;
	void UpdateNodeFromFunction();
};