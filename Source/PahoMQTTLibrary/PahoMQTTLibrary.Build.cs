/**
 * Copyright (C) 2025 ROS Industrial Consortium Asia Pacific
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

using System.IO;
using UnrealBuildTool;

public class PahoMQTTLibrary : ModuleRules
{
    public PahoMQTTLibrary(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // FALLBACK ABSOLUTE PATH
        // string ProjectRoot = "/home/johnaa/Documents/Unreal Projects/PahoMQTTWorkspace/Plugins/PahoMQTTLibrary";
        
        string ProjectRoot = Path.GetFullPath(Path.Combine(ModuleDirectory  , "../../"));
        string ThirdPartyPaho = Path.Combine(ProjectRoot, "ThirdParty", "Paho");
        string LibPath = Path.Combine(ThirdPartyPaho, "lib");
        string IncludePath = Path.Combine(ThirdPartyPaho, "include");

        // Include MQTT headers
        if (Directory.Exists(IncludePath))
        {
            PublicIncludePaths.Add(IncludePath);
        }

        // Use the synchronous static archive (contains MQTTClient_* functions)
        string LibPahoCStatic = Path.Combine(LibPath, "libpaho-mqtt3c.so");
        if (File.Exists(LibPahoCStatic))
        {
            PublicAdditionalLibraries.Add(LibPahoCStatic);
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
        // Paho MQTT need Json and SSL
        PublicDependencyModuleNames.AddRange(new string[] { "Core"});
        PrivateDependencyModuleNames.AddRange(new string[] { "CoreUObject", "Engine", "InputCore", "JsonUtilities", "Networking", "Projects", "SSL", "Sockets", "WebSockets"});
    }
}
