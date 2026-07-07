#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FRMFGlbSceneExporterModule : public IModuleInterface
{
public:
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;

private:
  void RegisterMenus();
  void RunExportScript();
  void OpenExportFolder();
};
