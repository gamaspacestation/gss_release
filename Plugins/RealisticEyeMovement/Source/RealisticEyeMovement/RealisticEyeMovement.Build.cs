using System.IO;
using UnrealBuildTool;

public class RealisticEyeMovement : ModuleRules
{
    public RealisticEyeMovement(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange( new string[] { "Core", "CoreUObject", "Engine"});
    }
}
