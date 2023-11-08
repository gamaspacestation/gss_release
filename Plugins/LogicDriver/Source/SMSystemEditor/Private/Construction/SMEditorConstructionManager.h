// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "SMStateMachine.h"
#include "Blueprints/SMBlueprint.h"

#include "TickableEditorObject.h"

class FSMBlueprintEditor;
class USMEditorInstance;
class USMGraph;
struct FSMNode_Base;

struct FSMEditorStateMachine
{
	/** The sm instance used during editor time. */
	USMEditorInstance* StateMachineEditorInstance;
	
	/** Storage for all editor runtime nodes. This memory is manually managed! */
	TArray<FSMNode_Base*> EditorInstanceNodeStorage;

	/** Created runtime nodes mapped to their graph node. */
	TMap<FSMNode_Base*, UEdGraphNode*> RuntimeNodeToGraphNode;
};

struct FSMConstructionConfiguration
{
	/** Construction scripts will not run if the blueprint is being compiled. */
	bool bSkipOnCompile = true;

	/** Requires the construction script refresh the slate node completely. */
	bool bFullRefreshNeeded = true;

	/** Signal not to the dirty the asset. This is ignored if the BP has structural modifications. */
	bool bDoNotDirty = false;

	/** If this is being triggered from a load. */
	bool bFromLoad = false;
};

/** Configuration options for conditionally compiling. */
struct FSMConditionalCompileConfiguration
{
	FSMConditionalCompileConfiguration() = default;

	/** Calls EnsureCachedDependenciesUpToDate. */
	bool bUpdateDependencies = true;

	/** Calls ForceRecreateProperties on all nodes. */
	bool bRecreateGraphProperties = false;

	/** If the BP should compile this tick. */
	bool bCompileNow = false;
};

/**
 * Construction manager singleton for running construction scripts and building editor state machines.
 */
class SMSYSTEMEDITOR_API FSMEditorConstructionManager : public FTickableEditorObject
{
public:
	FSMEditorConstructionManager(FSMEditorConstructionManager const&) = delete;
	void operator=(FSMEditorConstructionManager const&) = delete;

	/** Cancels RunAllConstructionScriptsForBlueprint if true. */
	bool bDisableConstructionScripts = false;
private:
	FSMEditorConstructionManager() {}

public:
	virtual ~FSMEditorConstructionManager() override;

	/** Access the Construction Manager. */
	static FSMEditorConstructionManager* GetInstance();

	// FTickableEditorObject
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
	virtual TStatId GetStatId() const override;
	// ~FTickableEditorObject

	/** Checks if there are construction scripts for this frame. */
	bool HasPendingConstructionScripts() const;

	/**
	 * True if construction scripts are currently running for this frame.
	 * 
	 * @param ForBlueprint If construction scripts are running for the given blueprint. Null implies any.
	 * @return True if construction scripts are in progress.
	 */
	bool IsRunningConstructionScripts(USMBlueprint* ForBlueprint = nullptr) const;
	
	/**
	 * Frees all associated memory and resets the editor state machine map.
	 */
	void CleanupAllEditorStateMachines();

	/**
	 *  Shutdown the editor instance and free node memory.
	 *
	 *  @param InBlueprint for the editor state machine.
	 */
	void CleanupEditorStateMachine(USMBlueprint* InBlueprint);

	/**
	 * Runs all construction scripts for every node in a blueprint. This is executed on this frame
	 * and even during a compile.
	 *
	 * @param InBlueprint The blueprint to run all construction scripts for.
	 * @param bCleanupEditorStateMachine If the editor state machine should be cleaned up afterward. If this is false
	 * then CleanupEditorStateMachine must be called manually.
	 */
	void RunAllConstructionScriptsForBlueprintImmediately(USMBlueprint* InBlueprint, bool bCleanupEditorStateMachine = true);

	/**
	 * Runs all construction scripts for every node in a blueprint. This is executed on the next frame.
	 *
	 * @param InObject The exact blueprint or the object belonging to the blueprint to run all construction scripts for.
	 * @param InConfiguration Provided configuration for the construction run.
	 */
	void RunAllConstructionScriptsForBlueprint(UObject* InObject, const FSMConstructionConfiguration& InConfiguration = FSMConstructionConfiguration());

	/**
	 * Signal a blueprint should run the conditional compile operation next tick. This won't occur if the blueprint is running construction scripts.
	 *
	 * @param InBlueprint The state machine blueprint to run conditional compile on.
	 * @param InConfiguration The configuration of the conditional compile.
	 */
	void QueueBlueprintForConditionalCompile(USMBlueprint* InBlueprint, const FSMConditionalCompileConfiguration& InConfiguration = FSMConditionalCompileConfiguration());

	/**
	 * Create or update a state machine for editor use.
	 *
	 * @param InBlueprint The blueprint owning the state machine.
	 * 
	 * @return The editor state machine created.
	 */
	FSMEditorStateMachine& CreateEditorStateMachine(USMBlueprint* InBlueprint);

	/**
	 * Retrieve an existing editor state machine if one exists.
	 *
	 * @param InBlueprint The blueprint owning the state machine.
	 * @param OutEditorStateMachine The existing editor state machine if one exists. This may be invalidated
	 * if another editor state machine is added after retrieving this one.
	 *
	 * @return True if the state machine was found, false if it doesn't exist.
	 */
	bool TryGetEditorStateMachine(USMBlueprint* InBlueprint, FSMEditorStateMachine& OutEditorStateMachine);

	/**
	 * Allow construction scripts to run on load.
	 *
	 * @param bAllow If true construction scripts may run on load, otherwise they will be skipped.
	 */
	void SetAllowConstructionScriptsOnLoad(bool bAllow)
	{
		bAllowConstructionScriptsOnLoad = bAllow;
	}

	/**
	 * @return True if construction scripts are allowed to run on load.
	 */
	bool AreConstructionScriptsAllowedOnLoad() const;

	/**
	 *  Signal that a blueprint should or shouldn't run its construction scripts when it is loaded.
	 *  This will stay in effect until removed. This setting is overruled by SetAllowConstructionScriptsOnLoad().
	 *
	 *  @param InPath The full path of the blueprint.
	 *  @param bValue True the blueprint is allowed to run construction scripts on load, false it is not.
	 */
	void SetAllowConstructionScriptsOnLoadForBlueprint(const FString& InPath, bool bValue);

protected:
	/**
	 * Recursively build out a state machine from an editor graph. This is executed this frame.
	 *
	 * @param InGraph A state machine editor graph.
	 * @param StateMachineOut The outgoing state machine being assembled. This should be the root.
	 * @param EditorStateMachineInOut Heap memory will be initialized here. This memory MUST be freed manually to prevent a memory leak.
	 */
	void ConstructEditorStateMachine(USMGraph* InGraph, FSMStateMachine& StateMachineOut, FSMEditorStateMachine& EditorStateMachineInOut);
	
	/** Configure the initial root FSM for a state machine blueprint. */
	void SetupRootStateMachine(FSMStateMachine& StateMachineInOut, const USMBlueprint* InBlueprint) const;

	/** If the given blueprint qualifies for conditional compile. */
	bool CanConditionallyCompileBlueprint(USMBlueprint* InBlueprint) const;

	/**
	 * Assemble editor state machines and run construction scripts this frame.
	 *
	 * @param InBlueprint The blueprint to run all construction scripts for.
	 * @param InConfigurationData Configuration data to apply.
	 *
	 * @return True if construction scripts ran.
	 */
	bool RunAllConstructionScriptsForBlueprint_Internal(USMBlueprint* InBlueprint, const FSMConstructionConfiguration& InConfigurationData);

	/**
	 * Conditionally compile the blueprint this frame if possible.
	 *
	 * @param InBlueprint The blueprint to run all construction scripts for.
	 * @param InConfiguration Configuration data to apply.
	 */
	void ConditionalCompileBlueprint_Internal(USMBlueprint* InBlueprint, const FSMConditionalCompileConfiguration& InConfiguration);

private:
	/** Loaded blueprints mapped to their editor state machine. */
	TMap<TWeakObjectPtr<USMBlueprint>, FSMEditorStateMachine> EditorStateMachines;

	/** All blueprints waiting to have their construction scripts run. */
	TMap<TWeakObjectPtr<USMBlueprint>, FSMConstructionConfiguration> BlueprintsPendingConstruction;

	/** All blueprints in process of being constructed for a frame. */
	TSet<TWeakObjectPtr<USMBlueprint>> BlueprintsBeingConstructed;

	/** Blueprints which should run a conditional compile. */
	TMap<TWeakObjectPtr<USMBlueprint>, FSMConditionalCompileConfiguration> BlueprintsToConditionallyCompile;

	/** The path of blueprints which shouldn't have their construction scripts run on load. */
	TSet<FString> BlueprintsToSkipConstructionScriptsOnLoad;

	/** Disable or enable construction scripts to run if triggered during load. */
	bool bAllowConstructionScriptsOnLoad = true;
};

/**
 * Disable editor construction scripts only within the scope of this struct.
 */
struct SMSYSTEMEDITOR_API FSMDisableConstructionScriptsOnScope
{
	bool bOriginallyEnabled;
	FSMDisableConstructionScriptsOnScope() : bOriginallyEnabled(FSMEditorConstructionManager::GetInstance()->bDisableConstructionScripts)
	{
		FSMEditorConstructionManager::GetInstance()->bDisableConstructionScripts = true;
	}

	~FSMDisableConstructionScriptsOnScope()
	{
		Cancel();
	}

	void Cancel()
	{
		FSMEditorConstructionManager::GetInstance()->bDisableConstructionScripts = bOriginallyEnabled;
	}
};