using System.IO;
using UnrealBuildTool;

public class VDA5050CoreWrapper : ModuleRules
{
    public VDA5050CoreWrapper(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseRTTI = true;
        bEnableExceptions = true;

        string ThirdPartyPath = Path.GetFullPath(
            Path.Combine(ModuleDirectory, "../../ThirdParty/"));
        string IncludePath = Path.Combine(ThirdPartyPath, "include");
        string LibPath = Path.Combine(ThirdPartyPath, "lib");

        PrivateIncludePaths.Add(IncludePath);

        PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libvda5050_execution.so"));
        PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libmqtt_client.so"));
        PublicAdditionalLibraries.Add(Path.Combine(LibPath, "liblogger.so"));
        PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libclient.so"));
        PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libpaho-mqttpp3.so"));
        PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libpaho-mqtt3a.so"));
        PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libfmt.so"));

        // Copy to output dir at runtime
        RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)", "libvda5050_execution.so"), Path.Combine(LibPath, "libvda5050_execution.so"), StagedFileType.NonUFS);
        RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)", "libmqtt_client.so"), Path.Combine(LibPath, "libmqtt_client.so"), StagedFileType.NonUFS);
        RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)", "liblogger.so"), Path.Combine(LibPath, "liblogger.so"), StagedFileType.NonUFS);
        RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)", "libclient.so"), Path.Combine(LibPath, "libclient.so"), StagedFileType.NonUFS);
        RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)", "libpaho-mqttpp3.so"), Path.Combine(LibPath, "libpaho-mqttpp3.so"), StagedFileType.NonUFS);
        RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)", "libpaho-mqtt3a.so"), Path.Combine(LibPath, "libpaho-mqtt3a.so"), StagedFileType.NonUFS);
        RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)", "libfmt.so"), Path.Combine(LibPath, "libfmt.so"), StagedFileType.NonUFS);

        PublicRuntimeLibraryPaths.Add(LibPath);

        PublicSystemLibraries.Add("pthread");
        PublicSystemLibraries.Add("dl");

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );
    }
}
