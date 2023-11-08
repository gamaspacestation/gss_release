// Copyright 2022 UNAmedia. All Rights Reserved.

using UnrealBuildTool;

public class MixamoAnimationRetargeting : ModuleRules
{
	public MixamoAnimationRetargeting(ReadOnlyTargetRules Target) : base(Target)
	{
		ShortName="MAR";
		
		PublicIncludePaths.AddRange(
			new string[] {
				//"MixamoAnimationRetargeting/Public"
				// ... add public include paths required here ...
			}
		);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"MixamoAnimationRetargeting/Private",
				// ... add other private include paths required here ...
			}
		);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
				"Projects",
				"InputCore",
				//"LevelEditor",
				"CoreUObject",
				"Engine",
				"UnrealEd",
				"Slate",
				"SlateCore",
				"EditorStyle",
                "ContentBrowser",
				"ContentBrowserData",
				"ContentBrowserAssetDataSource",
				"RenderCore",
				"IKRig",
				"IKRigEditor",
				"MessageLog"
			}
		);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);

		// Needed to avoid a deadlock of errors: file "XXX.cpp" is required
		// to include file "XXX.h" as first header file; but without this
		// option it's also required to include the PCH header file as first
		// file...
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
    }
}
