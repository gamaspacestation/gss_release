// Copyright 2022 UNAmedia. All Rights Reserved.

#pragma once

#include <Engine/EngineBaseTypes.h>



class FMixamoToolkitEditorIntegration : public TSharedFromThis<FMixamoToolkitEditorIntegration>
{
public:
	void Register();
	void Unregister();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	// Store currently selected assets from Content Browser here to avoid passing them in lambda closures.
	TArray<FAssetData> ContentBrowserSelectedAssets;

private:
	// Tooltips.
	FText TooltipGetter_RetargetMixamoSkeletons() const;
	FText TooltipGetter_ExtractRootMotion() const;

private:

	// Actions.
	void ExecuteAction_RetargetMixamoSkeletons() const;
	bool CanExecuteAction_RetargetMixamoSkeleton(const FAssetData & Asset) const;
	bool CanExecuteAction_RetargetMixamoSkeletons() const;

	void ExecuteAction_ExtractRootMotion() const;
	bool CanExecuteAction_ExtractRootMotion(const FAssetData & Asset) const;
	bool CanExecuteAction_ExtractRootMotion() const;
	
	TSharedRef<class FExtender> MakeContentBrowserContextMenuExtender(const TArray<FAssetData> & NewSelectedAssets);
	void AddContentBrowserContextSubMenu(class FMenuBuilder& MenuBuilder) const;
	void AddContentBrowserContextMenuEntries(class FMenuBuilder& MenuBuilder) const;
};