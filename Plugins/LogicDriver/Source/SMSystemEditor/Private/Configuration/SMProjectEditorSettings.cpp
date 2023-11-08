// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "SMProjectEditorSettings.h"

#include "SMConduitInstance.h"
#include "SMStateInstance.h"
#include "SMStateMachineInstance.h"
#include "SMTransitionInstance.h"

USMProjectEditorSettings::USMProjectEditorSettings()
{
	InstalledVersion = "";
	bUpdateAssetsOnStartup = true;
	bDisplayAssetUpdateProgress = true;
	bDisplayUpdateNotification = true;
	
	bDisplayMemoryLimitsOnCompile = true;
	bAlwaysDisplayStructMemoryUsage = false;
	StructMemoryLimitWarningThreshold = 0.9f;
	
	bDisplayInputEventNotes = true;
	
	bRestrictInvalidCharacters = true;
	bWarnIfChildrenAreOutOfDate = true;
	bCalculateGuidsOnCompile = true;
	bLinkerLoadHandling = true;
	
	bDefaultNewTransitionsToTrue = false;
	bConfigureNewConduitsAsTransitions = true;
	
	EditorNodeConstructionScriptSetting = ESMEditorConstructionScriptProjectSetting::SM_Standard;
	DefaultStateClass = USMStateInstance::StaticClass();
	DefaultStateMachineClass = USMStateMachineInstance::StaticClass();
	DefaultConduitClass = USMConduitInstance::StaticClass();
	DefaultTransitionClass = USMTransitionInstance::StaticClass();
	bEnableVariableCustomization = true;
	bEnableReferenceTemplatesByDefault = false;
	bRunConstructionScriptsOnLoad = true;

	OverrideActorSoftReferencePins = ESMPinOverride::LogicDriverOnly;
	
	bEnablePreviewMode = true;
}

void USMProjectEditorSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	SaveConfig();
}
