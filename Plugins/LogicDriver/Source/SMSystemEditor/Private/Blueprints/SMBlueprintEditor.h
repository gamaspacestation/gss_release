// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "BlueprintEditor.h"

#include "ISMSystemEditorModule.h"

class USMBlueprint;
class FSMBlueprintEditorToolbar;
class ISMPreviewModeViewportClient;

class SMSYSTEMEDITOR_API FSMBlueprintEditor : public FBlueprintEditor
{
public:
	FSMBlueprintEditor();
	virtual ~FSMBlueprintEditor() override;

	void InitSMBlueprintEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, USMBlueprint* Blueprint);

	// IToolkit
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FString GetDocumentationLink() const override;
	// ~IToolkit

	// FTickableEditorObject
	virtual void Tick(float DeltaTime) override;
	// ~FTickableEditorObject

	// FBlueprintEditor
	virtual void CreateDefaultCommands() override;
	virtual void RefreshEditors(ERefreshBlueprintEditorReason::Type Reason = ERefreshBlueprintEditorReason::UnknownReason) override;
	virtual void SetCurrentMode(FName NewMode) override;
	virtual void JumpToHyperlink(const UObject* ObjectReference, bool bRequestRename) override;
	virtual void OnBlueprintChangedImpl(UBlueprint* InBlueprint, bool bIsJustBeingCompiled) override;
	virtual FGraphAppearanceInfo GetGraphAppearance(UEdGraph* InGraph) const override;
	virtual void PasteNodesHere(UEdGraph* Graph, const FVector2D& Location) override;
	virtual void DeleteSelectedNodes() override;
	// ~FBlueprintEditor

	/** Clear the current selection, select the new nodes, and optionally focus the selection which accounts for multiple nodes. */
	void SelectNodes(const TSet<UEdGraphNode*>& InGraphNodes, bool bZoomToFit = false);

	/** The main editor toolbar. */
	TSharedPtr<FSMBlueprintEditorToolbar> GetStateMachineToolbar() const { return StateMachineToolbar; }

	/** Return the loaded blueprint as a USMBlueprint. */
	USMBlueprint* GetStateMachineBlueprint() const;

	/** True during destructor. */
	bool IsShuttingDown() const { return bShuttingDown; }
	
	void CloseInvalidTabs();
	
	bool IsSelectedPropertyNodeValid(bool bCheckReadOnlyStatus = true) const;

	/** Graph nodes selected by the user at the time of a paste operation. */
	const TSet<TWeakObjectPtr<class USMGraphNode_Base>>& GetSelectedGraphNodesDuringPaste() const { return SelectedGraphNodesOnPaste; }

	/** Set by property node. This isn't guaranteed to be valid unless used in a selected property command. */
	TWeakObjectPtr<class USMGraphK2Node_PropertyNode_Base> SelectedPropertyNode;
	
	/** Set when right clicking on a node. */
	TWeakObjectPtr<UEdGraphNode> SelectedNodeForContext;
	
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCreateGraphEditorCommands, FSMBlueprintEditor*, TSharedPtr<FUICommandList>);
	/** Event fired when a graph in a state machine blueprint is renamed. */
	static FOnCreateGraphEditorCommands OnCreateGraphEditorCommandsEvent;

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSelectedNodesChanged, TSharedPtr<FSMBlueprintEditor>, const TSet<UObject*>& /* New Selection */);
	FOnSelectedNodesChanged OnSelectedNodesChangedEvent;

protected:
	//////////////////////////////////////
	/////// Begin Preview Module
	
	/** Starts previewing the asset. */
	void StartPreviewSimulation();

	/** Verifies a preview simulation can start. */
	bool CanStartPreviewSimulation() const;
	
	/** Terminates simulation */
	void StopPreviewSimulation();

	/** Deletes the selected preview item. */
	void DeletePreviewSelection();

public:
	/** Store a reference to preview client. */
	void SetPreviewClient(const TSharedPtr<ISMPreviewModeViewportClient>& InPreviewClient);

	/** Direct access to the preview client. */
	TWeakPtr<ISMPreviewModeViewportClient> GetPreviewClient() const { return PreviewViewportClient; }

	/** True if the preview setting was enabled when this editor was opened. */
	bool IsPreviewModeAllowed() const { return bPreviewModeAllowed; }
	
	/////// End Preview Module
	//////////////////////////////////////

protected:

	// FEditorUndoClient
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;
	// ~FEditorUndoClient

	/** Extend menu */
	void ExtendMenu();

	/** Extend toolbar */
	void ExtendToolbar();

	void BindCommands();

	/** When a debug object was set for the blueprint being edited. */
	void OnDebugObjectSet(UObject* Object);

	/** Find all nodes for the blueprint and reset their debug state. */
	void ResetBlueprintDebugStates();
	
	/** FBlueprintEditor interface */
	virtual void OnActiveTabChanged(TSharedPtr<SDockTab> PreviouslyActive, TSharedPtr<SDockTab> NewlyActivated) override;
	virtual void OnSelectedNodesChangedImpl(const TSet<UObject*>& NewSelection) override;
	virtual void OnCreateGraphEditorCommands(TSharedPtr<FUICommandList> GraphEditorCommandsList) override;
	virtual void CopySelectedNodes() override;
	virtual void PasteNodes() override;
	/** ~FBlueprintEditor interface */
	
	/** A self transition for the same state. */
	void CreateSingleNodeTransition();
	bool CanCreateSingleNodeTransition() const;

	void CollapseNodesToStateMachine();
	bool CanCollapseNodesToStateMachine() const;

	void CutCombineStates();
	void CopyCombineStates();
	bool CanCutOrCopyCombineStates() const;
	
	void ConvertStateMachineToReference();
	bool CanConvertStateMachineToReference() const;

	void ChangeStateMachineReference();
	bool CanChangeStateMachineReference() const;

	void JumpToStateMachineReference();
	bool CanJumpToStateMachineReference() const;

	void EnableIntermediateGraph();
	bool CanEnableIntermediateGraph() const;

	void DisableIntermediateGraph();
	bool CanDisableIntermediateGraph() const;

	void ReplaceWithStateMachine();
	bool CanReplaceWithStateMachine() const;

	void ReplaceWithStateMachineReference();
	bool CanReplaceWithStateMachineReference() const;

	void ReplaceWithStateMachineParent();
	bool CanReplaceWithStateMachineParent() const;
	
	void ReplaceWithState();
	bool CanReplaceWithState() const;

	void ReplaceWithConduit();
	bool CanReplaceWithConduit() const;

	void GoToGraph();
	bool CanGoToGraph() const;

	void GoToNodeBlueprint();
	bool CanGoToNodeBlueprint() const;
	
	void GoToPropertyBlueprint();
	bool CanGoToPropertyBlueprint() const;

	void GoToTransitionStackBlueprint();
	bool CanGoToTransitionStackBlueprint() const;
	
	void GoToPropertyGraph();
	bool CanGoToPropertyGraph() const;

	void ClearGraphProperty();
	bool CanClearGraphProperty() const;

	void ToggleGraphPropertyEdit();
	bool CanToggleGraphPropertyEdit() const;

private:
	/** The extender to pass to the level editor to extend its window menu */
	TSharedPtr<FExtender> MenuExtender;

	/** Toolbar extender */
	TSharedPtr<FExtender> ToolbarExtender;

	/** The command list for this editor */
	TSharedPtr<FUICommandList> GraphEditorCommands;

	/** Custom toolbar used for switching modes */
	TSharedPtr<FSMBlueprintEditorToolbar> StateMachineToolbar;

	/** Selected state machine graph node */
	TWeakObjectPtr<class USMGraphK2Node_Base> SelectedStateMachineNode;

	/** The currently loaded blueprint. */
	TWeakObjectPtr<UBlueprint> LoadedBlueprint;

	/** When the user sets a debug object. */
	FDelegateHandle OnDebugObjectSetHandle;

	/** Preview world viewport. */
	TWeakPtr<ISMPreviewModeViewportClient> PreviewViewportClient;

	/** Graph nodes selected only at the time of a paste operation. */
	TSet<TWeakObjectPtr<class USMGraphNode_Base>> SelectedGraphNodesOnPaste;
	
	/** True during hyper link jump! */
	bool bJumpingToHyperLink;

	/** Called from destructor. */
	bool bShuttingDown;

	/** If preview mode has been enabled from settings for this editor. */
	bool bPreviewModeAllowed;
};


class SMSYSTEMEDITOR_API FSMNodeBlueprintEditor : public FBlueprintEditor
{
public:
	FSMNodeBlueprintEditor();
	virtual ~FSMNodeBlueprintEditor() override;

	// IToolkit
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetDocumentationLink() const override;
	virtual FGraphAppearanceInfo GetGraphAppearance(UEdGraph* InGraph) const override;
	// ~IToolkit

	void InitNodeBlueprintEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost,
		const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode);
	
private:
#if !LOGICDRIVER_HAS_PROPER_VARIABLE_CUSTOMIZATION
	/** Currently loaded node blueprint editors. */
	static TSet<FSMNodeBlueprintEditor*> AllNodeBlueprintEditors;
	bool bVariablesCustomized = false;
#endif

protected:
	// FBlueprintEditor
	virtual void OnBlueprintChangedImpl(UBlueprint* InBlueprint, bool bIsJustBeingCompiled) override;
	// ~FBlueprintEditor
};