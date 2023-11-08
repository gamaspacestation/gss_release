// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "SMExtendedEditorModule.h"

#include "SMEditorTextGraphLogging.h"
#include "Commands/SMExtendedEditorCommands.h"
#include "Configuration/SMExtendedEditorStyle.h"
#include "Configuration/SMTextGraphEditorSettings.h"
#include "Utilities/SMTextGraphUtils.h"

#include "Blueprints/SMBlueprintEditor.h"

#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "SMExtendedEditorModule"

DEFINE_LOG_CATEGORY(LogLogicDriverExtendedEditor)

void FSMExtendedEditorModule::StartupModule()
{
	FSMExtendedEditorStyle::Initialize();
	FSMExtendedEditorCommands::Register();
	RegisterSettings();

	// Variable renames have special handling.
	RenameVariableReferencesDelegateHandle = FBlueprintEditorUtils::OnRenameVariableReferencesEvent.AddStatic(&FSMTextGraphUtils::HandleRenameVariableReferencesEvent);
	RenameGraphsDelegateHandle = USMBlueprint::OnRenameGraphEvent.AddStatic(&FSMTextGraphUtils::HandleRenameGraphEvent);
	OnEditorCommandsCreatedHandle = FSMBlueprintEditor::OnCreateGraphEditorCommandsEvent.AddStatic(&FSMExtendedEditorCommands::OnEditorCommandsCreated);
	OnPropertyChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddStatic(&FSMTextGraphUtils::HandleOnPropertyChangedEvent);
	OnBlueprintPostConditionallyCompiledHandle = FSMBlueprintEditorUtils::OnBlueprintPostConditionallyCompiledEvent.AddStatic(&FSMTextGraphUtils::HandlePostConditionallyCompileBlueprintEvent);
}

void FSMExtendedEditorModule::ShutdownModule()
{
	FBlueprintEditorUtils::OnRenameVariableReferencesEvent.Remove(RenameVariableReferencesDelegateHandle);
	USMBlueprint::OnRenameGraphEvent.Remove(RenameGraphsDelegateHandle);
	FSMBlueprintEditor::OnCreateGraphEditorCommandsEvent.Remove(OnEditorCommandsCreatedHandle);
	FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(OnPropertyChangedHandle);
	FSMBlueprintEditorUtils::OnBlueprintPostConditionallyCompiledEvent.Remove(OnBlueprintPostConditionallyCompiledHandle);
	
	FSMExtendedEditorStyle::Shutdown();
	FSMExtendedEditorCommands::Unregister();
	UnregisterSettings();
}

void FSMExtendedEditorModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "LogicDriverTextGraphEditor",
			LOCTEXT("SMTextGraphEditorSettingsName", "Logic Driver Text Graph Editor"),
			LOCTEXT("SMTextGraphEditorSettingsDescription", "Configure text graph editor settings."),
			GetMutableDefault<USMTextGraphEditorSettings>());
	}
}

void FSMExtendedEditorModule::UnregisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "LogicDriverTextGraphEditor");
	}
}

IMPLEMENT_MODULE(FSMExtendedEditorModule, SMExtendedEditor)

#undef LOCTEXT_NAMESPACE
