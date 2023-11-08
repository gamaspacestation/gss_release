//  Copyright (c) 2022 KomodoBit Games. All rights reserved.


#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FPedestrian_SystemModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
