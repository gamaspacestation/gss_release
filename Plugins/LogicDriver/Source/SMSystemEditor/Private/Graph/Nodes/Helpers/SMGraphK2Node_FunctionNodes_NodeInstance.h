// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "SMGraphK2Node_FunctionNodes.h"

#include "SMStateInstance.h"
#include "SMConduitInstance.h"

#include "SMGraphK2Node_FunctionNodes_NodeInstance.generated.h"

UCLASS(MinimalAPI)
class USMGraphK2Node_FunctionNode_NodeInstance : public USMGraphK2Node_FunctionNode
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual bool IsCompatibleWithGraph(UEdGraph const* Graph) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual UObject* GetJumpTargetForDoubleClick() const override;
	//~ UEdGraphNode

	// USMGraphK2Node_RuntimeNodeReference
	virtual void PreConsolidatedEventGraphValidate(FCompilerResultsLog& MessageLog) override;
	virtual bool HandlesOwnExpansion() const override { return true; }
	virtual void CustomExpandNode(FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_RuntimeNodeReference

	// USMGraphK2Node_FunctionNode
	/** Creates a function node and wires execution pins. The self pin can be null and will be used from the auto created cast node. */
	virtual bool ExpandAndWireStandardFunction(UFunction* Function, UEdGraphPin* SelfPin, FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_FunctionNode

	/** Return the function name to expect. Such as 'OnStateBegin'. */
	virtual FName GetInstanceRuntimeFunctionName() const { return NAME_None; }

	/** Return the appropriate node instance class to use based on the compile status. */
	virtual UClass* GetNodeInstanceClass() const;

protected:
	UPROPERTY()
	TSubclassOf<UObject> NodeInstanceClass;
};

////////////////////////
/// Node Base Classes
////////////////////////

UCLASS(MinimalAPI)
class USMGraphK2Node_StateInstance_Base : public USMGraphK2Node_FunctionNode_NodeInstance
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual bool IsCompatibleWithGraph(UEdGraph const* Graph) const override;
	// ~UEdGraphNode
};

UCLASS(MinimalAPI)
class USMGraphK2Node_ConduitInstance_Base : public USMGraphK2Node_FunctionNode_NodeInstance
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual bool IsCompatibleWithGraph(UEdGraph const* Graph) const override;
	// ~UEdGraphNode
};

////////////////////////
/// Usable Node Classes
////////////////////////

UCLASS(MinimalAPI)
class USMGraphK2Node_StateInstance_Begin : public USMGraphK2Node_StateInstance_Base
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ UEdGraphNode

	// USMGraphK2Node_RuntimeNodeReference
	virtual void CustomExpandNode(FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_RuntimeNodeReference

	// USMGraphK2Node_FunctionNode_NodeInstance
	virtual FName GetInstanceRuntimeFunctionName() const override { return GET_FUNCTION_NAME_CHECKED(USMStateInstance_Base, OnStateBegin); }
	// ~USMGraphK2Node_FunctionNode_NodeInstance
};

UCLASS(MinimalAPI)
class USMGraphK2Node_StateInstance_Update : public USMGraphK2Node_StateInstance_Base
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ UEdGraphNode

	// USMGraphK2Node_RuntimeNodeReference
	virtual void CustomExpandNode(FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_RuntimeNodeReference

	// USMGraphK2Node_FunctionNode_NodeInstance
	virtual FName GetInstanceRuntimeFunctionName() const override { return GET_FUNCTION_NAME_CHECKED(USMStateInstance_Base, OnStateUpdate); }
	// ~USMGraphK2Node_FunctionNode_NodeInstance
};

UCLASS(MinimalAPI)
class USMGraphK2Node_StateInstance_End : public USMGraphK2Node_StateInstance_Base
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ UEdGraphNode

	// USMGraphK2Node_RuntimeNodeReference
	virtual void CustomExpandNode(FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_RuntimeNodeReference

	// USMGraphK2Node_FunctionNode_NodeInstance
	virtual FName GetInstanceRuntimeFunctionName() const override { return GET_FUNCTION_NAME_CHECKED(USMStateInstance_Base, OnStateEnd); }
	// ~USMGraphK2Node_FunctionNode_NodeInstance
};

UCLASS(MinimalAPI)
class USMGraphK2Node_StateInstance_StateMachineStart : public USMGraphK2Node_StateInstance_Base
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ UEdGraphNode

	// USMGraphK2Node_RuntimeNodeReference
	virtual void CustomExpandNode(FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_RuntimeNodeReference

	// USMGraphK2Node_FunctionNode_NodeInstance
	virtual FName GetInstanceRuntimeFunctionName() const override { return GET_FUNCTION_NAME_CHECKED(USMStateInstance_Base, OnRootStateMachineStart); }
	// ~USMGraphK2Node_FunctionNode_NodeInstance
};

UCLASS(MinimalAPI)
class USMGraphK2Node_StateInstance_StateMachineStop : public USMGraphK2Node_StateInstance_Base
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ UEdGraphNode

	// USMGraphK2Node_RuntimeNodeReference
	virtual void CustomExpandNode(FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_RuntimeNodeReference

	// USMGraphK2Node_FunctionNode_NodeInstance
	virtual FName GetInstanceRuntimeFunctionName() const override { return GET_FUNCTION_NAME_CHECKED(USMStateInstance_Base, OnRootStateMachineStop); }
	// ~USMGraphK2Node_FunctionNode_NodeInstance
};

UCLASS(MinimalAPI)
class USMGraphK2Node_StateInstance_OnStateInitialized : public USMGraphK2Node_StateInstance_Base
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ UEdGraphNode

	// USMGraphK2Node_RuntimeNodeReference
	virtual void CustomExpandNode(FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_RuntimeNodeReference

	// USMGraphK2Node_FunctionNode_NodeInstance
	virtual FName GetInstanceRuntimeFunctionName() const override { return GET_FUNCTION_NAME_CHECKED(USMStateInstance, OnStateInitialized); }
	// ~USMGraphK2Node_FunctionNode_NodeInstance
};

UCLASS(MinimalAPI)
class USMGraphK2Node_StateInstance_OnStateShutdown : public USMGraphK2Node_StateInstance_Base
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ UEdGraphNode

	// USMGraphK2Node_RuntimeNodeReference
	virtual void CustomExpandNode(FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_RuntimeNodeReference

	// USMGraphK2Node_FunctionNode_NodeInstance
	virtual FName GetInstanceRuntimeFunctionName() const override { return GET_FUNCTION_NAME_CHECKED(USMStateInstance, OnStateShutdown); }
	// ~USMGraphK2Node_FunctionNode_NodeInstance
};

UCLASS(MinimalAPI)
class USMGraphK2Node_ConduitInstance_CanEnterTransition : public USMGraphK2Node_ConduitInstance_Base
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool IsNodePure() const override { return true; }
	//~ UEdGraphNode

	// USMGraphK2Node_RuntimeNodeReference
	virtual void CustomExpandNode(FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_RuntimeNodeReference

	// USMGraphK2Node_FunctionNode_NodeInstance
	virtual FName GetInstanceRuntimeFunctionName() const override { return GET_FUNCTION_NAME_CHECKED(USMConduitInstance, CanEnterTransition); }
	// ~USMGraphK2Node_FunctionNode_NodeInstance
};

UCLASS(MinimalAPI)
class USMGraphK2Node_ConduitInstance_OnConduitEntered : public USMGraphK2Node_ConduitInstance_Base
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ UEdGraphNode

	// USMGraphK2Node_RuntimeNodeReference
	virtual void CustomExpandNode(FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_RuntimeNodeReference

	// USMGraphK2Node_FunctionNode_NodeInstance
	virtual FName GetInstanceRuntimeFunctionName() const override { return GET_FUNCTION_NAME_CHECKED(USMConduitInstance, OnConduitEntered); }
	// ~USMGraphK2Node_FunctionNode_NodeInstance
};

UCLASS(MinimalAPI)
class USMGraphK2Node_ConduitInstance_OnConduitInitialized : public USMGraphK2Node_ConduitInstance_Base
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ UEdGraphNode

	// USMGraphK2Node_RuntimeNodeReference
	virtual void CustomExpandNode(FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_RuntimeNodeReference

	// USMGraphK2Node_FunctionNode_NodeInstance
	virtual FName GetInstanceRuntimeFunctionName() const override { return GET_FUNCTION_NAME_CHECKED(USMConduitInstance, OnConduitInitialized); }
	// ~USMGraphK2Node_FunctionNode_NodeInstance
};

UCLASS(MinimalAPI)
class USMGraphK2Node_ConduitInstance_OnConduitShutdown : public USMGraphK2Node_ConduitInstance_Base
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ UEdGraphNode

	// USMGraphK2Node_RuntimeNodeReference
	virtual void CustomExpandNode(FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty) override;
	// ~USMGraphK2Node_RuntimeNodeReference

	// USMGraphK2Node_FunctionNode_NodeInstance
	virtual FName GetInstanceRuntimeFunctionName() const override { return GET_FUNCTION_NAME_CHECKED(USMConduitInstance, OnConduitShutdown); }
	// ~USMGraphK2Node_FunctionNode_NodeInstance
};