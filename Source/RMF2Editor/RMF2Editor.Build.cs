using UnrealBuildTool;

public class RMF2Editor : ModuleRules
{
    public RMF2Editor(ReadOnlyTargetRules Target) : base(Target)
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
            "GLTFExporter",
            "AssetRegistry",
            "AssetTools",
            "MaterialEditor",
            "Json"
        });
    }
}
