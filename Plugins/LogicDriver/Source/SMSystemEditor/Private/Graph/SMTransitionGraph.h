// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "SMGraphK2.h"
#include "Nodes/RootNodes/SMGraphK2Node_TransitionResultNode.h"

#include "SMTransitionGraph.generated.h"

class USMGraphNode_TransitionEdge;

UCLASS()
class SMSYSTEMEDITOR_API USMTransitionGraph : public USMGraphK2
{
	GENERATED_UCLASS_BODY()

public:
	// USMGraphK2
	virtual bool HasAnyLogicConnections() const override;
	virtual FSMNode_Base* GetRuntimeNode() const override { return ResultNode ? ResultNode->GetRunTimeNode() : nullptr; }
	// ~USMGraphK2

	/** Determine if the graph should be evaluated at runtime or can be statically known. */
	ESMConditionalEvaluationType GetConditionalEvaluationType() const;

	/** If there is non-const logic which executes on a successful transition. */
	bool HasTransitionEnteredLogic() const;

	/** If this has the pre eval node and logic executing. */
	bool HasPreEvalLogic() const;

	/** If this has the post eval node and logic executing. */
	bool HasPostEvalLogic() const;

	/** If this has the initialize node and logic executing. */
	bool HasInitLogic() const;

	/** If this has the shut down node and logic executing. */
	bool HasShutdownLogic() const;

	template<typename T>
	bool HasNodeWithExecutionLogic() const;

	USMGraphNode_TransitionEdge* GetOwningTransitionNode() const;
	USMGraphNode_TransitionEdge* GetOwningTransitionNodeChecked() const;

	UPROPERTY()
	class USMGraphK2Node_TransitionResultNode* ResultNode;
};

