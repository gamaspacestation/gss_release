#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

REALISTICEYEMOVEMENT_API DECLARE_LOG_CATEGORY_EXTERN(LogRealisticEyeMovement, Log, All);

class FRealisticEyeMovementModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
