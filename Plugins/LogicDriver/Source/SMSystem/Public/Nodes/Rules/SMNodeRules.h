// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SMNodeRules.generated.h"


USTRUCT()
struct SMSYSTEM_API FSMNodeClassRule
{
	GENERATED_USTRUCT_BODY()

	FSMNodeClassRule();
	virtual ~FSMNodeClassRule() = default;
	virtual UClass* GetClass() const
	{
		return nullptr;
	}

	/** Checks if a class is a base node class. Considers null a base class. */
	static bool IsBaseClass(const UClass* Class);
	
	/** If all children of this class should be considered. */
	UPROPERTY(EditDefaultsOnly, AdvancedDisplay, Category = "Rule", meta = (NoResetToDefault))
	bool bIncludeChildren;

	/** Invert the rule. */
	UPROPERTY(EditDefaultsOnly, AdvancedDisplay, Category = "Rule", meta = (NoResetToDefault))
	bool bNOT;
};

USTRUCT()
struct SMSYSTEM_API FSMStateClassRule : public FSMNodeClassRule
{
	GENERATED_USTRUCT_BODY()

	virtual UClass* GetClass() const override;
	
	/** The state class to look for. */
	UPROPERTY(EditDefaultsOnly, Category = "Rule", meta = (NoResetToDefault, AllowAbstract))
	TSoftClassPtr<class USMStateInstance_Base> StateClass;
};

USTRUCT()
struct SMSYSTEM_API FSMStateMachineClassRule : public FSMNodeClassRule
{
	GENERATED_USTRUCT_BODY()

	virtual UClass* GetClass() const override;
	
	/** The state machine class to look for. */
	UPROPERTY(EditDefaultsOnly, Category = "Rule", meta = (NoResetToDefault, AllowAbstract))
	TSoftClassPtr<class USMStateMachineInstance> StateMachineClass;
};

USTRUCT()
struct SMSYSTEM_API FSMNodeConnectionRule
{
	GENERATED_USTRUCT_BODY()

	FSMNodeConnectionRule();

	/** The start of a connection. */
	UPROPERTY(EditDefaultsOnly, Category = "Rule", meta = (InstancedTemplate))
	FSMStateClassRule FromState;

	/** The end of a connection. */
	UPROPERTY(EditDefaultsOnly, Category = "Rule", meta = (InstancedTemplate))
	FSMStateClassRule ToState;

	/** The state machine this connection exists in. */
	UPROPERTY(EditDefaultsOnly, Category = "Rule", meta = (InstancedTemplate))
	FSMStateMachineClassRule InStateMachine;

	static bool DoesClassMatch(const UClass* ExpectedClass, const UClass* ActualClass, const FSMNodeClassRule& Rule);

	template<typename T>
	static bool DoRulesPass(UClass* Class, const TArray<T>& Rules)
	{
		if (Rules.Num() == 0)
		{
			return true;
		}

		bool bCheckingInverse = false;
		bool bAllInversedPassed = true;
		for (const FSMNodeClassRule& Rule : Rules)
		{
			if (Rule.bNOT)
			{
				bCheckingInverse = true;
			}
			if (DoesClassMatch(Rule.GetClass(), Class, Rule))
			{
				// Only one regular rules needs to pass.
				if (!Rule.bNOT)
				{
					return true;
				}
			}
			else if (Rule.bNOT)
			{
				// There was more than one NOT. They all need to pass.
				bAllInversedPassed = false;
			}
		}

		return bCheckingInverse && bAllInversedPassed;
	}
};

USTRUCT()
struct SMSYSTEM_API FSMConnectionValidator
{
	GENERATED_USTRUCT_BODY()

};

/**
 * Describe under what conditions transitions should be allowed.
 */
USTRUCT()
struct SMSYSTEM_API FSMTransitionConnectionValidator : public FSMConnectionValidator
{
	GENERATED_USTRUCT_BODY()

	/**
	 * If any connection rules are present at least one must be valid for this connection be allowed.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Behavior", meta = (InstancedTemplate))
	TArray<FSMNodeConnectionRule> AllowedConnections;
	
	/** Checks if this class has rules and if any of them apply. */
	bool IsConnectionValid(const UClass* FromClass, const UClass* ToClass, const UClass* StateMachineClass, bool bPassOnNoRules = true) const;
};

/**
 * Describe under what conditions nodes are allowed to be connected.
 */
USTRUCT()
struct SMSYSTEM_API FSMStateConnectionValidator : public FSMConnectionValidator
{
	GENERATED_USTRUCT_BODY()

	/**
	 * States that can connect to this state.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Behavior", meta = (InstancedTemplate))
	TArray<FSMStateClassRule> AllowedInboundStates;

	/**
	 * States that this state can connect to.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Behavior", meta = (InstancedTemplate))
	TArray<FSMStateClassRule> AllowedOutboundStates;
	
	/**
	* State machines that this state can be placed in.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Behavior", meta = (InstancedTemplate))
	TArray<FSMStateMachineClassRule> AllowedInStateMachines;

	/** Checks if this class has rules and if any of them apply. */
	bool IsInboundConnectionValid(UClass* FromClass, UClass* StateMachineClass) const;
	
	/** Checks if this class has rules and if any of them apply. */
	bool IsOutboundConnectionValid(UClass* ToClass, UClass* StateMachineClass) const;
};


/**
 * Describe under what conditions nodes are allowed to be placed.
 */
USTRUCT()
struct SMSYSTEM_API FSMStateMachineNodePlacementValidator : public FSMConnectionValidator
{
	GENERATED_USTRUCT_BODY()

	FSMStateMachineNodePlacementValidator();
	
	/**
	 * States that can be placed in this state machine. This restricts which items show up on the graph context menu. None implies all.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Behavior", meta = (InstancedTemplate))
	TArray<FSMStateClassRule> AllowedStates;

	/**
	 * Restricts the placement of state machine references within this state machine.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Behavior", meta = (InstancedTemplate))
	bool bAllowReferences;

	/**
	 * Restricts the placement of state machine parents within this state machine.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Behavior", meta = (InstancedTemplate))
	bool bAllowParents;

	/**
	 * Allow sub state machines to be added or collapsed.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Behavior", meta = (InstancedTemplate))
	bool bAllowSubStateMachines;

	/**
	 * The default state machine class to assign when adding or collapsing a state machine.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Behavior", meta = (InstancedTemplate, EditCondition = "bAllowSubStateMachines"))
	TSoftClassPtr<class USMStateMachineInstance> DefaultSubStateMachineClass;

	/** Checks if this state can be placed in this state machine. */
	bool IsStateAllowed(UClass* StateClass) const;
};

