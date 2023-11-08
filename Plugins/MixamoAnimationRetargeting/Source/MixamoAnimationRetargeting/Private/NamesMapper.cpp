// Copyright 2022 UNAmedia. All Rights Reserved.

#include "NamesMapper.h"



namespace
{

/**
Iterate over a "column" of data of a "mapping table".

@param Table The mapping table. It's assumed to be a continuos array of "items", where each item is a pair of "elements" stored continuously.
@param TableNum The number of items in the Table (this means that the Table contains TableNum*2 elements).
@param iColumn The column index of the elements we want to iterate over.
@param Functor The callback called for each iterated element. Prototype is void(const TTableItem & Elem).

If needed, we can get rid of the template using TFunctionRef<>.
*/
template<typename TTableItem, typename TFunctor>
void IterateOnMappingTable(const TTableItem * Table, int32 TableNum, int32 iColumn, TFunctor Functor)
{
	for (int32 i = 0; i < TableNum; i += 2)
	{
		Functor(Table[i + iColumn]);
	}
}

template<typename TTableItem, typename TContainer>
void GetMappingTableColumn(const TTableItem * Table, int32 TableNum, int32 iColumn, TContainer & Output)
{
	Output.Reset(TableNum / 2);
	IterateOnMappingTable(Table, TableNum, iColumn, [&](const TTableItem & Item) { Output.Emplace(Item); });
}

} // namespace *unnamed*



FStaticNamesMapper::FStaticNamesMapper(
	const char* const* SourceToDestinationMapping,
	int32 SourceToDestinationMappingNum,
	bool Reverse
)
	: Mapping(SourceToDestinationMapping),
	  MappingNum(SourceToDestinationMappingNum),
	  SrcOfs(Reverse ? 1 : 0),
	  DstOfs(Reverse ? 0 : 1)
{
	check(Mapping != nullptr);
	checkf(MappingNum % 2 == 0, TEXT("The input array is expeted to have an even number of entries"));
}



FName FStaticNamesMapper::MapName(const FName& SourceName) const
{
	for (int32 i = 0; i < MappingNum; i += 2)
	{
		if (FName(Mapping[i + SrcOfs]) == SourceName)
		{
			return FName(Mapping[i + DstOfs]);
		}
	}
	return NAME_None;
}



TArray<FName> FStaticNamesMapper::MapNames(const TArray<FName>& Names) const
{
	TArray<FName> Result;
	Result.Reserve(Names.Num());
	for (const FName & Name : Names)
	{
		const FName MappedName = MapName(Name);
		if (!MappedName.IsNone())
		{
			Result.Emplace(MappedName);
		}
	}
	return Result;
}



FStaticNamesMapper FStaticNamesMapper::GetInverseMapper() const
{
	return FStaticNamesMapper(Mapping, MappingNum, SrcOfs == 0);
}



void FStaticNamesMapper::GetSource(TArray<FName>& Out) const
{
	GetMappingTableColumn(Mapping, MappingNum, 0, Out);
}



void FStaticNamesMapper::GetDestination(TArray<FName>& Out) const
{
	GetMappingTableColumn(Mapping, MappingNum, 1, Out);
}
