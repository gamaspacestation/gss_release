// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "EdGraph/EdGraphSchema.h"
#include "Templates/SubclassOf.h"

#include "SMGraphSchema.generated.h"

class USMGraphNode_StateNode;
class USMGraphNode_TransitionEdge;
class USMTransitionInstance;

/** Action to add a node to the graph */
USTRUCT()
struct SMSYSTEMEDITOR_API FSMGraphSchemaAction_NewNode : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY()

public:
	FSMGraphSchemaAction_NewNode(): OwnerOfTemporaries(nullptr), GraphNodeTemplate(nullptr), NodeClass(nullptr),
	                                bDontOverrideDefaultClass(false),
	                                bDontCallPostPlacedNode(false)
	{
	}

	FSMGraphSchemaAction_NewNode(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), OwnerOfTemporaries(nullptr),
		  GraphNodeTemplate(nullptr),
		  NodeClass(nullptr), bDontOverrideDefaultClass(false), bDontCallPostPlacedNode(false)
	{
	}

	// FEdGraphSchemaAction
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// ~FEdGraphSchemaAction

	/** Only used if the TransientPackage is provided as the outer. */
	UEdGraph* OwnerOfTemporaries;

	/** The UEdGraphNode to be spawned. */
	UEdGraphNode* GraphNodeTemplate;

	UClass* NodeClass;
	bool bDontOverrideDefaultClass;
	bool bDontCallPostPlacedNode;
};

/** Action to reference a state machine */
USTRUCT()
struct SMSYSTEMEDITOR_API FSMGraphSchemaAction_NewStateMachineReferenceNode : public FSMGraphSchemaAction_NewNode
{
	GENERATED_USTRUCT_BODY()

public:
	FSMGraphSchemaAction_NewStateMachineReferenceNode()
	{
		bDontOverrideDefaultClass = true;	
	}

	FSMGraphSchemaAction_NewStateMachineReferenceNode(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, const int32 InGrouping)
		: FSMGraphSchemaAction_NewNode(InNodeCategory, InMenuDesc, InToolTip, InGrouping)
	{
		bDontOverrideDefaultClass = true;
	}

	// FEdGraphSchemaAction
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	// ~FEdGraphSchemaAction
};

/** Action to create new comment */
USTRUCT()
struct SMSYSTEMEDITOR_API FSMGraphSchemaAction_NewComment : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	FSMGraphSchemaAction_NewComment()
		: FEdGraphSchemaAction()
	{}

	FSMGraphSchemaAction_NewComment(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
	{}

	// FEdGraphSchemaAction
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	// ~FEdGraphSchemaAction
};

UCLASS()
class SMSYSTEMEDITOR_API USMGraphSchema : public UEdGraphSchema
{
	GENERATED_UCLASS_BODY()

public:
	// UEdGraphSchema
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;
	virtual EGraphType GetGraphType(const UEdGraph* TestEdGraph) const override;
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	virtual void GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const override;
	virtual bool TryCreateConnection(UEdGraphPin* PinA, UEdGraphPin* PinB) const override;
	virtual bool CreateAutomaticConversionNodeAndConnections(UEdGraphPin* A, UEdGraphPin* B) const override;
	virtual FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const override;
	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
	virtual void GetGraphDisplayInformation(const UEdGraph& Graph, /*out*/ FGraphDisplayInfo& DisplayInfo) const override;

	virtual void BreakNodeLinks(UEdGraphNode& TargetNode) const override;
	virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotification) const override;
	virtual void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const override;

	virtual bool SupportsDropPinOnNode(UEdGraphNode* InTargetNode, const FEdGraphPinType& InSourcePinType, EEdGraphPinDirection InSourcePinDirection, FText& OutErrorMessage) const override;
	virtual bool CanDuplicateGraph(UEdGraph* InSourceGraph) const override { return false; }
	virtual void HandleGraphBeingDeleted(UEdGraph& GraphBeingRemoved) const override;
	virtual void OnPinConnectionDoubleCicked(UEdGraphPin* PinA, UEdGraphPin* PinB, const FVector2D& GraphPosition) const override;
	// ~UEdGraphSchema

	static bool DoesUserAllowPlacement(const UEdGraphNode* A, const UEdGraphNode* B, FPinConnectionResponse& ResponseOut);

	static bool CanReplaceNode(const UEdGraphNode* InGraphNode);
	static bool CanReplaceNodeWith(const UEdGraphNode* InGraphNode, bool& bStateMachine, bool& bStateMachineRef, bool& bState, bool& bConduit, bool& bStateMachineParent);

	/** Determine the transition class to use when creating a new connection based on user rules. */
	static bool SetTransitionClassFromRules(USMGraphNode_TransitionEdge* InTransitionEdge, const UClass* InFromStateClass,
		const UClass* InToStateClass, const UClass* InStateMachineClass, TSubclassOf<USMTransitionInstance> InBaseClass = nullptr);

protected:
	void GetReplaceWithMenuActions(class FMenuBuilder& MenuBuilder, const UEdGraphNode* InGraphNode) const;
};

