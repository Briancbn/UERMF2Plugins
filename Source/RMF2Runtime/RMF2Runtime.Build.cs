using System.IO;
using UnrealBuildTool;

public class RMF2Runtime : ModuleRules
{
    public RMF2Runtime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        string ProjectRoot = Path.GetFullPath(Path.Combine(ModuleDirectory  , "../../"));
        string ThirdPartyPaho = Path.Combine(ProjectRoot, "ThirdParty");
        string LibPath = Path.Combine(ThirdPartyPaho, "lib");
        string IncludePath = Path.Combine(ThirdPartyPaho, "include");

        // Include MQTT headers
        if (Directory.Exists(IncludePath))
        {
            PublicIncludePaths.Add(IncludePath);
        }

        // Use the synchronous static archive (contains MQTTClient_* functions)
        string LibPahoCRuntime = Path.Combine(LibPath, "libpaho-mqtt3c.so");
        if (File.Exists(LibPahoCRuntime))
        {
            PublicAdditionalLibraries.Add(LibPahoCRuntime);
            RuntimeDependencies.Add(LibPahoCRuntime);
        }
        else
        {
            // Fallback (system installed) — not recommended for portability
            PublicAdditionalLibraries.Add("paho-mqtt3c");
        }

        // System libraries required when linking static archives
        PublicSystemLibraries.Add("pthread");
        PublicSystemLibraries.Add("dl");
        PublicSystemLibraries.Add("rt");

        // Module dependencies
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "VDA5050CoreWrapper",
            }
        );

        // Paho MQTT need Json and SSL
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "InputCore", 
                "JsonUtilities", 
                "Networking", 
                "Projects", 
                "SSL", 
                "Sockets", 
                "WebSockets",
            }
        );
    }
}
