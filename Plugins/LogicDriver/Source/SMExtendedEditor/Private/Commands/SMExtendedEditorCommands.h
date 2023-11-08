// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "Framework/Commands/UICommandList.h"
#include "Framework/Commands/Commands.h"
#include "EditorStyleSet.h"
#include "SMUnrealTypeDefs.h"

class FSMExtendedEditorCommands : public TCommands<FSMExtendedEditorCommands>
{
public:
	/** Constructor */
	FSMExtendedEditorCommands()
		: TCommands<FSMExtendedEditorCommands>(TEXT("SMExtendedEditor"), NSLOCTEXT("Contexts", "SMExtendedEditor", "State Machine Editor"),
			NAME_None, FSMUnrealAppStyle::Get().GetStyleSetName())
	{
	}

	// TCommand
	virtual void RegisterCommands() override;
	FORCENOINLINE static const FSMExtendedEditorCommands& Get();
	// ~TCommand

	static void OnEditorCommandsCreated(class FSMBlueprintEditor* Editor, TSharedPtr<FUICommandList> CommandList);

	/** Use the node to edit. */
	TSharedPtr<FUICommandInfo> StartTextPropertyEdit;
	static void EditText(FSMBlueprintEditor* Editor);
	static bool CanEditText(FSMBlueprintEditor* Editor);
	
};
