// Copyright 2022 UNAmedia. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"



class FMixamoToolkitCommands : public TCommands<FMixamoToolkitCommands>
{
public:
	FMixamoToolkitCommands();

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	//TSharedPtr< FUICommandInfo > OpenBatchConverterWindow;
	TSharedPtr< FUICommandInfo > RetargetMixamoSkeleton;
	TSharedPtr< FUICommandInfo > ExtractRootMotion;
};