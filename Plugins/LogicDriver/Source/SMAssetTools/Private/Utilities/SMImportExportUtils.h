// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "UObject/UnrealType.h"

namespace LD
{
	namespace ImportExportUtils
	{
		/** Checks if a property has flags for export or import. */
		SMASSETTOOLS_API bool ShouldPropertyBeImportedOrExported(const FProperty* InProperty);
	}
}
