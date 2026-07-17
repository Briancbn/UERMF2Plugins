#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FRMF2EditorModule : public IModuleInterface
{
public:
  virtual ~FRMF2EditorModule() override;
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;

private:
  TUniquePtr<class FGLBExporter> GLBExporter;
};
