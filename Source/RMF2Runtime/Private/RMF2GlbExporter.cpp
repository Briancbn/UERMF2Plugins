#include "RMFGlbSceneExporter.h"
#include "ToolMenus.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "IPythonScriptPlugin.h"

#define LOCTEXT_NAMESPACE "FRMFGlbSceneExporterModule"

void FRMFGlbSceneExporterModule::StartupModule()
{
    UToolMenus::RegisterStartupCallback(
        FSimpleMulticastDelegate::FDelegate::CreateRaw(
            this,
            &FRMFGlbSceneExporterModule::RegisterMenus
        )
    );
}

void FRMFGlbSceneExporterModule::ShutdownModule()
{
    UToolMenus::UnRegisterStartupCallback(this);
    UToolMenus::UnregisterOwner(this);
}

void FRMFGlbSceneExporterModule::RegisterMenus()
{
    FToolMenuOwnerScoped OwnerScoped(this);

    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
    FToolMenuSection& Section = Menu->FindOrAddSection("RMFGLBSceneExporter");

    Section.AddMenuEntry(
        "RMFExportSceneToGLB",
        LOCTEXT("RMFExportSceneToGLB", "Export Scene to GLB"),
        LOCTEXT("RMFExportSceneToGLBTooltip", "Exports the current level to GLB using generated GLTF-safe materials."),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FRMFGlbSceneExporterModule::RunExportScript))
    );

    Section.AddMenuEntry(
        "RMFOpenGLBExportFolder",
        LOCTEXT("RMFOpenGLBExportFolder", "Open GLB Export Folder"),
        LOCTEXT("RMFOpenGLBExportFolderTooltip", "Opens the GLB export output folder."),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FRMFGlbSceneExporterModule::OpenExportFolder))
    );
}

void FRMFGlbSceneExporterModule::RunExportScript()
{
    TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("RMFGlbSceneExporter"));

    if (!Plugin.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("RMFGlbSceneExporter plugin not found."));
        return;
    }

    FString ScriptPath = FPaths::Combine(
        Plugin->GetBaseDir(),
        TEXT("Content/Python/export_scene_auto_create_gltf_materials.py")
    );

    if (!FPaths::FileExists(ScriptPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Export script not found: %s"), *ScriptPath);
        return;
    }

    UE_LOG(LogTemp, Display, TEXT("Running RMF GLB export script: %s"), *ScriptPath);

    FString PythonCommand = FString::Printf(
        TEXT("exec(open(r'%s', encoding='utf-8').read())"),
        *ScriptPath
    );

    IPythonScriptPlugin::Get()->ExecPythonCommand(*PythonCommand);
}

void FRMFGlbSceneExporterModule::OpenExportFolder()
{
    FString ExportFolder = FPaths::Combine(
        FPaths::ProjectSavedDir(),
        TEXT("GLTFExports")
    );

    FPlatformProcess::ExploreFolder(*ExportFolder);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRMFGlbSceneExporterModule, RMFGlbSceneExporter)