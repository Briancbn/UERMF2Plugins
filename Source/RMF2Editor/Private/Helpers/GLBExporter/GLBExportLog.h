#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRMF2GLBExporter, Log, All);

struct FGLBActorConfig
{
  bool bSkipHiddenActors = true;
  bool bExportOnlyMeshActors = true;

  TArray<FString> ExcludedNameParts = {
      TEXT("camera"),
      TEXT("cinecamera"),
      TEXT("cam_"),
      TEXT("viewport"),
      TEXT("light"),
      TEXT("lighting"),
      TEXT("directional"),
      TEXT("skylight"),
      TEXT("spotlight"),
      TEXT("pointlight"),
      TEXT("rectlight"),
      TEXT("sky"),
      TEXT("atmosphere"),
      TEXT("hdri"),
      TEXT("backdrop"),
      TEXT("reflectioncapture"),
      TEXT("reflection_capture"),
      TEXT("volumetriccloud"),
      TEXT("exponentialheightfog"),
      TEXT("fog"),
      TEXT("kiva"),
      TEXT("forklift")
  };

  TArray<FString> ExcludedClassParts = {
      TEXT("cameraactor"),
      TEXT("cinecameraactor"),
      TEXT("pointlight"),
      TEXT("spotlight"),
      TEXT("directionallight"),
      TEXT("rectlight"),
      TEXT("skylight"),
      TEXT("light"),
      TEXT("skyatmosphere"),
      TEXT("exponentialheightfog"),
      TEXT("volumetriccloud"),
      TEXT("reflectioncapture"),
      TEXT("spherecapture"),
      TEXT("boxreflectioncapture"),
      TEXT("hdribackdrop")
  };
};

struct FGLBTextureConfig
{
  TArray<FString> BaseColorKeywords = {
      TEXT("basecolor"),
      TEXT("base_color"),
      TEXT("albedo"),
      TEXT("diffuse"),
      TEXT("color"),
      TEXT("colour"),
      TEXT("_bc"),
      TEXT("_d")
  };

  TArray<FString> NormalKeywords = {TEXT("normal"), TEXT("nrm"), TEXT("_n")};

  TArray<FString> ORMKeywords = {
      TEXT("orme"),
      TEXT("orm"),
      TEXT("occlusionroughnessmetallic"),
      TEXT("occlusion_roughness_metallic"),
      TEXT("roughnessmetallic"),
      TEXT("arm"),
      TEXT("rao")
  };

  TArray<FString> RoughnessKeywords = {
      TEXT("roughness"),
      TEXT("_rgh"),
      TEXT("_rough")
  };

  TArray<FString> MetallicKeywords = {
      TEXT("metallic"),
      TEXT("metalness"),
      TEXT("_metal")
  };

  TArray<FString> AOKeywords = {
      TEXT("ao"),
      TEXT("ambientocclusion"),
      TEXT("occlusion")
  };

  TArray<FString> OpacityKeywords = {
      TEXT("opacity"),
      TEXT("alpha"),
      TEXT("mask"),
      TEXT("color_mask"),
      TEXT("colour_mask")
  };
};

struct FGLBMaterialConfig
{
  FString GeneratedFolder = TEXT("/Game/Generated/GLTF_AutoMaterials");
  bool bUniqueMaterialPerSlot = false;
  bool bReuseGeneratedMaterials = true;
};

struct FGLBExportConfig
{
  FString ExportDirectoryName = TEXT("GLTFExports");
  FString OutputFilename = TEXT("full_scene_auto_gltf_materials.glb");
  FString ReportFilename = TEXT("full_scene_auto_gltf_materials_report.json");

  FGLBActorConfig Actors;
  FGLBTextureConfig Textures;
  FGLBMaterialConfig Materials;
};

struct FGLBExportReport
{
  bool bSuccess = false;
  FString OutputFile;
  FString FailureReason;
  int32 ActorsScanned = 0;
  int32 ActorsSkipped = 0;
  int32 ActorsExported = 0;
  int32 MaterialSlotsReplaced = 0;
  int32 GeneratedMaterialsCreated = 0;
  int32 GeneratedMaterialsReused = 0;
  TArray<FString> MaterialsWithoutBaseColor;
  TArray<FString> Suggestions;
  TArray<FString> Warnings;
  TArray<FString> Errors;

  bool WriteJson(const FString& ReportPath) const;
};
