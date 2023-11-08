// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class SMAssetTools : ModuleRules
{
	public SMAssetTools(ReadOnlyTargetRules Target) : base(Target)
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
				"EditorStyle",
				"EditorWidgets",
				"AssetTools",
				"GraphEditor",
				"WorkspaceMenuStructure",
				"DesktopPlatform",
				"Json",
				"JsonUtilities",
				"ContentBrowserData"
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