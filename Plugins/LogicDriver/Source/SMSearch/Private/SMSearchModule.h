// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "Widgets/Docking/SDockTab.h"

#include "ISMSearchModule.h"

class FSMSearchModule : public ISMSearchModule
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual TSharedPtr<ISMSearch> GetSearchInterface() const override;

private:
	void RegisterSettings();
	void UnregisterSettings();
	
	static TSharedRef<SDockTab> SpawnSearchInTab(const FSpawnTabArgs& SpawnTabArgs);

private:
	mutable TSharedPtr<ISMSearch> SearchInterface;
};
