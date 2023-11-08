// Copyright 2022 UNAmedia. All Rights Reserved.

#include "Runtime/Launch/Resources/Version.h"

#include "MixamoAnimationRetargeting.h"

// You should place include statements to your module's private header files here.  You only need to
// add includes for headers that are used in most of your module's source files though.

#include "MixamoToolkitPrivate.h"



// Forward declaration of FAssetData
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 17) || (ENGINE_MAJOR_VERSION > 4)
// https://github.com/EpicGames/UnrealEngine/commit/70d3bd4b726884ccd6fe348fa45f11252aa99e04
// Change 3439819 by Matt.Kuhlenschmidt
// Turned FAssetData into a struct for some upcoming script exposure of FAssetData
struct FAssetData;
#else
class FAssetData;
#endif
