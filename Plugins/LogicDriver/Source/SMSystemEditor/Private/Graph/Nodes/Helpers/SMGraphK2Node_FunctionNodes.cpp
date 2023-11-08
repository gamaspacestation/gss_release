// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "SMGraphK2Node_FunctionNodes.h"
#include "Graph/SMConduitGraph.h"
#include "Graph/SMIntermediateGraph.h"
#include "Graph/SMStateGraph.h"
#include "Graph/SMTransitionGraph.h"
#include "Graph/Nodes/Helpers/SMGraphK2Node_StateReadNodes.h"
#include "Graph/Nodes/RootNodes/SMGraphK2Node_TransitionInitializedNode.h"
#include "Graph/Nodes/RootNodes/SMGraphK2Node_TransitionShutdownNode.h"
#include "Utilities/SMBlueprintEditorUtils.h"

#include "SMInstance.h"
#include "Blueprints/SMBlueprint.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintDelegateNodeSpawner.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_AddDelegate.h"
#include "K2Node_CallFunction.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_RemoveDelegate.h"
#include "EdGraph/EdGraph.h"

#define LOCTEXT_NAMESPACE "SMStateMachineFunctionNode"

USMGraphK2Node_FunctionNode::USMGraphK2Node_FunctionNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FText USMGraphK2Node_FunctionNode::GetMenuCategory() const
{
	return FText::FromString(STATE_MACHINE_HELPER_CATEGORY);
}

bool USMGraphK2Node_FunctionNode::IsActionFilteredOut(FBlueprintActionFilter const& Filter)
{
	for (UBlueprint* Blueprint : Filter.Context.Blueprints)
	{
		if (!Cast<USMBlueprint>(Blueprint))
		{
			return true;
		}
	}

	for (UEdGraph* Graph : Filter.Context.Graphs)
	{
		if (!Graph->IsA<USMTransitionGraph>() && !Graph->IsA<USMStateGraph>() && !Graph->IsA<USMConduitGraph>())
		{
			return true;
		}
	}

	return false;
}

void USMGraphK2Node_FunctionNode::PostPlacedNewNode()
{
	if (USMGraphK2Node_RuntimeNodeContainer* Container = GetRuntimeContainer())
	{
		RuntimeNodeGuid = Container->GetRunTimeNodeChecked()->GetNodeGuid();
	}
}

void USMGraphK2Node_FunctionNode::PostPasteNode()
{
	// Skip parent handling all together. Duplicating this type of node is fine.
	UK2Node::PostPasteNode();
	if (USMGraphK2Node_RuntimeNodeContainer* Container = GetRuntimeContainer())
	{
		RuntimeNodeGuid = Container->GetRunTimeNodeChecked()->GetNodeGuid();
	}
}

bool USMGraphK2Node_FunctionNode::IsCompatibleWithGraph(UEdGraph const* Graph) const
{
	return Graph->IsA<USMTransitionGraph>() || Graph->IsA<USMStateGraph>();
}

UEdGraphPin* USMGraphK2Node_FunctionNode::GetInputPin() const
{
	const int32 VarInputPin = INDEX_PIN_INPUT + 1;

	if (Pins.Num() <= VarInputPin || Pins[VarInputPin]->Direction == EGPD_Output)
	{
		return nullptr;
	}

	return Pins[VarInputPin];
}

bool USMGraphK2Node_FunctionNode::ExpandAndWireStandardFunction(UFunction* Function, UEdGraphPin* SelfPin,
	FSMKismetCompilerContext& CompilerContext, USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer,
	FProperty* NodeProperty)
{
	// Call end function.
	UK2Node_CallFunction* StartFunctionNode = FSMBlueprintEditorUtils::CreateFunctionCall(CompilerContext.ConsolidatedEventGraph, Function);
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(StartFunctionNode, this);
	
	UEdGraphPin* SelfPinNew = StartFunctionNode->FindPinChecked(FName(USMGraphK2Schema::PN_Self));
	UEdGraphPin* ExecutePinNew = StartFunctionNode->FindPinChecked(USMGraphK2Schema::PN_Execute);
	UEdGraphPin* ThenPinNew = StartFunctionNode->FindPinChecked(USMGraphK2Schema::PN_Then);

	UEdGraphPin* ExecutePinOld = FindPinChecked(USMGraphK2Schema::PN_Execute);
	UEdGraphPin* ThenPinOld = FindPinChecked(USMGraphK2Schema::PN_Then);

	// Wire the reference pin to the self pin so we are calling start on the reference.
	CompilerContext.ConsolidatedEventGraph->GetSchema()->TryCreateConnection(SelfPin, SelfPinNew);

	// Wire old pins to new pins.
	ExecutePinNew->CopyPersistentDataFromOldPin(*ExecutePinOld);
	CompilerContext.MessageLog.NotifyIntermediatePinCreation(ExecutePinNew, ExecutePinOld);
	ThenPinNew->CopyPersistentDataFromOldPin(*ThenPinOld);
	CompilerContext.MessageLog.NotifyIntermediatePinCreation(ThenPinNew, ThenPinOld);

	BreakAllNodeLinks();
	return true;
}

void USMGraphK2Node_FunctionNode_StateMachineRef::GetMenuActions(
	FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
}

USMGraphK2Node_FunctionNode_StateMachineRef::USMGraphK2Node_FunctionNode_StateMachineRef(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool USMGraphK2Node_FunctionNode_StateMachineRef::IsCompatibleWithGraph(UEdGraph const* Graph) const
{
	return Graph->IsA<USMIntermediateGraph>();
}

FText USMGraphK2Node_FunctionNode_StateMachineRef::GetMenuCategory() const
{
	return FText::FromString(STATE_MACHINE_INSTANCE_CALL_CATEGORY);
}

USMGraphK2Node_StateMachineRef_Start::USMGraphK2Node_StateMachineRef_Start(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USMGraphK2Node_FunctionNode_StateMachineRef::CustomExpandNode(FSMKismetCompilerContext& CompilerContext,
	USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty)
{
}

void USMGraphK2Node_StateMachineRef_Start::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	GetMenuActions_Internal(ActionRegistrar);
}

void USMGraphK2Node_StateMachineRef_Start::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, USMGraphK2Schema::PC_Exec, USMGraphK2Schema::PN_Execute);
	CreatePin(EGPD_Input, USMGraphK2Schema::PC_Object, UObject::StaticClass(), TEXT("Context"));
	CreatePin(EGPD_Output, USMGraphK2Schema::PC_Exec, USMGraphK2Schema::PN_Then);
}

FText USMGraphK2Node_StateMachineRef_Start::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (TitleType == ENodeTitleType::MenuTitle || TitleType == ENodeTitleType::ListView)
	{
		return LOCTEXT("StartStateMachineReference", "Start State Machine Reference");
	}

	return FText::FromString(TEXT("Start State Machine Reference"));
}

FText USMGraphK2Node_StateMachineRef_Start::GetTooltipText() const
{
	return LOCTEXT("StateMachineRefStart", "Start the state machine reference.");
}

void USMGraphK2Node_StateMachineRef_Start::CustomExpandNode(FSMKismetCompilerContext& CompilerContext,
	USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty)
{
	// Retrieve state machine reference.
	UFunction* Function = USMInstance::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(USMInstance, GetReferencedInstanceByGuid));
	UK2Node_CallFunction* GetReferenceFunctionNode = CreateFunctionCallWithGuidInput(Function, CompilerContext, RuntimeNodeContainer, NodeProperty);

	UEdGraphPin* GetReferenceOutputPin = GetReferenceFunctionNode->GetReturnValuePin();

	// Call state machine start.

	UFunction* StartFunction = USMInstance::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(USMInstance, StartWithNewContext));
	UK2Node_CallFunction* StartFunctionNode = FSMBlueprintEditorUtils::CreateFunctionCall(CompilerContext.ConsolidatedEventGraph, StartFunction);
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(StartFunctionNode, this);

	UEdGraphPin* SelfPinIn = StartFunctionNode->FindPinChecked(FName("self"));
	UEdGraphPin* ContextPinIn = StartFunctionNode->FindPinChecked(FName("Context"));
	UEdGraphPin* ExecutePinIn = StartFunctionNode->FindPinChecked(FName("execute"));
	UEdGraphPin* ThenPinIn = StartFunctionNode->FindPinChecked(FName("then"));

	UEdGraphPin* ContextPinOut = FindPinChecked(FName("Context"));
	UEdGraphPin* ExecutePinOut = FindPinChecked(FName("execute"));
	UEdGraphPin* ThenPinOut = FindPinChecked(FName("then"));

	// Wire the reference pin to the self pin so we are calling start on the reference.
	CompilerContext.ConsolidatedEventGraph->GetSchema()->TryCreateConnection(GetReferenceOutputPin, SelfPinIn);

	// Wire old pins to new pins.
	ContextPinIn->CopyPersistentDataFromOldPin(*ContextPinOut);
	ExecutePinIn->CopyPersistentDataFromOldPin(*ExecutePinOut);
	ThenPinIn->CopyPersistentDataFromOldPin(*ThenPinOut);

	BreakAllNodeLinks();
}



USMGraphK2Node_StateMachineRef_Update::USMGraphK2Node_StateMachineRef_Update(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USMGraphK2Node_StateMachineRef_Update::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	GetMenuActions_Internal(ActionRegistrar);
}

void USMGraphK2Node_StateMachineRef_Update::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, USMGraphK2Schema::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Input, USMGraphK2Schema::PC_Real, USMGraphK2Schema::PC_Float, TEXT("DeltaSeconds"));
	CreatePin(EGPD_Output, USMGraphK2Schema::PC_Exec, UEdGraphSchema_K2::PN_Then);
}

FText USMGraphK2Node_StateMachineRef_Update::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (TitleType == ENodeTitleType::MenuTitle || TitleType == ENodeTitleType::ListView)
	{
		return LOCTEXT("UpdateStateMachineReference", "Update State Machine Reference");
	}

	return FText::FromString(TEXT("Update State Machine Reference"));
}

FText USMGraphK2Node_StateMachineRef_Update::GetTooltipText() const
{
	return LOCTEXT("StateMachineRefUpdate", "Update the state machine reference.");
}

void USMGraphK2Node_StateMachineRef_Update::CustomExpandNode(FSMKismetCompilerContext& CompilerContext,
	USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty)
{
	// Retrieve state machine reference.
	UFunction* Function = USMInstance::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(USMInstance, GetReferencedInstanceByGuid));
	UK2Node_CallFunction* GetReferenceFunctionNode = CreateFunctionCallWithGuidInput(Function, CompilerContext, RuntimeNodeContainer, NodeProperty);

	UEdGraphPin* GetReferenceOutputPin = GetReferenceFunctionNode->GetReturnValuePin();

	// Call state machine start.

	UFunction* StartFunction = USMInstance::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(USMInstance, RunUpdateAsReference));
	UK2Node_CallFunction* StartFunctionNode = FSMBlueprintEditorUtils::CreateFunctionCall(CompilerContext.ConsolidatedEventGraph, StartFunction);
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(StartFunctionNode, this);
	
	UEdGraphPin* SelfPinIn = StartFunctionNode->FindPinChecked(FName("self"));
	UEdGraphPin* SecondsPinIn = StartFunctionNode->FindPinChecked(FName("DeltaSeconds"));
	UEdGraphPin* ExecutePinIn = StartFunctionNode->FindPinChecked(FName("execute"));
	UEdGraphPin* ThenPinIn = StartFunctionNode->FindPinChecked(FName("then"));

	UEdGraphPin* SecondsPinOut = FindPinChecked(FName("DeltaSeconds"));
	UEdGraphPin* ExecutePinOut = FindPinChecked(FName("execute"));
	UEdGraphPin* ThenPinOut = FindPinChecked(FName("then"));

	// Wire the reference pin to the self pin so we are calling start on the reference.
	CompilerContext.ConsolidatedEventGraph->GetSchema()->TryCreateConnection(GetReferenceOutputPin, SelfPinIn);

	// Wire old pins to new pins.
	SecondsPinIn->CopyPersistentDataFromOldPin(*SecondsPinOut);
	ExecutePinIn->CopyPersistentDataFromOldPin(*ExecutePinOut);
	ThenPinIn->CopyPersistentDataFromOldPin(*ThenPinOut);

	BreakAllNodeLinks();
}



USMGraphK2Node_StateMachineRef_Stop::USMGraphK2Node_StateMachineRef_Stop(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void USMGraphK2Node_StateMachineRef_Stop::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	GetMenuActions_Internal(ActionRegistrar);
}

void USMGraphK2Node_StateMachineRef_Stop::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, USMGraphK2Schema::PC_Exec, USMGraphK2Schema::PN_Execute);
	CreatePin(EGPD_Output, USMGraphK2Schema::PC_Exec, USMGraphK2Schema::PN_Then);
}

FText USMGraphK2Node_StateMachineRef_Stop::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (TitleType == ENodeTitleType::MenuTitle || TitleType == ENodeTitleType::ListView)
	{
		return LOCTEXT("StopStateMachineReference", "Stop State Machine Reference");
	}

	return FText::FromString(TEXT("Stop State Machine Reference"));
}

FText USMGraphK2Node_StateMachineRef_Stop::GetTooltipText() const
{
	return LOCTEXT("StateMachineRefStop", "Stop the state machine reference.");
}

void USMGraphK2Node_StateMachineRef_Stop::CustomExpandNode(FSMKismetCompilerContext& CompilerContext,
	USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty)
{
	// Retrieve state machine reference.
	UFunction* Function = USMInstance::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(USMInstance, GetReferencedInstanceByGuid));
	UK2Node_CallFunction* GetReferenceFunctionNode = CreateFunctionCallWithGuidInput(Function, CompilerContext, RuntimeNodeContainer, NodeProperty);

	UEdGraphPin* GetReferenceOutputPin = GetReferenceFunctionNode->GetReturnValuePin();

	// Call state machine start.

	UFunction* StartFunction = USMInstance::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(USMInstance, Stop));
	UK2Node_CallFunction* StartFunctionNode = FSMBlueprintEditorUtils::CreateFunctionCall(CompilerContext.ConsolidatedEventGraph, StartFunction);
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(StartFunctionNode, this);
	
	UEdGraphPin* SelfPinIn = StartFunctionNode->FindPinChecked(FName("self"));
	UEdGraphPin* ExecutePinIn = StartFunctionNode->FindPinChecked(FName("execute"));
	UEdGraphPin* ThenPinIn = StartFunctionNode->FindPinChecked(FName("then"));

	UEdGraphPin* ExecutePinOut = FindPinChecked(FName("execute"));
	UEdGraphPin* ThenPinOut = FindPinChecked(FName("then"));

	// Wire the reference pin to the self pin so we are calling start on the reference.
	CompilerContext.ConsolidatedEventGraph->GetSchema()->TryCreateConnection(GetReferenceOutputPin, SelfPinIn);

	// Wire old pins to new pins.
	ExecutePinIn->CopyPersistentDataFromOldPin(*ExecutePinOut);
	ThenPinIn->CopyPersistentDataFromOldPin(*ThenPinOut);

	BreakAllNodeLinks();
}


USMGraphK2Node_FunctionNode_TransitionEvent::USMGraphK2Node_FunctionNode_TransitionEvent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), DelegateOwnerClass(nullptr), DelegateOwnerInstance()
{
	bCanRenameNode = false;
}

void USMGraphK2Node_FunctionNode_TransitionEvent::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, USMGraphK2Schema::PC_Exec, UEdGraphSchema_K2::PN_Then);
	UpdateNodeFromFunction();
}

void USMGraphK2Node_FunctionNode_TransitionEvent::GetMenuActions(
	FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
}

bool USMGraphK2Node_FunctionNode_TransitionEvent::IsCompatibleWithGraph(UEdGraph const* Graph) const
{
	return Graph->IsA<USMTransitionGraph>();
}

bool USMGraphK2Node_FunctionNode_TransitionEvent::IsActionFilteredOut(FBlueprintActionFilter const& Filter)
{
	return true;
}

FText USMGraphK2Node_FunctionNode_TransitionEvent::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromName(DelegatePropertyName);
}

bool USMGraphK2Node_FunctionNode_TransitionEvent::HasExternalDependencies(TArray<UStruct*>* OptionalOutput) const
{
	const UBlueprint* SourceBlueprint = GetBlueprint();

	FProperty* MCProperty = FSMBlueprintEditorUtils::GetDelegateProperty(DelegatePropertyName, DelegateOwnerClass);
	UClass* PropertySourceClass = MCProperty ? MCProperty->GetOwnerClass() : nullptr;
	const bool bPropertyResult = (PropertySourceClass != nullptr) && (PropertySourceClass->ClassGeneratedBy != SourceBlueprint);
	if (bPropertyResult && OptionalOutput)
	{
		OptionalOutput->AddUnique(PropertySourceClass);
	}

	UFunction* Signature = GetDelegateFunction();
	UClass* SignatureSourceClass = Signature ? Signature->GetOwnerClass() : nullptr;
	const bool bSignatureResult = (SignatureSourceClass != nullptr) && (SignatureSourceClass->ClassGeneratedBy != SourceBlueprint);
	if (bSignatureResult && OptionalOutput)
	{
		OptionalOutput->AddUnique(Signature);
	}
	
	const bool bSuperResult = Super::HasExternalDependencies(OptionalOutput);
	return bSignatureResult || bPropertyResult || bSuperResult;
}

void USMGraphK2Node_FunctionNode_TransitionEvent::ReconstructNode()
{
	Super::ReconstructNode();
	if (FMulticastDelegateProperty* DelegateProperty = FSMBlueprintEditorUtils::GetDelegateProperty(DelegatePropertyName, DelegateOwnerClass, GetDelegateFunction()))
	{
		SetEventReferenceFromDelegate(DelegateProperty, DelegateOwnerInstance);
	}
}

void USMGraphK2Node_FunctionNode_TransitionEvent::PostCompileValidate(FCompilerResultsLog& MessageLog)
{
	Super::PostCompileValidate(MessageLog);

	// If the delegate has parameters added or removed we won't know so we'll refresh on compile.
	if (UBlueprint* Blueprint = FSMBlueprintEditorUtils::FindBlueprintForNode(this))
	{
		if (!Blueprint->HasAnyFlags(RF_NeedPostLoad))
		{
			ReconstructNode();
		}
	}
}

void USMGraphK2Node_FunctionNode_TransitionEvent::CustomExpandNode(FSMKismetCompilerContext& CompilerContext,
                                                                   USMGraphK2Node_RuntimeNodeContainer* RuntimeNodeContainer, FProperty* NodeProperty)
{
	const UFunction* CustomFunction = GetDelegateFunction();
	if (!CustomFunction)
	{
		CompilerContext.MessageLog.Error(TEXT("Could not locate function for node @@"), this);
		return;
	}

	const FString NameSuffix = FString::Printf(TEXT("Event_%s"), *EventReference.GetMemberName().ToString());
	const FString CustomEventName = CompilerContext.CreateUniqueName(RuntimeNodeContainer, NameSuffix);

	// Create a custom event node to take the place of this node.
	UK2Node_CustomEvent* CustomEvent = UK2Node_CustomEvent::CreateFromFunction(FVector2D(NodePosX, NodePosY), CompilerContext.ConsolidatedEventGraph,
		CustomEventName, CustomFunction);
	if (!CustomEvent)
	{
		CompilerContext.MessageLog.Error(TEXT("Could not create custom event for node @@"), this);
		return;
	}
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(CustomEvent, this);
	
	UEdGraphPin* DelegateOutputPin = nullptr;
	for (UEdGraphPin* EventPin : CustomEvent->Pins)
	{
		if (UEdGraphPin* OurPin = FindPin(EventPin->PinName, EventPin->Direction))
		{
			EventPin->CopyPersistentDataFromOldPin(*OurPin);
			CompilerContext.MessageLog.NotifyIntermediatePinCreation(EventPin, OurPin);
		}

		if (EventPin->PinName == UK2Node_Event::DelegateOutputName && EventPin->PinType.PinCategory == USMGraphK2Schema::PC_Delegate)
		{
			DelegateOutputPin = EventPin;
		}
	}

	if (!DelegateOutputPin)
	{
		CompilerContext.MessageLog.Error(TEXT("Could not locate delegate output pin for node @@"), this);
		return;
	}

	BreakAllNodeLinks();

	const FMulticastDelegateProperty* DelegateProperty = FSMBlueprintEditorUtils::GetDelegateProperty(DelegatePropertyName, DelegateOwnerClass, CustomFunction);
	if (!DelegateProperty)
	{
		CompilerContext.MessageLog.Error(TEXT("Delegate property missing for transition event node @@"), this);
		return;
	}
	
	// Create initialized node to wire up binding to the event.
	USMGraphK2Node_TransitionInitializedNode* TransitionInitializedNode = CompilerContext.ConsolidatedEventGraph->CreateIntermediateNode<USMGraphK2Node_TransitionInitializedNode>();
	TransitionInitializedNode->AllocateDefaultPins();
	TransitionInitializedNode->ContainerOwnerGuid = RuntimeNodeContainer->ContainerOwnerGuid;
	TransitionInitializedNode->RuntimeNodeGuid = RuntimeNodeContainer->GetRunTimeNodeChecked()->GetNodeGuid();
	
	const UBlueprintNodeSpawner* AddSpawner = UBlueprintDelegateNodeSpawner::Create(UK2Node_AddDelegate::StaticClass(), DelegateProperty);
	const UK2Node_AddDelegate* AddDelegateNode = CastChecked<UK2Node_AddDelegate>(AddSpawner->Invoke(CompilerContext.ConsolidatedEventGraph, IBlueprintNodeBinder::FBindingSet(), FVector2D()));
	UEdGraphPin* DelegateInputPin = AddDelegateNode->FindPin(UEdGraphSchema_K2::PN_DelegateEntry);

	const UEdGraphSchema* Schema = GetSchema();
	
	Schema->TryCreateConnection(TransitionInitializedNode->GetOutputPin(), AddDelegateNode->GetExecPin());
	Schema->TryCreateConnection(DelegateOutputPin, DelegateInputPin);

	// Create shutdown node to clear binding.
	USMGraphK2Node_TransitionShutdownNode* TransitionShutdownNode = Cast<USMGraphK2Node_TransitionShutdownNode>(CompilerContext.ConsolidatedEventGraph->CreateIntermediateNode<USMGraphK2Node_TransitionShutdownNode>());
	TransitionShutdownNode->AllocateDefaultPins();
	TransitionShutdownNode->ContainerOwnerGuid = RuntimeNodeContainer->ContainerOwnerGuid;
	TransitionShutdownNode->RuntimeNodeGuid = RuntimeNodeContainer->GetRunTimeNodeChecked()->GetNodeGuid();

	const UBlueprintNodeSpawner* RemoveSpawner = UBlueprintDelegateNodeSpawner::Create(UK2Node_RemoveDelegate::StaticClass(), DelegateProperty);
	const UK2Node_RemoveDelegate* RemoveDelegateNode = CastChecked<UK2Node_RemoveDelegate>(RemoveSpawner->Invoke(CompilerContext.ConsolidatedEventGraph, IBlueprintNodeBinder::FBindingSet(), FVector2D()));
	DelegateInputPin = RemoveDelegateNode->FindPin(UEdGraphSchema_K2::PN_DelegateEntry);

	Schema->TryCreateConnection(TransitionShutdownNode->GetOutputPin(), RemoveDelegateNode->GetExecPin());
	Schema->TryCreateConnection(DelegateOutputPin, DelegateInputPin);

	// Wire correct instance.
	if (DelegateOwnerInstance >= SMDO_Context)
	{
		if (DelegateOwnerClass == nullptr)
		{
			CompilerContext.MessageLog.Error(TEXT("DelegateOwnerClass not assigned for node @@."), this);
			return;
		}

		UEdGraphPin* FromPin = nullptr;
		
		if (DelegateOwnerInstance == SMDO_Context)
		{
			UFunction* Function = Function = USMInstance::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(USMInstance, GetContext));
			const UK2Node_CallFunction* ContextFunctionNode = FSMBlueprintEditorUtils::CreateFunctionCall(CompilerContext.ConsolidatedEventGraph, Function);
			FromPin = ContextFunctionNode->GetReturnValuePin();
		}
		else if (DelegateOwnerInstance == SMDO_PreviousState)
		{
			USMGraphK2Node_StateReadNode_GetNodeInstance* NodeInstance = CompilerContext.ConsolidatedEventGraph->CreateIntermediateNode<USMGraphK2Node_StateReadNode_GetNodeInstance>();
			NodeInstance->ContainerOwnerGuid = RuntimeNodeContainer->ContainerOwnerGuid;
			NodeInstance->RuntimeNodeGuid = RuntimeNodeContainer->GetRunTimeNodeChecked()->GetNodeGuid();
			NodeInstance->AllocatePinsForType(TransitionClass);

			UFunction* Function = USMTransitionInstance::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(USMTransitionInstance, GetPreviousStateInstance));
			const UK2Node_CallFunction* PreviousTransitionNode = FSMBlueprintEditorUtils::CreateFunctionCall(CompilerContext.ConsolidatedEventGraph, Function);

			const FString SelfPinName = "Self";
			UEdGraphPin* SelfPin = PreviousTransitionNode->FindPinChecked(SelfPinName, EEdGraphPinDirection::EGPD_Input);

			Schema->TryCreateConnection(NodeInstance->GetOutputPin(), SelfPin);
			
			FromPin = PreviousTransitionNode->GetReturnValuePin();
		}

		if (!ensure(FromPin))
		{
			CompilerContext.MessageLog.Error(TEXT("Could not find FromPin while expanding auto-bound transition @@"), this);
			return;
		}
		
		UK2Node_DynamicCast* CastNode = CompilerContext.SpawnIntermediateNode<UK2Node_DynamicCast>(this, CompilerContext.ConsolidatedEventGraph);

		UClass* Class = FSMBlueprintEditorUtils::TryGetFullyGeneratedClass(DelegateOwnerClass);  // 4.25 won't cast the skeleton class correctly during a play-in-editor session.
		CastNode->TargetType = Class;
		CastNode->PostPlacedNewNode();
		CastNode->SetPurity(true);
		CastNode->ReconstructNode();

		if (CastNode->GetCastResultPin() == nullptr)
		{
			CompilerContext.MessageLog.Error(TEXT("Can't create cast node for @@."), this);
			return;
		}

		// Context to Cast Source.
		if (!Schema->TryCreateConnection(FromPin, CastNode->GetCastSourcePin()))
		{
			CompilerContext.MessageLog.Error(TEXT("Could not cast to DelegateOwnerClass @@."), this);
			return;
		}

		// Cast Result to AddDelegate Target.
		UEdGraphPin* AddDelegateTargetPin = AddDelegateNode->FindPin(UEdGraphSchema_K2::PN_Self, EGPD_Input);
		if (!Schema->TryCreateConnection(CastNode->GetCastResultPin(), AddDelegateTargetPin))
		{
			CompilerContext.MessageLog.Error(TEXT("Cast result of Context to DelegateOwnerClass not accepted in AddDelegate for node @@."), this);
			return;
		}

		// Cast Result to RemoveDelegate Target.
		UEdGraphPin* RemoveDelegateTargetPin = RemoveDelegateNode->FindPin(UEdGraphSchema_K2::PN_Self, EGPD_Input);
		if (!Schema->TryCreateConnection(CastNode->GetCastResultPin(), RemoveDelegateTargetPin))
		{
			CompilerContext.MessageLog.Error(TEXT("Cast result of Context to DelegateOwnerClass not accepted in RemoveDelegate for node @@."), this);
			return;
		}
	}
}

void USMGraphK2Node_FunctionNode_TransitionEvent::SetEventReferenceFromDelegate(FMulticastDelegateProperty* Delegate, ESMDelegateOwner InstanceType)
{
	DelegatePropertyName = Delegate->GetFName();
	DelegateOwnerClass = Delegate->GetOwnerClass();
	DelegateOwnerInstance = InstanceType;
	EventReference.SetFromField<UFunction>(Delegate->SignatureFunction, false);
}

UFunction* USMGraphK2Node_FunctionNode_TransitionEvent::GetDelegateFunction() const
{
	if (UFunction* FoundFunction = EventReference.ResolveMember<UFunction>(DelegateOwnerClass))
	{
		return FoundFunction;
	}

	// During compile non-native delegates declared in this blueprint won't be in the funcmap so we'll need to check the skeleton class instead.
	if (DelegateOwnerClass)
	{
		if (const UBlueprint* Blueprint = Cast<UBlueprint>(DelegateOwnerClass->ClassGeneratedBy))
		{
			if (UFunction* FoundFunction = EventReference.ResolveMember<UFunction>(Blueprint->SkeletonGeneratedClass))
			{
				return FoundFunction;
			}
		}
	}

	// TODO: Likely not necessary.
	if (const FMulticastDelegateProperty* DelegateProperty = FSMBlueprintEditorUtils::GetDelegateProperty(DelegatePropertyName, DelegateOwnerClass))
	{
		if (DelegateProperty->SignatureFunction)
		{
			return DelegateProperty->SignatureFunction;
		}
	}

	return nullptr;
}

void USMGraphK2Node_FunctionNode_TransitionEvent::UpdateNodeFromFunction()
{
	UFunction* CustomFunction = GetDelegateFunction();
	if (!CustomFunction)
	{
		return;
	}
	
	UK2Node_CustomEvent* CustomEvent = UK2Node_CustomEvent::CreateFromFunction(FVector2D(NodePosX, NodePosY), GetGraph(), EventReference.GetMemberName().ToString(), CustomFunction, false);

	TArray<UEdGraphPin*> OldPins = Pins;

	// Mark pending kill.
	for (UEdGraphPin* OldPin : OldPins)
	{
		RemovePin(OldPin);
	}
	Pins.Reset();

	for (UEdGraphPin* Pin : CustomEvent->Pins)
	{
		if (Pin->PinName == UK2Node_Event::DelegateOutputName && Pin->PinType.PinCategory == USMGraphK2Schema::PC_Delegate)
		{
			continue;
		}

		CreatePin(Pin->Direction, Pin->PinType, Pin->PinName);

		// Remap old pins if they exist.
		if (UEdGraphPin* const* OldPin = OldPins.FindByPredicate([&](const UEdGraphPin* P)
		{
			return P->PinName == Pin->PinName && P->Direction == Pin->Direction;
		}))
		{
			Pin->CopyPersistentDataFromOldPin(**OldPin);
		}
	}
	
	FSMBlueprintEditorUtils::RemoveNode(FSMBlueprintEditorUtils::FindBlueprintForNodeChecked(this), CustomEvent, true);
}

#undef LOCTEXT_NAMESPACE
