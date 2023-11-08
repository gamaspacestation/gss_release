// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "SMGraphNode_Base.h"
#include "NodeStack/NodeStackContainer.h"

#include "SMStateInstance.h"

#include "GameplayTagContainer.h"

#include "EdGraph/EdGraphNode.h"

#include "SMGraphNode_StateNode.generated.h"

class USMGraphNode_TransitionEdge;
class USMGraphNode_LinkStateNode;

/** Base class required as states and conduits branch separately from a common source. */
UCLASS()
class SMSYSTEMEDITOR_API USMGraphNode_StateNodeBase : public USMGraphNode_Base
{
	GENERATED_UCLASS_BODY()

	/**
	 * Add tags to this state that Any State nodes will recognize.
	 * This can allow control over which specific Any State nodes should impact this state.
	 *
	 * On the Any State you can define an AnyStateTagQuery to control which tags an Any State should recognize.
	 * Only valid in the editor.
	 */
	UPROPERTY(EditAnywhere, Category = "Any State")
	FGameplayTagContainer AnyStateTags;
	
	/**
	 * @deprecated Set on the node template instead.
	 */
	UPROPERTY()
	uint8 bAlwaysUpdate_DEPRECATED: 1;

	/**
	 * @deprecated Set on the node template instead.
	 */
	UPROPERTY()
	uint8 bDisableTickTransitionEvaluation_DEPRECATED: 1;

	/**
	 * @deprecated Set on the node template instead.
	 */
	UPROPERTY()
	uint8 bEvalTransitionsOnStart_DEPRECATED: 1;

	/**
	 * @deprecated Set on the node template instead.
	 */
	UPROPERTY()
	uint8 bExcludeFromAnyState_DEPRECATED: 1;
	
	/**
	 * Set by the editor and read by the schema to allow self transitions.
	 * We don't want to drag / drop self transitions because a single pin click will
	 * trigger them. There doesn't seem to be an ideal way for the schema to detect
	 * mouse movement to prevent single clicks when in CanCreateConnection,
	 * so we're relying on a context menu.
	 */
	uint8 bCanTransitionToSelf: 1;

public:
	// UEdGraphNode
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool GetCanRenameNode() const override;
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
	virtual void PostPlacedNewNode() override;
	virtual void PostPasteNode() override;
	virtual void DestroyNode() override;
	virtual TSharedPtr<INameValidatorInterface> MakeNameValidator() const override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
	// ~UEdGraphNode

	// USMGraphNode_Base
	virtual void ImportDeprecatedProperties() override;
	virtual void OnNodeMoved(const FVector2D& NewPosition) override;
	virtual void PreCompile(FSMKismetCompilerContext& CompilerContext) override;
	// ~USMGraphNode_Base
	
	/** Copy configuration settings to the runtime node. */
	virtual void SetRuntimeDefaults(FSMState_Base& State) const;
	virtual FString GetStateName() const { return GetNodeName(); }

	/**
	 * Checks if there are no outbound transitions.
	 * @param bCheckAnyState Checks if an Any State will prevent this from being an end state.
	 */
	virtual bool IsEndState(bool bCheckAnyState = true) const;

	/** Checks if there are any connections to this node. Does not count self. */
	virtual bool HasInputConnections() const;

	/** Checks if there are any connections from this node. */
	virtual bool HasOutputConnections() const;

	/** If transitions are supposed to run in parallel. */
	bool ShouldDefaultTransitionsToParallel() const;

	/** If this node shouldn't receive transitions from an Any State. */
	bool ShouldExcludeFromAnyState() const;
	
	/** Checks if there is a node connected via outgoing transition. */
	bool HasTransitionToNode(UEdGraphNode* Node) const;

	/** Checks if there is a node connected via incoming transition. */
	bool HasTransitionFromNode(UEdGraphNode* Node) const;

	/** Returns the previous node at the given input linked to index. */
	USMGraphNode_StateNodeBase* GetPreviousNode(int32 Index = 0, bool bIncludeReroute = false) const;

	/** Returns the next node at the given output linked to index. */
	USMGraphNode_StateNodeBase* GetNextNode(int32 Index = 0, bool bIncludeReroute = false) const;

	/** Returns a transition going to the input pin. */
	USMGraphNode_TransitionEdge* GetPreviousTransition(int32 Index = 0) const;

	/** Returns a transitions from the output pin. */
	USMGraphNode_TransitionEdge* GetNextTransition(int32 Index = 0) const;

	/** Return all transitions to the input pin. */
	void GetInputTransitions(TArray<USMGraphNode_TransitionEdge*>& OutTransitions) const;

	/** Return all transitions from the output pin. */
	void GetOutputTransitions(TArray<USMGraphNode_TransitionEdge*>& OutTransitions) const;

	/** Return the entry pin if this states is connected to an entry node, nullptr otherwise. */
	UEdGraphPin* GetConnectedEntryPin() const;
	
	FLinearColor GetBackgroundColorForNodeInstance(const USMNodeInstance* NodeInstance) const;

	/** Return any linked states pointing to this node. */
	const TSet<USMGraphNode_LinkStateNode*>& GetLinkedStates() const { return LinkedStates; }

protected:
	virtual FLinearColor Internal_GetBackgroundColor() const override;
	
private:
	friend class SGraphNode_StateNode;
	bool bRequestInitialAnimation;

	friend class USMGraphNode_LinkStateNode;

	/** States linked to this state. */
	UPROPERTY()
	TSet<USMGraphNode_LinkStateNode*> LinkedStates;
};

/**
 * Regular state nodes which have K2 graphs.
 */
UCLASS()
class SMSYSTEMEDITOR_API USMGraphNode_StateNode : public USMGraphNode_StateNodeBase
{
public:
	GENERATED_UCLASS_BODY()

	/** Select a custom node class to use for this node. This can be a blueprint or C++ class. */
	UPROPERTY(EditAnywhere, NoClear, Category = "State", meta = (BlueprintBaseOnly))
	TSubclassOf<USMStateInstance> StateClass;

	/** Augment the state by adding additional state classes to perform logic processing. */
	UPROPERTY(EditAnywhere, Category = "State")
	TArray<FStateStackContainer> StateStack;

	// UEdGraphNode
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual UObject* GetJumpTargetForDoubleClick() const override;
	virtual void DestroyNode() override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	//~ UEdGraphNode
	
	// USMGraphNode_Base
	virtual void PlaceDefaultInstanceNodes() override;
	virtual FName GetNodeClassPropertyName() const override { return GET_MEMBER_NAME_CHECKED(USMGraphNode_StateNode, StateClass); }
	virtual FName GetNodeStackPropertyName() override { return GET_MEMBER_NAME_CHECKED(USMGraphNode_StateNode, StateStack); }
	virtual FName GetNodeStackElementClassPropertyName() const override { return GET_MEMBER_NAME_CHECKED(FStateStackContainer, StateStackClass); }
	virtual UClass* GetNodeClass() const override { return StateClass; }
	virtual void SetNodeClass(UClass* Class) override;
	virtual bool SupportsPropertyGraphs() const override { return true; }
	virtual FName GetFriendlyNodeName() const override { return "State"; }
	virtual const FSlateBrush* GetNodeIcon() const override;
	virtual void InitTemplate() override;
	virtual bool AreTemplatesFullyLoaded() const override;
	virtual bool DoesNodePossiblyHaveConstructionScripts() const override;
	virtual void RunAllConstructionScripts_Internal() override;
	virtual void RestoreArchetypeValuesPriorToConstruction() override;
	virtual void PreCompileNodeInstanceValidation(FCompilerResultsLog& CompilerContext, USMCompilerLog* CompilerLog, USMGraphNode_Base* OwningNode) override;
	virtual void OnCompile(FSMKismetCompilerContext& CompilerContext) override;
	/**
	 * Retrieve the array index from the template guid.
	 *
	 * @return the array index or INDEX_NONE if not found.
	 */
	virtual int32 GetIndexOfTemplate(const FGuid& TemplateGuid) const override;

	virtual void GetAllNodeTemplates(TArray<USMNodeInstance*>& OutNodeInstances) const override;
	// ~USMGraphNode_Base

	// ISMEditorGraphNodeInterface
	virtual USMNodeInstance* AddStackNode(TSubclassOf<USMNodeInstance> NodeClass, int32 StackIndex) override;
	virtual void RemoveStackNode(int32 StackIndex) override;
	virtual void ClearStackNodes() override;
	// ~ISMEditorGraphNodeInterface

	/** Return all state stack templates. */
	const TArray<FStateStackContainer>& GetAllNodeStackTemplates() const;

	/**
	 * Retrieve the template instance from an index.
	 *
	 * @return the NodeInstance template.
	 */
	virtual USMNodeInstance* GetTemplateFromIndex(int32 Index) const override;
	
	void InitStateStack();
	void DestroyStateStack();
};
