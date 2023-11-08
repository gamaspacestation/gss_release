// Copyright 2022 UNAmedia. All Rights Reserved.

#pragma once

#include "Engine/EngineTypes.h"



/**
Class to map FName objects, using an aliased mapping table.

@attention The mapping table is aliased and it's lifecycle must include the one of this object.
	Usually you should initialize it with a static/constexpr table.
*/
class FStaticNamesMapper
{
public:
	FStaticNamesMapper(const char* const* SourceToDestinationMapping, int32 SourceToDestinationMappingNum, bool Reverse = false);

	/**
	Map a bone index from the source skeleton to a bone index into the destination skeleton.

	Returns INDEX_NONE if the bone can't be mapped.
	*/
	FName MapName(const FName & SourceName) const;
	/// Map a set of names.
	TArray<FName> MapNames(const TArray<FName>& Names) const;
	FStaticNamesMapper GetInverseMapper() const;

	void GetSource(TArray<FName>& Out) const;
	void GetDestination(TArray<FName>& Out) const;

protected:
	const char* const* Mapping;
	int32 MappingNum;
	int32 SrcOfs;
	int32 DstOfs;
};