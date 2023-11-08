// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "ISMEditorGraphNodeInterface.h"
#include "PropertyNodes/SMGraphK2Node_PropertyNode.h"

#include "SMNodeInstance.h"

#include "EdGraph/EdGraphNode.h"
#include "Styling/SlateBrush.h"

#include "SMGraphNode_Base.generated.h"

class USMEditorSettings;
class USMGraph;
class FSMKismetCompilerContext;
struct FSMNode_Base;

#define INDEX_PIN_INPUT 0
#define INDEX_PIN_OUTPUT 1

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGraphNodeRefreshRequested, class USMGraphNode_Base* /* GraphNode */, bool /* bFullRefresh */);

USTRUCT()
struct FSMGraphNodeLog
{
	GENERATED_BODY()

	FSMGraphNodeLog(): LogType(0)
	{
	}

	FSMGraphNodeLog(int32 Type)
	{
		LogType = Type;
	}

	UPROPERTY()
	FString ConsoleMessage;

	UPROPERTY()
	FString NodeMessage;

	/** EMessageSeverity::Type */
	UPROPERTY()
	int32 LogType;

	/** Objects like nodes or pins to go to the log. */
	UPROPERTY()
	TArray<UObject*> ReferenceList;
};

UCLASS(abstract, config = EditorPerProjectUserSettings, PerObjectConfig)
class SMSYSTEMEDITOR_API USMGraphNode_Base : public UEdGraphNode, public ISMEditorGraphNodeInterface
{
	GENERATED_UCLASS_BODY()

public:
	bool bGenerateTemplateOnNodePlacement;

	// UObject
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
	// ~UObject

	virtual ~USMGraphNode_Base() override;

	// UEdGraphNode
	virtual void DestroyNode() override;
	virtual void PostPasteNode() override;
	virtual void PostEditUndo() override;
	virtual void PostPlacedNewNode() override;
	virtual void OnRenameNode(const FString& NewName) override;
	virtual UObject* GetJumpTargetForDoubleClick() const override;
	virtual bool CanJumpToDefinition() const override;
	virtual void JumpToDefinition() const override;
	virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const override;
	virtual void ReconstructNode() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual void ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const override;
	// ~UEdGraphNode

	// ISMEditorGraphNodeInterface
	virtual TScriptInterface<ISMEditorGraphPropertyNodeInterface> GetEditorGraphProperty(FName PropertyName, const USMNodeInstance* NodeInstance, int32 ArrayIndex) const override;
	virtual TArray<TScriptInterface<ISMEditorGraphPropertyNodeInterface>> GetEditorGraphPropertyAsArray(FName PropertyName, const USMNodeInstance* NodeInstance, int32 ArrayIndex) const override;
	virtual TArray<TScriptInterface<ISMEditorGraphPropertyNodeInterface>> GetAllEditorGraphProperties(const USMNodeInstance* NodeInstance) const override;
	virtual USMNodeInstance* AddStackNode(TSubclassOf<USMNodeInstance> NodeClass, int32 StackIndex) override { ensureMsgf(false, TEXT("AddStackNode is only supported on State nodes.")); return nullptr; }
	virtual void RemoveStackNode(int32 StackIndex) override { ensureMsgf(false, TEXT("RemoveStackNode is only supported on State nodes.")); }
	virtual void ClearStackNodes() override { ensureMsgf(false, TEXT("ClearStackNodes is only supported on State nodes.")); }
	// ~ISMEditorGraphNodeInterface
	
	/** Called during kismet pre-compile before the bound graph is copied to the consolidated event graph. */
	virtual void PreCompile(FSMKismetCompilerContext& CompilerContext);

	/** Called during kismet pre-compile specifically for node instances to run custom validation. */
	virtual void PreCompileNodeInstanceValidation(FCompilerResultsLog& CompilerContext, USMCompilerLog* CompilerLog, USMGraphNode_Base* OwningNode = nullptr);

	/** Trigger the protected OnPreCompileValidate method for a node instance. */
	static void RunPreCompileValidateForNodeInstance(const USMNodeInstance* InNodeInstance, USMCompilerLog* InCompilerLog);
	
	/** Called during kismet compile after this node has been cloned. */
	virtual void OnCompile(FSMKismetCompilerContext& CompilerContext);

	/** Resets the local and node debug state if valid. */
	virtual void ResetDebugState();
	
	/** Called from the slate node when it is constructed. */
	virtual void OnWidgetConstruct();
	
	/** So we can pass time ticks for specific node appearance behavior. */
	virtual void UpdateTime(float DeltaTime);

	/** Helper to set error messages that may happen before compile. */
	virtual void CheckSetErrorMessages() {}

	/** Resets collected logs. */
	virtual void ResetLogMessages();
	
	/** Calculate any error / info display. */
	void UpdateErrorMessageFromLogs();

	/** Adds a log message to the collected logs. */
	void AddNodeLogMessage(const FSMGraphNodeLog& Message);
	
	/** Combine all logs into a single message and retrieve the severity. Returns false if no messages exist. */
	bool TryGetNodeLogMessage(FString& OutMessage, int32& OutSeverity) const;

	/** Called by slate when the node is moved. */
	virtual void OnNodeMoved(const FVector2D& NewPosition);
	
	/** Sets the read only position to the current node position. */
	void SetReadOnlyNodePosition();

	/** Jump to the local graph. */
	virtual void GoToLocalGraph() const;

	/** If this node is allowed to go to it's local graph. */
	virtual bool CanGoToLocalGraph() const { return GetBoundGraph() != nullptr; }

	/** Return the node name from the bound graph. */
	virtual FString GetNodeName() const;

	/** Set the name of the node, typically renaming the graph. */
	virtual void SetNodeName(const FString& InNewName);

	/** Return the correct guid, either from a runtime node or the graph node guid. */
	virtual const FGuid& GetCorrectNodeGuid(bool* bIsRunTimeGuid = nullptr) const;

private:
	/** Set the read only position to a manual position. */
	void SetReadOnlyNodePosition(const FVector2D& Position);

public:
	// Node class settings.
	/** Instantiate a template for use as an archetype. */
	virtual void InitTemplate();

	/** Transfer the template to the transient package. */
	virtual void DestroyTemplate();
	
	/** Runs all template construction scripts.*/
	void RunAllConstructionScripts();

	/** If this node can ever run construction scrips. */
	virtual bool CanRunConstructionScripts() const { return true; }
	
	/** Check if this node might have construction scripts. */
	virtual bool DoesNodePossiblyHaveConstructionScripts() const;
	
	/** True only while construction scripts are running. */
	bool IsRunningConstructionScripts() const { return bIsRunningConstructionScripts; }
	
protected:
	/** Destroys all graph property graphs and empty all containers. */
	void DestroyAllPropertyGraphs();
	
	/** Place default nodes when a class is selected. */
	virtual void PlaceDefaultInstanceNodes();

	/** Runs all template construction scripts core behavior. */
	virtual void RunAllConstructionScripts_Internal();

	/** Restore specific construction scripts values. */
	virtual void RestoreArchetypeValuesPriorToConstruction();
	
	bool IsSafeToConditionallyCompile(EPropertyChangeType::Type ChangeType) const;

protected:
	bool bPostEditChangeConstructionRequiresFullRefresh;
	
private:
	bool bIsRunningConstructionScripts;
	
public:
	/** Return the correct node class property name for the node type. */
	virtual FName GetNodeClassPropertyName() const { return NAME_None; }

	/** Return the correct node class. This should be a TSubClass property in child nodes. */
	virtual UClass* GetNodeClass() const { return nullptr; }
	virtual void SetNodeClass(UClass* Class);
	UClass* GetDefaultNodeClass() const;
	
	/** Checks if the node template is user created or system supplied. System supplied templates don't get stored on the CDO. */
	bool IsUsingDefaultNodeClass() const { return GetNodeClass() == GetDefaultNodeClass(); }
	
	/** Checks if the node class is native only. */
	bool IsNodeClassNative() const;
	
	/** Checks if all execution points avoid blueprint graph calls. */
	virtual bool IsNodeFastPathEnabled() const;

private:
	mutable TOptional<bool> bFastPathEnabledCached;
	
public:
	virtual FName GetNodeTemplatePropertyName() { return GET_MEMBER_NAME_CHECKED(USMGraphNode_Base, NodeInstanceTemplate); }
	virtual FName GetNodeStackPropertyName() { return NAME_None; }
	virtual FName GetNodeStackElementClassPropertyName() const { return NAME_None; }
	USMNodeInstance* GetNodeTemplate() const { return NodeInstanceTemplate; }
	USMNodeInstance* GetNodeTemplateFromGuid(const FGuid& Guid) const;
	virtual bool AreTemplatesFullyLoaded() const;
	virtual int32 GetIndexOfTemplate(const FGuid& Guid) const { return INDEX_NONE; }
	virtual USMNodeInstance* GetTemplateFromIndex(int32 Index) const { return nullptr; }

	/**
	 * Return every node template, starting with primary node template, followed by any stack items.
	 * All possible indices are filled so some items might be nullptr.
	 */
	virtual void GetAllNodeTemplates(TArray<USMNodeInstance*>& OutNodeInstances) const;
	
	template<typename T>
	T* GetNodeTemplateAs(bool bCheck = false) const
	{
		return bCheck ? CastChecked<T>(NodeInstanceTemplate) : Cast<T>(NodeInstanceTemplate);
	}
	
	/** The state machine graph this node is placed in. */
	USMGraph* GetOwningStateMachineGraph() const;

	/** The graph this node owns and represents. */
	UEdGraph* GetBoundGraph() const { return BoundGraph; }

	/** Sets the bound graph to nullptr. */
	void ClearBoundGraph() { BoundGraph = nullptr; }

	/**
	 * Create graph properties for valid graph property structs or exposed properties on the node template.
	 * @param bGenerateNewGuids will either create new guids for struct properties or re-sync exposed properties.
	 * It will always re-sync the K2 property nodes with their containers.
	 */
	void CreateGraphPropertyGraphs(bool bGenerateNewGuids = false);

	/**
	 * Create graph properties for a specific template.
	 *
	 * @param Template the template to create properties for.
	 * @param bGenerateNewGuids will either create new guids for struct properties or re-sync exposed properties.
	 * @param LiveGuidsInOut all active guids. Will not reset set.
	 * @param bResetNonVariableGuids resets the guid of pure graph properties by setting the template guid.
	 *
	 * @return true if there has been a change.
	 */
	bool CreateGraphPropertyGraphsForTemplate(USMNodeInstance* Template, bool bGenerateNewGuids, TSet<FGuid>& LiveGuidsInOut, bool bResetNonVariableGuids = false);

	/** Destroy all property graphs associated with a template. */
	void RemoveGraphPropertyGraphsForTemplate(USMNodeInstance* Template);
	
	UEdGraph* GetGraphPropertyGraph(const FGuid& Guid) const;
	USMGraphK2Node_PropertyNode_Base* GetGraphPropertyNode(const FGuid& Guid) const;

	/**
	 * Search for a property node by variable name.
	 *
	 * @param VariableName the name of the variable.
	 * @param TemplateMatch a template who's guid will be checked against property template guid.
	 * @param Index the index to look for if this is an array property.
	 */
	USMGraphK2Node_PropertyNode_Base* GetGraphPropertyNode(const FName& VariableName, const USMNodeInstance* TemplateMatch = nullptr, int32 Index = INDEX_NONE) const;

	/**
	 * Search for a property node by variable name. Returns all results for the name which allows all elements of an array to be obtained.
	 *
	 * @param VariableName the name of the variable.
	 * @param TemplateMatch a template who's guid will be checked against property template guid.
	 * @param Index the index to look for if this is an array property.
	 */
	TArray<USMGraphK2Node_PropertyNode_Base*> GetGraphPropertyNodes(const FName& VariableName, const USMNodeInstance* TemplateMatch = nullptr, int32 Index = INDEX_NONE) const;

	const TMap<FGuid, UEdGraph*>& GetAllPropertyGraphs() const { return GraphPropertyGraphs; }
	const TMap<FGuid, USMGraphK2Node_PropertyNode_Base*>& GetAllPropertyGraphNodes() const { return GraphPropertyNodes; }
	/** Look for all property nodes that should be exposed. */
	TArray<USMGraphK2Node_PropertyNode_Base*> GetAllPropertyGraphNodesAsArray(const USMNodeInstance* TemplateMatch = nullptr) const;
	void InitPropertyGraphNodes(UEdGraph* PropertyGraph, FSMGraphProperty_Base* Property);
	void RefreshAllProperties(bool bModify, bool bSetFromPinFirst = true);

	/** Refresh a property graph by its Guid. */
	void RefreshPropertyByGuid(const FGuid& Guid, bool bModify, bool bSetFromPinFirst = true);
	
	/** Creates property graphs and refreshes properties. */
	void ForceRecreateProperties();
	
	/** Updates properties from pin inputs. */
	void SetGraphPropertyDefaultsFromPins();

	/** Updates pin inputs from graph properties. */
	void SetPinsFromGraphProperties(bool bUseArchetype);
	
	USMGraphK2Node_PropertyNode_Base* GetPropertyNodeUnderMouse() const;
	/** If property graphs can be placed within this node. */
	virtual bool SupportsPropertyGraphs() const { return false; }
	
	virtual UEdGraphPin* GetInputPin() const;
	virtual UEdGraphPin* GetOutputPin() const;

	/** Returns the first output node. */
	UEdGraphNode* GetOutputNode() const;

	/** Returns all connected output nodes. */
	void GetAllOutputNodes(TArray<UEdGraphNode*>& OutNodes) const;

	template<typename T>
	void GetAllOutputNodesAs(TArray<T*>& OutNodes) const
	{
		TArray<UEdGraphNode*> Nodes;
		GetAllOutputNodes(Nodes);

		for (UEdGraphNode* Node : Nodes)
		{
			if (T* TNode = Cast<T>(Node))
			{
				OutNodes.Add(TNode);
			}
		}
	}

	/** The background color this node should be. Separate from slate so we can use one slate object
	 * to represent many different nodes. */
	virtual FLinearColor GetBackgroundColor() const;
	/** The background color to use when this node is being debugged. */
	virtual FLinearColor GetActiveBackgroundColor() const;
	/** The icon image to use. */
	virtual const FSlateBrush* GetNodeIcon() const;

	/** If this is a node that gets compiled into the runtime blueprint.  */
	virtual bool CanExistAtRuntime() const { return true; };

	/** Helper to locate the runtime node this node represents. */
	FSMNode_Base* FindRuntimeNode() const;
	/** Locates the current debug node if one exists. */
	const FSMNode_Base* GetDebugNode() const;
	
	float GetDebugTime() const { return DebugTotalTime; }
	virtual float GetMaxDebugTime() const;
	virtual bool IsDebugNodeActive() const { return bIsDebugActive; }
	virtual bool WasDebugNodeActive() const { return bWasDebugActive; }

	virtual FName GetFriendlyNodeName() const { return "Node"; }

	/** Configure outdated versions. Currently called from the editor module on load and from pre-compile. */
	bool ConvertToCurrentVersion(bool bOnlyOnLoad = true);

protected:
	/** Called at the end of ConvertToCurrentVersion before the version is set. */
	virtual void OnConvertToCurrentVersion(bool bOnlyOnLoad) {};

public:
	/** Sets the version field to the current version. No additional changes are made. */
	bool SetToCurrentVersion();
	
	/** FOR TESTING: Force set to a specific version. */
	void ForceSetVersion(int32 NewVersion);
	
	/** Brings in old values previously defined in the node and sets them on the template. */
	virtual void ImportDeprecatedProperties() {}
	
	/** If the node is being pasted in this frame. */
	bool IsBeingPasted() const { return bJustPasted; }

	/** If the node is pre-compiling for this frame. */
	bool IsPreCompiling() const { return bIsPreCompiling; }

	/** If the node is performing an edit undo / redo. */
	bool IsEditUndo() const { return bIsEditUndo; }
	
	/**
	 * Request the corresponding slate widget refresh itself.
	 * @param bFullRefresh Completely redraw the widget.
	 */
	void RequestSlateRefresh(bool bFullRefresh = false);

	/** Reset any cached values saved. */
	virtual void ResetCachedValues();

	/** Called if this node is duplicated by the compiler. */
	void RecordDuplicatedNodeGuid(const FGuid& InGuid);

private:
	void OnBlueprintCacheCleared(const USMBlueprint* Blueprint);

private:
	friend class SGraphNode_BaseNode;
	FOnGraphNodeRefreshRequested OnGraphNodeRefreshRequestedEvent;
	
protected:
	virtual FLinearColor Internal_GetBackgroundColor() const;
	/** Return the custom color from a specific node instance.
	 *
	 * @param NodeInstance If null then the default NodeInstanceTemplate will be used.
	 */
	const FLinearColor* GetCustomBackgroundColor(const USMNodeInstance* NodeInstance = nullptr) const;

	void RemovePropertyGraph(USMPropertyGraph* PropertyGraph, bool RemoveFromMaps);
	void HandlePropertyGraphArrayRemoval(TArray<FSMGraphProperty_Base*>& GraphProperties, TArray<TSharedPtr<FSMGraphProperty>>& TempGraphProperties,
		FProperty* TargetProperty, int32 RemovalIndex, int32 ArraySize, const FSMGraphProperty* OverrideGraphProperty, USMNodeInstance* Template);
	void HandlePropertyGraphArrayInsertion(TArray<FSMGraphProperty_Base*>& GraphProperties, TArray<TSharedPtr<FSMGraphProperty>>& TempGraphProperties,
		FProperty* TargetProperty, int32 InsertionIndex, int32 ArraySize, const FSMGraphProperty* OverrideGraphProperty, USMNodeInstance* Template,
		bool bDuplicate = false);
	void HandlePropertyGraphArraySwap(TArray<FSMGraphProperty_Base*>& GraphProperties, int32 IndexA, int32 IndexB, USMNodeInstance* Template);

public:
	/** Signal that array indices are being swapped. Requires property graphs recreated. */
	void NotifySwapPropertyGraphArrayElements(const FName& InPropertyName, int32 IndexA, int32 IndexB, USMNodeInstance* InNodeInstance);

private:
	struct FArraySwapData
	{
		FName ArrayPropertyNameToSwap = NAME_None;
		int32 ArrayIndexToSwapA = INDEX_NONE;
		int32 ArrayIndexToSwapB = INDEX_NONE;
	};

	TMap<TWeakObjectPtr<USMNodeInstance>, FArraySwapData> ArraySwapData;

protected:
	/** Called when a UObject property changes. */
	void HandleOnPropertyChangedEvent(UObject* InObject, FPropertyChangedEvent& InPropertyChangedEvent);

private:
	FDelegateHandle OnPropertyChangedHandle;
	
protected:
	UPROPERTY()
	TArray<FSMGraphNodeLog> CollectedLogs;

	UPROPERTY()
	UEdGraph* BoundGraph;

	/** The node position on the graph. This is managed completely by the editor graph. */
	UPROPERTY(VisibleDefaultsOnly, AdvancedDisplay, Category = Display)
	FVector2D NodePosition;
	
	/** The instanced template to use as an archetype. This node name is used in EditorCustomization directly! */
	UPROPERTY(VisibleDefaultsOnly, Instanced, Category = "Class", meta = (DisplayName=Template, DisplayThumbnail=true))
	USMNodeInstance* NodeInstanceTemplate;

	UPROPERTY()
	TMap<FGuid, UEdGraph*> GraphPropertyGraphs;

	UPROPERTY()
	TMap<FGuid, USMGraphK2Node_PropertyNode_Base*> GraphPropertyNodes;

	/** The template each graph property points to. Generally the NodeInstanceTemplate but if a state uses the state stack it will vary. */
	UPROPERTY()
	TMap<FGuid, USMNodeInstance*> GraphPropertyTemplates;

	/** Node guids set by the compiler if this node is duplicated for run-time. */
	TSet<FGuid> DuplicatedNodeGuids;

	UPROPERTY(Transient)
	mutable FSlateBrush CachedBrush;

	UPROPERTY(Transient)
	mutable FString CachedTexture;

	UPROPERTY(Transient)
	mutable FVector2D CachedTextureSize;

	UPROPERTY(Transient)
	mutable FLinearColor CachedNodeTintColor;

	/** Resets on active change. */
	float DebugTotalTime;
	float MaxTimeToShowDebug;

	uint32 bIsDebugActive:	1;
	uint32 bWasDebugActive: 1;

	uint32 bIsPreCompiling: 1;
	uint32 bJustPasted: 1;
	uint32 bIsEditUndo: 1;
	
	/** Defaults to true and property graphs are reconstructed when a property changes on the node. */
	uint32 bCreatePropertyGraphsOnPropertyChange: 1;

	/** Do not modify. */
	uint32 bCreatePropertyGraphsSilently: 1;

	/**
	 * Redirect caused a post edit change, such as a force delete.
	 * Only valid by default during PostEditChangeProperty of this class.
	 */
	uint32 bChangeFromRedirect: 1;

private:
	uint32 bNativeGuidConversion: 1;
	
public:
	/** Member flag for forcing guid regeneration. */
	uint32 bRequiresGuidRegeneration:	1;
	
	/** True iff bRequiresGuidRegeneration and loaded version wrong. */
	uint32 bNeedsStateStackConversion:	1;

	/** Testing flag for forcing old guid generation WITHOUT template support. */
	uint32 bTEST_ForceNoTemplateGuid:	1;

	/** Property category expansion state for slate node. */
	UPROPERTY(config)
	TMap<FString, bool> PropertyCategoriesExpanded;

protected:
	int32 GetLoadedVersion() const { return LoadedVersion; }
	
private:
	// Graph node properties deprecated in favor of being stored on the node template.
#define TEMPLATE_PROPERTY_VERSION	1
#define STATESTACK_VERSION			2
#define CURRENT_VERSION STATESTACK_VERSION

	/** The current loaded version. Once saved it should be the highest version available. */
	UPROPERTY()
	int32 LoadedVersion = 0;
};
