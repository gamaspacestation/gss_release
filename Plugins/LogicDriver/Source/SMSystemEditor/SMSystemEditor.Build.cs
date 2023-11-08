// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class SMSystemEditor : ModuleRules
{
	public SMSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		DefaultBuildSettings = BuildSettingsVersion.V2;
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				Path.Combine(ModuleDirectory, "Public")
			}
		);

		PrivateIncludePaths.AddRange(
			new string[]
			{
				Path.Combine(ModuleDirectory, "Private")
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"SMSystem"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine",
				"CoreUObject",
				"Projects",
				"UnrealEd",
				"InputCore",
				"SlateCore",
				"Slate",
				"EditorStyle",
				"EditorWidgets",
				"ToolMenus",
				"ToolWidgets",
				"AssetTools",
				"GameplayTags",

				"WorkspaceMenuStructure",
				"DetailCustomizations",
				"PropertyEditor",

				"BlueprintGraph",
				"Kismet",
				"KismetCompiler",
				"KismetWidgets",

				"GraphEditor",
				"ContentBrowser",

				"ApplicationCore",
				"AppFramework",
				"MainFrame"
			}
		);

		PrivateIncludePathModuleNames.AddRange(
			new string[]
			{
				"AssetRegistry",
				"AssetTools",
				"BlueprintGraph",
				"Kismet",
				"KismetCompiler",
				"KismetWidgets",
				"EditorWidgets",
				"ContentBrowser",

				"SMPreviewEditor"
			}
		);
	}
}