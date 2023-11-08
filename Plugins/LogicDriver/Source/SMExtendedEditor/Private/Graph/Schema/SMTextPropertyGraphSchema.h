// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "Graph/Schema/SMPropertyGraphSchema.h"

#include "CoreMinimal.h"

#include "SMTextPropertyGraphSchema.generated.h"

UCLASS()
class SMEXTENDEDEDITOR_API USMTextPropertyGraphSchema : public USMPropertyGraphSchema
{
	GENERATED_UCLASS_BODY()

public:
	// UEdGraphSchema_K2
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;
	virtual bool CanDuplicateGraph(UEdGraph* InSourceGraph) const override { return false; }
	/** This isn't currently called by UE4. */
	virtual void GetGraphDisplayInformation(const UEdGraph& Graph, /*out*/ FGraphDisplayInfo& DisplayInfo) const override;
	// ~UEdGraphSchema_K2
};

