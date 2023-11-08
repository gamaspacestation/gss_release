// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class SMSearch : ModuleRules
{
	public SMSearch(ReadOnlyTargetRules Target) : base(Target)
	{
		DefaultBuildSettings = BuildSettingsVersion.V2;
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[]
			{
				Path.Combine(ModuleDirectory, "Private"),
				Path.Combine("SMSystemEditor", "Private")
			});

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"SMSystem",
				"SMSystemEditor",
				"SMExtendedRuntime"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine",
				"CoreUObject",
				"Slate",
				"SlateCore",
				"InputCore",
				"UnrealEd",
				"Projects",
				"EditorStyle",
				"EditorWidgets",
				"KismetWidgets",
				"GraphEditor",
				"BlueprintGraph",
				"WorkspaceMenuStructure",
				"Kismet",

				"SMAssetTools"
			}
		);

		PrivateIncludePaths.AddRange(
			new string[]
			{
				Path.Combine("SMSystemEditor", "Private"),
			}
		);
	}
}