#include "Helpers/GLBExporter/GLBExportOptions.h"

#include "Options/GLTFExportOptions.h"

UGLTFExportOptions* FGLBExportOptionsFactory::Create(UObject* Outer)
{
  UGLTFExportOptions* Options =
      NewObject<UGLTFExportOptions>(Outer ? Outer : GetTransientPackage());

  Options->BakeMaterialInputs = EGLTFMaterialBakeMode::Disabled;
  Options->TextureImageFormat = EGLTFTextureImageFormat::PNG;
  Options->bExportVertexColors = false;
  Options->bAdjustNormalmaps = true;
  Options->bExportProxyMaterials = false;
  Options->bExportCameras = false;
  Options->bExportLights = false;

  return Options;
}
