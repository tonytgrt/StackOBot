using UnrealBuildTool;

public class StackOBot : ModuleRules
{
    public StackOBot(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",    // Enhanced Input system (already a plugin in this project)
            "Niagara",          // VFX for laser beam (already a plugin)
            "UMG",              // Crosshair widget
        });
    }
}