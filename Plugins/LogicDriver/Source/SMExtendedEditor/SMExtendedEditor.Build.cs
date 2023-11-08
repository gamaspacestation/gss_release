// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class SMExtendedEditor : ModuleRules
{
	public SMExtendedEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		DefaultBuildSettings = BuildSettingsVersion.V2;
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				Path.Combine(ModuleDirectory, "Public"),
			}
		);

		PrivateIncludePaths.AddRange(
			new string[]
			{
				Path.Combine(ModuleDirectory, "Private"),
				Path.Combine("SMSystemEditor", "Private"),
				Path.Combine("SMExtendedRuntime", "Private")
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"SMSystem",
				"SMExtendedRuntime",
				"SMSystemEditor"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine",
				"CoreUObject",
				"UnrealEd",
				"RenderCore",
				"InputCore",
				"SlateCore",
				"Slate",
				"EditorStyle",
				"EditorWidgets",
				"ToolMenus",

				"DetailCustomizations",
				"PropertyEditor",

				"BlueprintGraph",
				"Kismet",
				"GraphEditor",

				"ApplicationCore",
				"UMG"
			}
		);

		PrivateIncludePathModuleNames.AddRange(
			new string[]
			{
				"AssetRegistry",
				"AssetTools",
				"BlueprintGraph",
				"Kismet",
				"KismetWidgets",
				"EditorWidgets"
			}
		);
	}
}