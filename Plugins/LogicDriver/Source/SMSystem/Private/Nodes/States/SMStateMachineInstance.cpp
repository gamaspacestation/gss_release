// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "SMStateMachineInstance.h"
#include "SMStateMachine.h"

USMStateMachineInstance::USMStateMachineInstance() : Super(), bReuseCurrentState(false), bReuseIfNotEndState(false)
{
}

void USMStateMachineInstance::GetAllStateInstances(TArray<USMStateInstance_Base*>& StateInstances) const
{
	StateInstances.Reset();
	
	if (FSMStateMachine* StateMachineOwner = (FSMStateMachine*)GetOwningNode())
	{
		const TArray<FSMState_Base*>& States = StateMachineOwner->GetStates();
		StateInstances.Reserve(States.Num());
		
		for (FSMState_Base* State : States)
		{
			if (USMStateInstance_Base* StateInstance = Cast<USMStateInstance_Base>(State->GetOrCreateNodeInstance()))
			{
				StateInstances.Add(StateInstance);
			}
		}
	}
}

USMStateInstance_Base* USMStateMachineInstance::GetContainedStateByName(const FString& StateName) const
{
	if (FSMStateMachine* StateMachineOwner = (FSMStateMachine*)GetOwningNode())
	{
		const TMap<FString, FSMState_Base*>& StateStructMap = StateMachineOwner->GetStateNameMap();
		if (FSMState_Base* const* StateBase = StateStructMap.Find(StateName))
		{
			return Cast<USMStateInstance_Base>((*StateBase)->GetOrCreateNodeInstance());
		}
	}

	return nullptr;
}

void USMStateMachineInstance::GetEntryStates(TArray<USMStateInstance_Base*>& EntryStates) const
{
	EntryStates.Reset();
	
	if (FSMStateMachine* StateMachineOwner = (FSMStateMachine*)GetOwningNode())
	{
		const TArray<FSMState_Base*>& OriginalEntryStates = StateMachineOwner->GetEntryStates();
		EntryStates.Reserve(OriginalEntryStates.Num());
		
		for (FSMState_Base* EntryState : OriginalEntryStates)
		{
			if (USMStateInstance_Base* NodeInstance = Cast<USMStateInstance_Base>(EntryState->GetOrCreateNodeInstance()))
			{
				EntryStates.Add(NodeInstance);
			}
		}
	}
}

void USMStateMachineInstance::GetActiveStates(TArray<USMStateInstance_Base*>& ActiveStates) const
{
	ActiveStates.Reset();
	
	if (FSMStateMachine* StateMachineOwner = (FSMStateMachine*)GetOwningNode())
	{
		const TArray<FSMState_Base*> OriginalActiveStates = StateMachineOwner->GetActiveStates();
		ActiveStates.Reserve(OriginalActiveStates.Num());
		
		for (FSMState_Base* ActiveState : OriginalActiveStates)
		{
			if (USMStateInstance_Base* NodeInstance = Cast<USMStateInstance_Base>(ActiveState->GetOrCreateNodeInstance()))
			{
				ActiveStates.Add(NodeInstance);
			}
		}
	}
}

USMInstance* USMStateMachineInstance::GetStateMachineReference() const
{
	if (const FSMStateMachine* StateMachineOwner = (FSMStateMachine*)GetOwningNodeContainer())
	{
		return StateMachineOwner->GetInstanceReference();
	}

	return nullptr;
}

const FSMNode_Base* USMStateMachineInstance::GetOwningNodeContainer() const
{
	if (FSMStateMachine* StateMachineOwner = (FSMStateMachine*)GetOwningNode())
	{
		if (FSMStateMachine* ReferencedBy = StateMachineOwner->GetReferencedByStateMachine())
		{
			// Return the real node that references our owning node.
			return ReferencedBy;
		}
	}

	return Super::GetOwningNodeContainer();
}

bool USMStateMachineInstance::GetWaitForEndState() const
{
	if (FSMStateMachine* StateMachineOwner = (FSMStateMachine*)GetOwningNodeContainer())
	{
		// Wait for end state only recognized on the owning container node for references.
		return StateMachineOwner->bWaitForEndState;
	}

	return bWaitForEndState;
}

void USMStateMachineInstance::SetWaitForEndState(const bool bValue)
{
	bWaitForEndState = bValue;
	if (FSMStateMachine* StateMachineOwner = (FSMStateMachine*)GetOwningNodeContainer())
	{
		// Wait for end state only recognized on the owning container node for references.
		StateMachineOwner->bWaitForEndState = bWaitForEndState;
	}
}

bool USMStateMachineInstance::GetReuseCurrentState() const
{
	if (FSMStateMachine* StateMachineOwner = (FSMStateMachine*)GetOwningNode())
	{
		return StateMachineOwner->bReuseCurrentState;
	}

	return bReuseCurrentState;
}

void USMStateMachineInstance::SetReuseCurrentState(const bool bValue)
{
	bReuseCurrentState = bValue;
	if (FSMStateMachine* StateMachineOwner = (FSMStateMachine*)GetOwningNode())
	{
		StateMachineOwner->SetReuseCurrentState(bReuseCurrentState, GetReuseIfNotEndState());
	}
}

bool USMStateMachineInstance::GetReuseIfNotEndState() const
{
	if (FSMStateMachine* StateMachineOwner = (FSMStateMachine*)GetOwningNode())
	{
		return StateMachineOwner->bOnlyReuseIfNotEndState;
	}

	return bReuseIfNotEndState;
}

void USMStateMachineInstance::SetReuseIfNotEndState(const bool bValue)
{
	bReuseIfNotEndState = bValue;
	if (FSMStateMachine* StateMachineOwner = (FSMStateMachine*)GetOwningNode())
	{
		StateMachineOwner->SetReuseCurrentState(GetReuseCurrentState(), bReuseIfNotEndState);
	}
}
