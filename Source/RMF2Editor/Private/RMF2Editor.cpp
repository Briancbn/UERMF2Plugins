// RMF2Editor.cpp
#include "RMF2Editor.h"
#include "GLBExporter.h"

FRMF2EditorModule::~FRMF2EditorModule() = default;
void FRMF2EditorModule::StartupModule()
{
  GLBExporter = MakeUnique<FGLBExporter>();
  GLBExporter->RegisterMenus();
}

void FRMF2EditorModule::ShutdownModule()
{
  if (GLBExporter)
  {
    GLBExporter->UnregisterMenus();
    GLBExporter.Reset();
  }
}

IMPLEMENT_MODULE(FRMF2EditorModule, RMF2Editor)