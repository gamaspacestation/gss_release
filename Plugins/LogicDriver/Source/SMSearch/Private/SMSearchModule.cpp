// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "SMSearchModule.h"

#include "ISettingsModule.h"
#include "SMSearchLog.h"
#include "Configuration/SMSearchStyle.h"
#include "Search/SMSearch.h"
#include "Search/Views/SSMSearchView.h"

#include "ISMSystemEditorModule.h"

#include "Configuration/SMSearchSettings.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SToolTip.h"

#define LOCTEXT_NAMESPACE "SMSearchModule"

DEFINE_LOG_CATEGORY(LogLogicDriverSearch)

FText TabTitle = LOCTEXT("TabTitle", "Search (Beta)");
FText TabTooltip = LOCTEXT("TabTooltip", "Search exposed property values within Logic Driver assets.");

void FSMSearchModule::StartupModule()
{
	FSMSearchStyle::Initialize();
	RegisterSettings();

	FTabSpawnerEntry& TabSpawnerEntry = FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SSMSearchView::TabName,
		FOnSpawnTab::CreateStatic(&FSMSearchModule::SpawnSearchInTab))
		.SetDisplayName(TabTitle)
		.SetTooltipText(TabTooltip)
		.SetIcon(FSlateIcon(FSMSearchStyle::GetStyleSetName(), "SMSearch.Tabs.Find"));

	const ISMSystemEditorModule& SMBlueprintEditorModule = FModuleManager::LoadModuleChecked<ISMSystemEditorModule>(LOGICDRIVER_EDITOR_MODULE_NAME);
	TabSpawnerEntry.SetGroup(SMBlueprintEditorModule.GetToolsWorkspaceGroup().ToSharedRef());
}

void FSMSearchModule::ShutdownModule()
{
	FSMSearchStyle::Shutdown();
	UnregisterSettings();

	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SSMSearchView::TabName);
	}
}

TSharedPtr<ISMSearch> FSMSearchModule::GetSearchInterface() const
{
	if (!SearchInterface.IsValid())
	{
		SearchInterface = MakeShared<FSMSearch>();
	}

	return SearchInterface;
}

void FSMSearchModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Editor", "Plugins", "LogicDriverSearch",
			LOCTEXT("SMSearchSettingsName", "Logic Driver Search"),
			LOCTEXT("SMSearchSettingsDescription", "Manage the search settings for Logic Driver assets."),
			GetMutableDefault<USMSearchSettings>());
	}
}

void FSMSearchModule::UnregisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Editor", "Plugins", "LogicDriverSearch");
	}
}

TSharedRef<SDockTab> FSMSearchModule::SpawnSearchInTab(const FSpawnTabArgs& SpawnTabArgs)
{
	const TSharedRef<SDockTab> MajorTab = SNew(SDockTab)
	.TabRole(ETabRole::NomadTab);

	MajorTab->SetTabToolTipWidget(SNew(SToolTip).Text(TabTooltip));
	MajorTab->SetContent(SNew(SSMSearchView));
	return MajorTab;
}

IMPLEMENT_MODULE(FSMSearchModule, SMSearch)

#undef LOCTEXT_NAMESPACE
