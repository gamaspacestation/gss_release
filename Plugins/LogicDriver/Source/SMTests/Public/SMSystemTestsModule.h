// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"

class FSMSystemTestsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

};
