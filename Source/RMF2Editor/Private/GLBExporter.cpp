#include "GLBExporter.h"

#include "HAL/PlatformProcess.h"
#include "Helpers/GLBExporter/GLBExportHelper.h"
#include "IPythonScriptPlugin.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FGLBExporter"

void FGLBExporter::RegisterMenus()
{
  UToolMenus::RegisterStartupCallback(
      FSimpleMulticastDelegate::FDelegate::CreateRaw(
          this,
          &FGLBExporter::RegisterMenuEntries
      )
  );
}

void FGLBExporter::UnregisterMenus()
{
  UToolMenus::UnRegisterStartupCallback(this);
  UToolMenus::UnregisterOwner(this);
}

void FGLBExporter::RegisterMenuEntries()
{
  FToolMenuOwnerScoped OwnerScoped(this);

  UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
  FToolMenuSection& Section = Menu->FindOrAddSection("GLBSceneExporter");

  Section.AddMenuEntry(
      "ExportSceneToGLB",
      LOCTEXT("ExportSceneToGLB", "Export Scene to GLB"),
      LOCTEXT(
          "ExportSceneToGLBTooltip",
          "Exports the current level to GLB using the native C++ pipeline."
      ),
      FSlateIcon(),
      FUIAction(FExecuteAction::CreateRaw(this, &FGLBExporter::RunNativeExport))
  );

  Section.AddMenuEntry(
      "ExportSceneToGLBPython",
      LOCTEXT("ExportSceneToGLBPython", "Export Scene to GLB (Python Legacy)"),
      LOCTEXT(
          "ExportSceneToGLBPythonTooltip",
          "Runs the previous Python exporter for output comparison."
      ),
      FSlateIcon(),
      FUIAction(FExecuteAction::CreateRaw(this, &FGLBExporter::RunExportScript))
  );

  Section.AddMenuEntry(
      "OpenGLBExportFolder",
      LOCTEXT("OpenGLBExportFolder", "Open GLB Export Folder"),
      LOCTEXT(
          "OpenGLBExportFolderTooltip",
          "Opens the GLB export output folder."
      ),
      FSlateIcon(),
      FUIAction(FExecuteAction::CreateRaw(this, &FGLBExporter::OpenExportFolder)
      )
  );
}

void FGLBExporter::RunExportScript()
{
  const TSharedPtr<IPlugin> Plugin =
      IPluginManager::Get().FindPlugin(TEXT("RMF2ForUnreal"));
  if (!Plugin.IsValid())
  {
    UE_LOG(LogTemp, Error, TEXT("RMF2ForUnreal plugin not found."));
    return;
  }

  const FString ScriptPath = FPaths::Combine(
      Plugin->GetBaseDir(),
      TEXT("Content/Python/RMF2Exporter/GLBExporter/GLBExporter.py")
  );
  if (!FPaths::FileExists(ScriptPath))
  {
    UE_LOG(LogTemp, Error, TEXT("Export script not found: %s"), *ScriptPath);
    return;
  }

  UE_LOG(
      LogTemp,
      Display,
      TEXT("Running legacy Python exporter: %s"),
      *ScriptPath
  );

  const FString PythonCommand = FString::Printf(
      TEXT("exec(open(r'%s', encoding='utf-8').read())"),
      *ScriptPath
  );
  IPythonScriptPlugin::Get()->ExecPythonCommand(*PythonCommand);
}

void FGLBExporter::RunNativeExport() { FGLBExportHelper::Run(); }

void FGLBExporter::OpenExportFolder()
{
  const FString ExportFolder =
      FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("GLTFExports"));
  FPlatformProcess::ExploreFolder(*ExportFolder);
}

#undef LOCTEXT_NAMESPACE
