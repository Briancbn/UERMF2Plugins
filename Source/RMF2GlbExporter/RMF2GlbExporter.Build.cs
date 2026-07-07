using UnrealBuildTool;

public class RMF2GlbExporter : ModuleRules
{
    public RMF2GlbExporter(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "RMF2Runtime"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "UnrealEd",
            "LevelEditor",
            "Slate",
            "SlateCore",
            "ToolMenus",
            "Projects",
            "PythonScriptPlugin",
            "EditorScriptingUtilities",
            "GLTFExporter"
        });
    }
}