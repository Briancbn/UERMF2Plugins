#include "Helpers/GLBExporter/GLBExportHelper.h"

#include "Components/MeshComponent.h"
#include "Editor.h"
#include "Exporters/GLTFExporter.h"
#include "HAL/FileManager.h"
#include "Helpers/GLBExporter/GLBActors.h"
#include "Helpers/GLBExporter/GLBExportLog.h"
#include "Helpers/GLBExporter/GLBExportOptions.h"
#include "Helpers/GLBExporter/GLBMaterialOverride.h"
#include "Helpers/GLBExporter/GLBMaterials.h"
#include "Helpers/GLBExporter/GLBTextures.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Misc/Paths.h"
#include "Options/GLTFExportOptions.h"
#include "Subsystems/EditorActorSubsystem.h"

bool FGLBExportHelper::Run()
{
  const FGLBExportConfig Config;
  FGLBExportReport Report;

  const FString ExportDirectory =
      FPaths::Combine(FPaths::ProjectSavedDir(), Config.ExportDirectoryName);
  const FString OutputPath =
      FPaths::Combine(ExportDirectory, Config.OutputFilename);
  const FString ReportPath =
      FPaths::Combine(ExportDirectory, Config.ReportFilename);
  Report.OutputFile = OutputPath;

  if (!IFileManager::Get().MakeDirectory(*ExportDirectory, true))
  {
    Report.FailureReason = TEXT("Could not create the export directory.");
    UE_LOG(
        LogRMF2GLBExporter,
        Error,
        TEXT("Could not create export directory: %s"),
        *ExportDirectory
    );
    return false;
  }

  UEditorActorSubsystem* ActorSubsystem =
      GEditor ? GEditor->GetEditorSubsystem<UEditorActorSubsystem>() : nullptr;
  UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
  if (!ActorSubsystem || !World)
  {
    Report.FailureReason = TEXT("The editor world is unavailable.");
    Report.WriteJson(ReportPath);
    UE_LOG(
        LogRMF2GLBExporter,
        Error,
        TEXT("Editor actor subsystem or world unavailable.")
    );
    return false;
  }

  FGLBActorFilter ActorFilter(Config.Actors);
  FGLBTextureClassifier TextureClassifier(Config.Textures);
  FGLBMaterialBuilder MaterialBuilder(Config.Materials);
  FScopedMaterialOverrides MaterialOverrides;

  TSet<AActor*> ActorsToExport;
  TMap<FString, UMaterial*> MaterialCache;

  const TArray<AActor*> Actors = ActorSubsystem->GetAllLevelActors();
  Report.ActorsScanned = Actors.Num();
  UE_LOG(
      LogRMF2GLBExporter,
      Display,
      TEXT("Scanning %d actors..."),
      Actors.Num()
  );

  for (AActor* Actor : Actors)
  {
    if (ActorFilter.ShouldExclude(Actor))
    {
      ++Report.ActorsSkipped;
      continue;
    }

    TArray<UMeshComponent*> MeshComponents;
    ActorFilter.GetRenderableMeshComponents(Actor, MeshComponents);
    if (Config.Actors.bExportOnlyMeshActors && MeshComponents.IsEmpty())
    {
      continue;
    }

    bool bActorHasRenderableMesh = false;
    for (UMeshComponent* Component : MeshComponents)
    {
      if (!IsValid(Component))
      {
        continue;
      }

      bActorHasRenderableMesh = true;
      for (int32 SlotIndex = 0; SlotIndex < Component->GetNumMaterials();
           ++SlotIndex)
      {
        UMaterialInterface* OriginalMaterial =
            Component->GetMaterial(SlotIndex);
        if (!IsValid(OriginalMaterial))
        {
          continue;
        }

        FString CacheKey = OriginalMaterial->GetPathName();
        if (Config.Materials.bUniqueMaterialPerSlot)
        {
          CacheKey = FString::Printf(
              TEXT("%s::%s::%d::%s"),
              *Actor->GetPathName(),
              *Component->GetPathName(),
              SlotIndex,
              *OriginalMaterial->GetPathName()
          );
        }

        UMaterial* GeneratedMaterial = MaterialCache.FindRef(CacheKey);
        if (!IsValid(GeneratedMaterial))
        {
          const TArray<UTexture*> TextureDependencies =
              TextureClassifier.FindTextureDependencies(OriginalMaterial);
          const FGLBTextureInfo TextureInfo =
              TextureClassifier.Classify(TextureDependencies);

          if (!TextureInfo.BaseColor)
          {
            Report.MaterialsWithoutBaseColor.AddUnique(
                OriginalMaterial->GetPathName()
            );
          }

          const FString GeneratedName = MaterialBuilder.MakeGeneratedName(
              OriginalMaterial,
              Actor,
              Component,
              SlotIndex
          );
          bool bCreated = false;
          GeneratedMaterial = MaterialBuilder.CreateOrReuse(
              OriginalMaterial,
              GeneratedName,
              TextureInfo,
              bCreated
          );
          if (!IsValid(GeneratedMaterial))
          {
            Report.Errors.Add(
                FString::Printf(
                    TEXT("Failed to generate a material for %s"),
                    *OriginalMaterial->GetPathName()
                )
            );
            continue;
          }

          MaterialCache.Add(CacheKey, GeneratedMaterial);
          if (bCreated)
          {
            ++Report.GeneratedMaterialsCreated;
          }
          else
          {
            ++Report.GeneratedMaterialsReused;
          }
        }

        if (MaterialOverrides.Override(Component, SlotIndex, GeneratedMaterial))
        {
          ++Report.MaterialSlotsReplaced;
        }
      }
    }

    if (bActorHasRenderableMesh)
    {
      ActorsToExport.Add(Actor);
    }
  }

  Report.ActorsExported = ActorsToExport.Num();
  if (ActorsToExport.IsEmpty())
  {
    Report.FailureReason = TEXT("No mesh actors were found to export.");
    Report.WriteJson(ReportPath);
    UE_LOG(LogRMF2GLBExporter, Error, TEXT("No mesh actors found to export."));
    return false;
  }

  UGLTFExportOptions* Options =
      FGLBExportOptionsFactory::Create(GetTransientPackage());
  FGLTFExportMessages Messages;

  UE_LOG(
      LogRMF2GLBExporter,
      Display,
      TEXT("Exporting %d actors to: %s"),
      ActorsToExport.Num(),
      *OutputPath
  );

  Report.bSuccess = UGLTFExporter::ExportToGLTF(
      World,
      OutputPath,
      Options,
      ActorsToExport,
      Messages
  );
  Report.Suggestions = Messages.Suggestions;
  Report.Warnings = Messages.Warnings;
  Report.Errors.Append(Messages.Errors);
  if (!Report.bSuccess)
  {
    Report.FailureReason = TEXT("UGLTFExporter reported a failure.");
  }

  MaterialOverrides.Restore();
  Report.WriteJson(ReportPath);

  if (Report.bSuccess)
  {
    UE_LOG(LogRMF2GLBExporter, Display, TEXT("Native GLB export completed."));
  }
  else
  {
    UE_LOG(LogRMF2GLBExporter, Error, TEXT("Native GLB export failed."));
  }

  return Report.bSuccess;
}
