// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "SMGraphNode_StateNode.h"

#include "SMGraphNode_LinkStateNode.h"
#include "Blueprints/SMBlueprintEditor.h"
#include "Utilities/SMBlueprintEditorUtils.h"
#include "Utilities/SMNodeInstanceUtils.h"
#include "Graph/SMGraph.h"
#include "Graph/SMStateGraph.h"
#include "Graph/Schema/SMStateGraphSchema.h"
#include "Helpers/SMGraphK2Node_FunctionNodes_NodeInstance.h"
#include "RootNodes/SMGraphK2Node_StateUpdateNode.h"
#include "RootNodes/SMGraphK2Node_StateEndNode.h"
#include "RootNodes/SMGraphK2Node_IntermediateNodes.h"
#include "RootNodes/SMGraphK2Node_TransitionInitializedNode.h"
#include "RootNodes/SMGraphK2Node_TransitionShutdownNode.h"
#include "SMGraphNode_TransitionEdge.h"
#include "SMUnrealTypeDefs.h"

#include "SMUtils.h"

#include "Kismet2/Kismet2NameValidators.h"
#include "UObject/UObjectThreadContext.h"

#define LOCTEXT_NAMESPACE "SMGraphStateNode"

class FSMStateNodeNameValidator : public FStringSetNameValidator
{
public:
	FSMStateNodeNameValidator(const USMGraphNode_StateNodeBase* InStateNode)
		: FStringSetNameValidator(FString())
	{
		TArray<USMGraphNode_StateNodeBase*> Nodes;
		USMGraph* StateMachine = CastChecked<USMGraph>(InStateNode->GetOuter());

		StateMachine->GetNodesOfClass<USMGraphNode_StateNodeBase>(Nodes);
		for (auto NodeIt = Nodes.CreateIterator(); NodeIt; ++NodeIt)
		{
			USMGraphNode_StateNodeBase* Node = *NodeIt;
			if (Node != InStateNode)
			{
				Names.Add(Node->GetStateName());
			}
		}
	}

	// Begin FSMStateNodeNameValidator
	virtual EValidatorResult IsValid(const FString& Name, bool bOriginal) override
	{
		EValidatorResult Result = FStringSetNameValidator::IsValid(Name, bOriginal);

		if (Result == EValidatorResult::Ok)
		{
			if (Name.Len() > 100)
			{
				Result = EValidatorResult::TooLong;
			}
			else if (FSMBlueprintEditorUtils::GetProjectEditorSettings()->bRestrictInvalidCharacters)
			{
				FText Reason;
				if (!FName(Name).IsValidXName(Reason, LD_INVALID_STATENAME_CHARACTERS))
				{
					Result = EValidatorResult::ContainsInvalidCharacters;
				}
				else if (Name.Compare(TEXT("Sequencer"), ESearchCase::IgnoreCase) == 0)
				{
					// Can cause problems during copy & paste.
					Result = EValidatorResult::LocallyInUse;
				}
			}
		}

		return Result;
	}
	// End FSMStateNodeNameValidator
};

USMGraphNode_StateNodeBase::USMGraphNode_StateNodeBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), bAlwaysUpdate_DEPRECATED(false), bDisableTickTransitionEvaluation_DEPRECATED(false),
	  bEvalTransitionsOnStart_DEPRECATED(false), bExcludeFromAnyState_DEPRECATED(false),
	  bCanTransitionToSelf(false), bRequestInitialAnimation(false)
{
}

void USMGraphNode_StateNodeBase::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, TEXT("Transition"), TEXT("In"));
	CreatePin(EGPD_Output, TEXT("Transition"), TEXT("Out"));
}

FText USMGraphNode_StateNodeBase::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	FString StateName = GetStateName();
	if (USMStateInstance_Base* StateInstance = Cast<USMStateInstance_Base>(NodeInstanceTemplate))
	{
		if (StateInstance->ShouldUseDisplayNameOnly())
		{
			StateName = FSMNodeInstanceUtils::GetNodeDisplayName(StateInstance);
		}
	}
	
	return FText::FromString(StateName);
}

bool USMGraphNode_StateNodeBase::GetCanRenameNode() const
{
	if (USMStateInstance_Base* StateInstance = Cast<USMStateInstance_Base>(NodeInstanceTemplate))
	{
		if (StateInstance->ShouldUseDisplayNameOnly())
		{
			return false;
		}
	}

	return Super::GetCanRenameNode();
}

void USMGraphNode_StateNodeBase::AutowireNewNode(UEdGraphPin* FromPin)
{
	Super::AutowireNewNode(FromPin);

	if (FromPin != nullptr)
	{
		UEdGraphPin* InputPin = GetInputPin();

		if (InputPin && GetSchema()->TryCreateConnection(FromPin, InputPin))
		{
			FromPin->GetOwningNode()->NodeConnectionListChanged();
		}
	}
}

void USMGraphNode_StateNodeBase::PostPlacedNewNode()
{
	SetToCurrentVersion();
	
	// Create a new state machine graph
	check(BoundGraph == nullptr);
	BoundGraph = FBlueprintEditorUtils::CreateNewGraph(
		this,
		NAME_None,
		USMStateGraph::StaticClass(),
		USMStateGraphSchema::StaticClass());
	check(BoundGraph);

	// Find an interesting name
	TSharedPtr<INameValidatorInterface> NameValidator = FNameValidatorFactory::MakeValidator(this);
	FBlueprintEditorUtils::RenameGraphWithSuggestion(BoundGraph, NameValidator, TEXT("State"));

	// Initialize the state machine graph
	const UEdGraphSchema* Schema = BoundGraph->GetSchema();
	Schema->CreateDefaultNodesForGraph(*BoundGraph);

	// Add the new graph as a child of our parent graph
	UEdGraph* ParentGraph = GetGraph();

	// TODO: This search is slow on large state machines!
	if (ParentGraph->SubGraphs.Find(BoundGraph) == INDEX_NONE)
	{
		ParentGraph->Modify();
		ParentGraph->SubGraphs.Add(BoundGraph);
	}

	if (bGenerateTemplateOnNodePlacement)
	{
		InitTemplate();
	}
}

void USMGraphNode_StateNodeBase::PostPasteNode()
{
	// Look for a unique name. We have to perform manual handling for this before passing it off to Unreal.
	// Every copy resets the unique name count, so State_1 pasted might be State_1_1. This makes it so it is State_2.
	// t-155
	const FString UniqueName = FSMBlueprintEditorUtils::FindUniqueName(GetStateName(), GetOwningStateMachineGraph());
	
	const TSharedPtr<INameValidatorInterface> NameValidator = FNameValidatorFactory::MakeValidator(this);
	FBlueprintEditorUtils::RenameGraphWithSuggestion(BoundGraph, NameValidator, UniqueName);
	
	TArray<UEdGraphNode*> ContainedNodes;
	FSMBlueprintEditorUtils::GetAllNodesOfClassNested<UEdGraphNode>(BoundGraph, ContainedNodes);

	for (UEdGraphNode* GraphNode : ContainedNodes)
	{
		GraphNode->CreateNewGuid();
		GraphNode->PostPasteNode();
		// Required to correct context display issues.
		GraphNode->ReconstructNode();
	}

	Super::PostPasteNode();

	bRequestInitialAnimation = true;
}

void USMGraphNode_StateNodeBase::DestroyNode()
{
	Modify();
	if (BoundGraph)
	{
		BoundGraph->Modify();
	}
	
	UEdGraph* GraphToRemove = BoundGraph;

	BoundGraph = nullptr;
	Super::DestroyNode();

	if (GraphToRemove)
	{
		UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNodeChecked(this);
		FBlueprintEditorUtils::RemoveGraph(Blueprint, GraphToRemove, EGraphRemoveFlags::Recompile);
	}
}

TSharedPtr<INameValidatorInterface> USMGraphNode_StateNodeBase::MakeNameValidator() const
{
	return MakeShareable(new FSMStateNodeNameValidator(this));
}

void USMGraphNode_StateNodeBase::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	// Template has been changed.
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(USMGraphNode_StateNodeBase, NodeInstanceTemplate))
	{
		//  Check if it's a property we care about.
		FEditPropertyChain::TDoubleLinkedListNode* MemberNode = PropertyChangedEvent.PropertyChain.GetActiveMemberNode();
		if (MemberNode && MemberNode->GetNextNode() && MemberNode->GetValue())
		{
			const FName Name = PropertyChangedEvent.PropertyChain.GetActiveMemberNode()->GetNextNode()->GetValue()->GetFName();
			if (Name == GET_MEMBER_NAME_CHECKED(USMStateInstance_Base, bDefaultToParallel))
			{
				for (int32 Idx = 0; Idx < GetOutputPin()->LinkedTo.Num(); ++Idx)
				{
					if (USMGraphNode_TransitionEdge* Transition = GetNextTransition(Idx))
					{
						if (USMTransitionInstance* Instance = Transition->GetNodeTemplateAs<USMTransitionInstance>())
						{
							Instance->SetRunParallel(ShouldDefaultTransitionsToParallel());
						}
					}
				}
			}
		}
	}
}

void USMGraphNode_StateNodeBase::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);
}

void USMGraphNode_StateNodeBase::ImportDeprecatedProperties()
{
	Super::ImportDeprecatedProperties();

	if (USMStateInstance_Base* StateInstance = Cast<USMStateInstance_Base>(GetNodeTemplate()))
	{
		StateInstance->SetAlwaysUpdate(bAlwaysUpdate_DEPRECATED);
		StateInstance->SetDisableTickTransitionEvaluation(bDisableTickTransitionEvaluation_DEPRECATED);
		StateInstance->SetEvalTransitionsOnStart(bEvalTransitionsOnStart_DEPRECATED);
		StateInstance->SetExcludeFromAnyState(bExcludeFromAnyState_DEPRECATED);
	}
}

void USMGraphNode_StateNodeBase::OnNodeMoved(const FVector2D& NewPosition)
{
	{
		// Transition position managed by state.
		TArray<USMGraphNode_TransitionEdge*> AllTransitions;
		GetOutputTransitions(AllTransitions);
		GetInputTransitions(AllTransitions);
		for (USMGraphNode_TransitionEdge* Transition : AllTransitions)
		{
			Transition->SetReadOnlyNodePosition();
		}
	}

	Super::OnNodeMoved(NewPosition);
}

void USMGraphNode_StateNodeBase::PreCompile(FSMKismetCompilerContext& CompilerContext)
{
	Super::PreCompile(CompilerContext);

	// Verify linked states are correct and cleanup if necessary.
	if (const UEdGraph* Graph = GetGraph())
	{
		for (auto It = LinkedStates.CreateIterator(); It; ++It)
		{
			if (!Graph->Nodes.Contains(*It))
			{
				It.RemoveCurrent();
			}
		}
	}
}

void USMGraphNode_StateNodeBase::SetRuntimeDefaults(FSMState_Base& State) const
{
	State.SetNodeName(GetStateName());

	State.NodePosition = NodePosition;
	State.bHasInputEvents = FSMBlueprintEditorUtils::DoesGraphHaveInputEvents(GetBoundGraph());
	
	if (const USMStateInstance_Base* StateInstance = Cast<USMStateInstance_Base>(GetNodeTemplate()))
	{
		State.bAlwaysUpdate = StateInstance->GetAlwaysUpdate();
		State.bDisableTickTransitionEvaluation = StateInstance->GetDisableTickTransitionEvaluation();
		State.bAllowParallelReentry = StateInstance->GetAllowParallelReentry();
		State.bStayActiveOnStateChange = StateInstance->GetStayActiveOnStateChange();
		State.bEvalTransitionsOnStart = StateInstance->GetEvalTransitionsOnStart();
	}
}

bool USMGraphNode_StateNodeBase::IsEndState(bool bCheckAnyState) const
{
	// Must have entry.
	if (!HasInputConnections())
	{
		return false;
	}

	// Check Any States since they add transitions to this node on compile.
	if (bCheckAnyState && FSMBlueprintEditorUtils::IsNodeImpactedFromAnyStateNode(this))
	{
		return false;
	}

	// If no output definitely end state.
	if (const UEdGraphPin* OutputPin = GetOutputPin())
	{
		if (OutputPin->LinkedTo.Num() == 0)
		{
			return true;
		}

		for (const UEdGraphPin* Pin : OutputPin->LinkedTo)
		{
			if (const USMGraphNode_TransitionEdge* Transition = Cast<USMGraphNode_TransitionEdge>(Pin->GetOwningNode()))
			{
				// Transitioning to self doesn't count.
				if (Transition->GetFromState() == Transition->GetToState())
				{
					continue;
				}

				// There has to be some way out of here...
				if (Transition->PossibleToTransition())
				{
					return false;
				}
			}
		}
	}

	return true;
}

bool USMGraphNode_StateNodeBase::HasInputConnections() const
{
	if (UEdGraphPin* Pin = GetInputPin())
	{
		if (Pin->LinkedTo.Num() == 0)
		{
			for (const USMGraphNode_LinkStateNode* LinkedState : LinkedStates)
			{
				if (LinkedState->HasInputConnections())
				{
					return true;
				}
			}

			return false;
		}

		for (const UEdGraphPin* InputPin : Pin->LinkedTo)
		{
			if (InputPin->GetOwningNode()->IsA<USMGraphNode_StateMachineEntryNode>())
			{
				return true;
			}

			if (const USMGraphNode_TransitionEdge* Transition = Cast<USMGraphNode_TransitionEdge>(InputPin->GetOwningNode()))
			{
				// Ignore self and input connections which can't transition.
				if (Transition->GetFromState() == Transition->GetToState() || !Transition->PossibleToTransition())
				{
					continue;
				}

				return true;
			}
		}
	}

	return false;
}

bool USMGraphNode_StateNodeBase::HasOutputConnections() const
{
	if (UEdGraphPin* Pin = GetOutputPin())
	{
		return Pin->LinkedTo.Num() > 0;
	}

	return false;
}

bool USMGraphNode_StateNodeBase::ShouldDefaultTransitionsToParallel() const
{
	if (USMStateInstance_Base* StateInstance = Cast<USMStateInstance_Base>(GetNodeTemplate()))
	{
		return StateInstance->GetDefaultToParallel();
	}

	return false;
}

bool USMGraphNode_StateNodeBase::ShouldExcludeFromAnyState() const
{
	if (USMStateInstance_Base* StateInstance = Cast<USMStateInstance_Base>(GetNodeTemplate()))
	{
		return StateInstance->GetExcludeFromAnyState();
	}

	return false;
}

bool USMGraphNode_StateNodeBase::HasTransitionToNode(UEdGraphNode* Node) const
{
	if (UEdGraphPin* OutputPin = GetOutputPin())
	{
		for (UEdGraphPin* Pin : OutputPin->LinkedTo)
		{
			if (Pin == nullptr)
			{
				continue;
			}
			if (USMGraphNode_TransitionEdge* Transition = Cast<USMGraphNode_TransitionEdge>(Pin->GetOwningNode()))
			{
				if (Transition->GetToState() == Node)
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool USMGraphNode_StateNodeBase::HasTransitionFromNode(UEdGraphNode* Node) const
{
	if (UEdGraphPin* InputPin = GetInputPin())
	{
		for (UEdGraphPin* Pin : InputPin->LinkedTo)
		{
			if (Pin == nullptr)
			{
				continue;
			}
			if (USMGraphNode_TransitionEdge* Transition = Cast<USMGraphNode_TransitionEdge>(Pin->GetOwningNode()))
			{
				if (Transition->GetFromState() == Node)
				{
					return true;
				}
			}
		}
	}

	return false;
}

USMGraphNode_StateNodeBase* USMGraphNode_StateNodeBase::GetPreviousNode(int32 Index /*= 0*/, bool bIncludeReroute /*= false*/) const
{
	if (const USMGraphNode_TransitionEdge* Transition = GetPreviousTransition(Index))
	{
		return Transition->GetFromState(bIncludeReroute);
	}

	return nullptr;
}

USMGraphNode_StateNodeBase* USMGraphNode_StateNodeBase::GetNextNode(int32 Index /*= 0*/, bool bIncludeReroute /*= false*/) const
{
	if (const USMGraphNode_TransitionEdge* Transition = GetNextTransition(Index))
	{
		return Transition->GetToState(bIncludeReroute);
	}

	return nullptr;
}

USMGraphNode_TransitionEdge* USMGraphNode_StateNodeBase::GetPreviousTransition(int32 Index) const
{
	if (UEdGraphPin* InputPin = GetInputPin())
	{
		if (InputPin->LinkedTo.Num() <= Index || InputPin->LinkedTo[Index] == nullptr)
		{
			return nullptr;
		}

		if (USMGraphNode_TransitionEdge* Transition = Cast<USMGraphNode_TransitionEdge>(InputPin->LinkedTo[Index]->GetOwningNode()))
		{
			return Transition;
		}
	}
	return nullptr;
}

USMGraphNode_TransitionEdge* USMGraphNode_StateNodeBase::GetNextTransition(int32 Index) const
{
	if (UEdGraphPin* OutputPin = GetOutputPin())
	{
		if (OutputPin->LinkedTo.Num() <= Index || OutputPin->LinkedTo[Index] == nullptr)
		{
			return nullptr;
		}

		if (USMGraphNode_TransitionEdge* Transition = Cast<USMGraphNode_TransitionEdge>(OutputPin->LinkedTo[Index]->GetOwningNode()))
		{
			return Transition;
		}
	}

	return nullptr;
}

void USMGraphNode_StateNodeBase::GetInputTransitions(TArray<USMGraphNode_TransitionEdge*>& OutTransitions) const
{
	if (UEdGraphPin* InputPin = GetInputPin())
	{
		for (int32 Idx = 0; Idx < InputPin->LinkedTo.Num(); ++Idx)
		{
			if (InputPin->LinkedTo[Idx] == nullptr)
			{
				// Could be null during a paste.
				continue;
			}
			if (USMGraphNode_TransitionEdge* Transition = Cast<USMGraphNode_TransitionEdge>(InputPin->LinkedTo[Idx]->GetOwningNode()))
			{
				OutTransitions.AddUnique(Transition);
			}
		}
	}
}

void USMGraphNode_StateNodeBase::GetOutputTransitions(TArray<USMGraphNode_TransitionEdge*>& OutTransitions) const
{
	if (UEdGraphPin* OutputPin = GetOutputPin())
	{
		for (int32 Idx = 0; Idx < OutputPin->LinkedTo.Num(); ++Idx)
		{
			if (OutputPin->LinkedTo[Idx] == nullptr)
			{
				// Could be null during a paste.
				continue;
			}
			if (USMGraphNode_TransitionEdge* Transition = Cast<USMGraphNode_TransitionEdge>(OutputPin->LinkedTo[Idx]->GetOwningNode()))
			{
				OutTransitions.AddUnique(Transition);
			}
		}
	}
}

UEdGraphPin* USMGraphNode_StateNodeBase::GetConnectedEntryPin() const
{
	if (UEdGraphPin* InputPin = GetInputPin())
	{
		for (int32 Idx = 0; Idx < InputPin->LinkedTo.Num(); ++Idx)
		{
			if (InputPin->LinkedTo[Idx]->GetOwningNode()->IsA<USMGraphNode_StateMachineEntryNode>())
			{
				return InputPin->LinkedTo[Idx];
			}
		}
	}

	return nullptr;
}

FLinearColor USMGraphNode_StateNodeBase::GetBackgroundColorForNodeInstance(const USMNodeInstance* NodeInstance) const
{
	const USMEditorSettings* Settings = FSMBlueprintEditorUtils::GetEditorSettings();
	const FLinearColor* CustomColor = GetCustomBackgroundColor(NodeInstance);
	const FLinearColor ColorModifier = !CustomColor ? FLinearColor(0.6f, 0.6f, 0.6f, 0.5f) : *CustomColor;
	const FLinearColor EndStateColor = !CustomColor ? Settings->EndStateColor * ColorModifier : CastChecked<USMStateInstance_Base>(NodeInstance)->GetEndStateColor();

	if (IsEndState())
	{
		return EndStateColor;
	}

	const FLinearColor DefaultColor = Settings->StateDefaultColor;

	// No input -- node unreachable.
	if (!HasInputConnections())
	{
		return DefaultColor * ColorModifier;
	}

	// State is active
	if (FSMBlueprintEditorUtils::GraphHasAnyLogicConnections(BoundGraph))
	{
		return CustomColor ? *CustomColor * FLinearColor(1.f, 1.f, 1.f, 1.2f) : Settings->StateWithLogicColor * ColorModifier;
	}

	return DefaultColor * ColorModifier;
}

FLinearColor USMGraphNode_StateNodeBase::Internal_GetBackgroundColor() const
{
	return GetBackgroundColorForNodeInstance(NodeInstanceTemplate);
}


USMGraphNode_StateNode::USMGraphNode_StateNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USMGraphNode_StateNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// Enable templates
	bool bStateChange = false;
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(USMGraphNode_StateNode, StateClass))
	{
		InitTemplate();
		// Disable property graph refresh because InitTemplate handles it.
		bCreatePropertyGraphsOnPropertyChange = false;

		bStateChange = true;
	}
	else if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(USMGraphNode_StateNode, StateStack) || PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(FStateStackContainer, StateStackClass))
	{
		if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Duplicate)
		{
			// Array element duplication requires a new template generated.
			const int32 ArrayIndex = PropertyChangedEvent.GetArrayIndex(PropertyChangedEvent.GetPropertyName().ToString());
			if (ArrayIndex >= 0 && ArrayIndex + 1 < StateStack.Num())
			{
				const FStateStackContainer& OriginalStateStack = StateStack[ArrayIndex];
				FStateStackContainer& NewStateStack = StateStack[ArrayIndex + 1];

				NewStateStack.TemplateGuid = FGuid::NewGuid();
				if (OriginalStateStack.NodeStackInstanceTemplate && OriginalStateStack.NodeStackInstanceTemplate->GetClass() != GetDefaultNodeClass())
				{
					if (NewStateStack.NodeStackInstanceTemplate != OriginalStateStack.NodeStackInstanceTemplate)
					{
						// This state *shouldn't* exist because the object isn't deep copied, but who knows if USTRUCT UPROPERTY UObject handling changes?
						NewStateStack.DestroyTemplate();
					}
					
					NewStateStack.NodeStackInstanceTemplate = Cast<USMNodeInstance>(StaticDuplicateObject(OriginalStateStack.NodeStackInstanceTemplate, OriginalStateStack.NodeStackInstanceTemplate->GetOuter()));
					UEngine::CopyPropertiesForUnrelatedObjects(OriginalStateStack.NodeStackInstanceTemplate, NewStateStack.NodeStackInstanceTemplate);
					NewStateStack.NodeStackInstanceTemplate->SetTemplateGuid(NewStateStack.TemplateGuid);

					for (TFieldIterator<FProperty> It(NewStateStack.NodeStackInstanceTemplate->GetClass()); It; ++It)
					{
						// Look for real graph properties (not auto generated from a variable), they won't have had their guids cleared.
						
						FProperty* Property = *It;

						FName VarName = Property->GetFName();
						if (VarName == GET_MEMBER_NAME_CHECKED(USMNodeInstance, ExposedPropertyOverrides))
						{
							continue;
						}

						if (FStructProperty* StructProperty = FSMNodeInstanceUtils::GetGraphPropertyFromProperty(Property))
						{
							TArray<FSMGraphProperty_Base*> GraphProperties;
							USMUtils::BlueprintPropertyToNativeProperty(Property, NewStateStack.NodeStackInstanceTemplate, GraphProperties);
							for (FSMGraphProperty_Base* GraphProperty : GraphProperties)
							{
								GraphProperty->InvalidateGuid();
							}
						}
					}

					FSMBlueprintEditorUtils::DuplicateStackTemplatePropertyGraphs(this, this, NewStateStack, OriginalStateStack.TemplateGuid);
				}
			}
		}
		
		InitStateStack();
	}
	else
	{
		// Template and state stack require full generation, otherwise quick generation is fine.
		bPostEditChangeConstructionRequiresFullRefresh = false;
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
	bCreatePropertyGraphsOnPropertyChange = true;
	bPostEditChangeConstructionRequiresFullRefresh = true;

	if (bStateChange && IsSafeToConditionallyCompile(PropertyChangedEvent.ChangeType))
	{
		FSMBlueprintEditorUtils::ConditionallyCompileBlueprint(FSMBlueprintEditorUtils::FindBlueprintForNodeChecked(this), false);
	}
}

UObject* USMGraphNode_StateNode::GetJumpTargetForDoubleClick() const
{
	if (FSMBlueprintEditorUtils::GetEditorSettings()->StateDoubleClickBehavior == ESMJumpToGraphBehavior::PreferExternalGraph)
	{
		if (const UClass* Class = GetNodeClass())
		{
			if (UBlueprint* NodeBlueprint = UBlueprint::GetBlueprintFromClass(Class))
			{
				return NodeBlueprint;
			}
		}
	}
	
	return Super::GetJumpTargetForDoubleClick();
}

void USMGraphNode_StateNode::DestroyNode()
{
	Super::DestroyNode();
	DestroyStateStack();
}

FSlateIcon USMGraphNode_StateNode::GetIconAndTint(FLinearColor& OutColor) const
{
	return FSlateIcon(FSMUnrealAppStyle::Get().GetStyleSetName(), TEXT("Graph.StateNode.Icon"));
}

void USMGraphNode_StateNode::PlaceDefaultInstanceNodes()
{
	Super::PlaceDefaultInstanceNodes();

	USMGraphK2Node_StateEntryNode* EntryNode = FSMBlueprintEditorUtils::GetFirstNodeOfClassNested<USMGraphK2Node_StateEntryNode>(BoundGraph);
	FSMBlueprintEditorUtils::PlaceNodeIfNotSet<USMGraphK2Node_StateInstance_Begin>(BoundGraph, EntryNode);

	USMGraphK2Node_StateUpdateNode* UpdateNode = FSMBlueprintEditorUtils::GetFirstNodeOfClassNested<USMGraphK2Node_StateUpdateNode>(BoundGraph);
	FSMBlueprintEditorUtils::PlaceNodeIfNotSet<USMGraphK2Node_StateInstance_Update>(BoundGraph, UpdateNode);

	USMGraphK2Node_StateEndNode* EndNode = FSMBlueprintEditorUtils::GetFirstNodeOfClassNested<USMGraphK2Node_StateEndNode>(BoundGraph);
	FSMBlueprintEditorUtils::PlaceNodeIfNotSet<USMGraphK2Node_StateInstance_End>(BoundGraph, EndNode);

	// Optional nodes.
	FSMBlueprintEditorUtils::SetupDefaultPassthroughNodes<USMGraphK2Node_IntermediateStateMachineStartNode, USMGraphK2Node_StateInstance_StateMachineStart>(BoundGraph);
	FSMBlueprintEditorUtils::SetupDefaultPassthroughNodes<USMGraphK2Node_IntermediateStateMachineStopNode, USMGraphK2Node_StateInstance_StateMachineStop>(BoundGraph);

	FSMBlueprintEditorUtils::SetupDefaultPassthroughNodes<USMGraphK2Node_TransitionInitializedNode, USMGraphK2Node_StateInstance_OnStateInitialized>(BoundGraph);
	FSMBlueprintEditorUtils::SetupDefaultPassthroughNodes<USMGraphK2Node_TransitionShutdownNode, USMGraphK2Node_StateInstance_OnStateShutdown>(BoundGraph);
}

void USMGraphNode_StateNode::SetNodeClass(UClass* Class)
{
	StateClass = Class;
	Super::SetNodeClass(Class);
}

const FSlateBrush* USMGraphNode_StateNode::GetNodeIcon() const
{
	if (const FSlateBrush* Icon = Super::GetNodeIcon())
	{
		return Icon;
	}

	return FSMUnrealAppStyle::Get().GetBrush(TEXT("Graph.StateNode.Icon"));
}

void USMGraphNode_StateNode::InitTemplate()
{
	Super::InitTemplate();
}

bool USMGraphNode_StateNode::AreTemplatesFullyLoaded() const
{
	if (!Super::AreTemplatesFullyLoaded())
	{
		return false;
	}

	for (const FStateStackContainer& Stack : StateStack)
	{
		if (!Stack.NodeStackInstanceTemplate || Stack.NodeStackInstanceTemplate->HasAnyFlags(RF_NeedLoad | RF_NeedPostLoad))
		{
			return false;
		}
	}

	return true;
}

bool USMGraphNode_StateNode::DoesNodePossiblyHaveConstructionScripts() const
{
	if (Super::DoesNodePossiblyHaveConstructionScripts())
	{
		return true;
	}

	for (const FStateStackContainer& Stack : StateStack)
	{
		if (Stack.NodeStackInstanceTemplate && Stack.NodeStackInstanceTemplate->bHasEditorConstructionScripts)
		{
			return true;
		}
	}

	return false;
}

void USMGraphNode_StateNode::RunAllConstructionScripts_Internal()
{
	Super::RunAllConstructionScripts_Internal();

	if (!FUObjectThreadContext::Get().IsRoutingPostLoad)
	{
		// Use an iterator in case the stack is modified during construction.
		for (auto It = StateStack.CreateIterator(); It; ++It)
		{
			if (It->NodeStackInstanceTemplate)
			{
				It->NodeStackInstanceTemplate->RunConstructionScript();
			}
		}
	}
}

void USMGraphNode_StateNode::RestoreArchetypeValuesPriorToConstruction()
{
	Super::RestoreArchetypeValuesPriorToConstruction();
	for (const FStateStackContainer& Stack : StateStack)
	{
		if (Stack.NodeStackInstanceTemplate)
		{
			Stack.NodeStackInstanceTemplate->RestoreArchetypeValuesPriorToConstruction();
		}
	}
}

void USMGraphNode_StateNode::PreCompileNodeInstanceValidation(FCompilerResultsLog& CompilerContext,
                                                              USMCompilerLog* CompilerLog, USMGraphNode_Base* OwningNode)
{
	Super::PreCompileNodeInstanceValidation(CompilerContext, CompilerLog, OwningNode);

	const TArray<FStateStackContainer>& Templates = GetAllNodeStackTemplates();
	for (const FStateStackContainer& Template : Templates)
	{
		RunPreCompileValidateForNodeInstance(Template.NodeStackInstanceTemplate, CompilerLog);
	}
}

void USMGraphNode_StateNode::OnCompile(FSMKismetCompilerContext& CompilerContext)
{
	Super::OnCompile(CompilerContext);

	const TArray<FStateStackContainer>& Templates = GetAllNodeStackTemplates();

	if (Templates.Num() > 0)
	{
		FSMNode_Base* RuntimeNode = FSMBlueprintEditorUtils::GetRuntimeNodeFromGraph(BoundGraph);
		check(RuntimeNode);

		for (const FStateStackContainer& Template : Templates)
		{
			if (Template.NodeStackInstanceTemplate && GetDefaultNodeClass() != Template.StateStackClass)
			{
				CompilerContext.AddDefaultObjectTemplate(RuntimeNode->GetNodeGuid(), Template.NodeStackInstanceTemplate, FTemplateContainer::StackTemplate, Template.TemplateGuid);
			}
		}
	}
}

int32 USMGraphNode_StateNode::GetIndexOfTemplate(const FGuid& TemplateGuid) const
{
	for (int32 Idx = 0; Idx < StateStack.Num(); ++Idx)
	{
		if (StateStack[Idx].TemplateGuid == TemplateGuid)
		{
			return Idx;
		}
	}

	return INDEX_NONE;
}

void USMGraphNode_StateNode::GetAllNodeTemplates(TArray<USMNodeInstance*>& OutNodeInstances) const
{
	Super::GetAllNodeTemplates(OutNodeInstances);
	for (const FStateStackContainer& Stack : GetAllNodeStackTemplates())
	{
		OutNodeInstances.Add(Stack.NodeStackInstanceTemplate);
	}
}

USMNodeInstance* USMGraphNode_StateNode::AddStackNode(TSubclassOf<USMNodeInstance> NodeClass, int32 StackIndex)
{
	Modify();

	const UClass* DefaultNodeClass = GetDefaultNodeClass();
	if (!ensureMsgf(NodeClass && NodeClass->IsChildOf(DefaultNodeClass) && NodeClass != DefaultNodeClass, TEXT("AddStackNode only accepts a valid child of a State Instance Class.")))
	{
		return nullptr;
	}

	const FStateStackContainer NewStateStack(static_cast<TSubclassOf<USMStateInstance>>(NodeClass));

	const int32 IndexUsed = USMUtils::InsertOrAddToArray(StateStack, NewStateStack, StackIndex);
	check(IndexUsed >= 0);

	InitStateStack();
	CreateGraphPropertyGraphs();

	USMStateInstance* StackInstance = Cast<USMStateInstance>(
		GetTemplateFromIndex(IndexUsed));

	ensure(StackInstance);
	return StackInstance;
}

void USMGraphNode_StateNode::RemoveStackNode(int32 StackIndex)
{
	Modify();
	USMUtils::RemoveAtOrPopFromArray(StateStack, StackIndex);
}

void USMGraphNode_StateNode::ClearStackNodes()
{
	Modify();
	StateStack.Empty();
}

const TArray<FStateStackContainer>& USMGraphNode_StateNode::GetAllNodeStackTemplates() const
{
	return StateStack;
}

USMNodeInstance* USMGraphNode_StateNode::GetTemplateFromIndex(int32 Index) const
{
	if (Index >= 0 && Index < StateStack.Num())
	{
		return StateStack[Index].NodeStackInstanceTemplate;
	}

	return nullptr;
}

void USMGraphNode_StateNode::InitStateStack()
{
	for (FStateStackContainer& StateContainer : StateStack)
	{
		StateContainer.InitTemplate(this);
	}
}

void USMGraphNode_StateNode::DestroyStateStack()
{
	for (FStateStackContainer& StateContainer : StateStack)
	{
		StateContainer.DestroyTemplate();
	}

	StateStack.Reset();
}

#undef LOCTEXT_NAMESPACE
