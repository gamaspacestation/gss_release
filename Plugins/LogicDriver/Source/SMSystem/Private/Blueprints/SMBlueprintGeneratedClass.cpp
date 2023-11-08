// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "Blueprints/SMBlueprintGeneratedClass.h"

USMBlueprintGeneratedClass::USMBlueprintGeneratedClass(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USMBlueprintGeneratedClass::PurgeClass(bool bRecompilingOnLoad)
{
	Super::PurgeClass(bRecompilingOnLoad);

	RootGuid.Invalidate();
}

void USMBlueprintGeneratedClass::SetRootGuid(const FGuid& Guid)
{
	RootGuid = Guid;
}


USMNodeBlueprintGeneratedClass::USMNodeBlueprintGeneratedClass(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}