#include "Helpers/GLBExporter/GLBTextures.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/Texture.h"
#include "Helpers/GLBExporter/GLBExportLog.h"
#include "Materials/MaterialInterface.h"
#include "Modules/ModuleManager.h"

FGLBTextureClassifier::FGLBTextureClassifier(const FGLBTextureConfig& InConfig)
    : Config(InConfig)
{
}

bool FGLBTextureClassifier::NameMatches(
    const FString& Name,
    const TArray<FString>& Keywords
)
{
  return Keywords.ContainsByPredicate(
      [&Name](const FString& Keyword)
      { return Name.Contains(Keyword, ESearchCase::IgnoreCase); }
  );
}

TArray<UTexture*> FGLBTextureClassifier::FindTextureDependencies(
    const UMaterialInterface* Material
) const
{
  TArray<UTexture*> Textures;
  if (!IsValid(Material))
  {
    return Textures;
  }

  FAssetRegistryModule& AssetRegistryModule =
      FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
          TEXT("AssetRegistry")
      );
  IAssetRegistry& Registry = AssetRegistryModule.Get();

  TArray<FName> Dependencies;
  Registry.GetDependencies(
      Material->GetOutermost()->GetFName(),
      Dependencies,
      UE::AssetRegistry::EDependencyCategory::Package,
      UE::AssetRegistry::FDependencyQuery()
  );

  TSet<FString> SeenObjectPaths;
  for (const FName Dependency : Dependencies)
  {
    TArray<FAssetData> Assets;
    Registry.GetAssetsByPackageName(Dependency, Assets);

    for (const FAssetData& Asset : Assets)
    {
      UTexture* Texture = Cast<UTexture>(Asset.GetAsset());
      if (!IsValid(Texture))
      {
        continue;
      }

      const FString ObjectPath = Texture->GetPathName();
      if (!SeenObjectPaths.Contains(ObjectPath))
      {
        SeenObjectPaths.Add(ObjectPath);
        Textures.Add(Texture);
      }
    }
  }

  return Textures;
}

FGLBTextureInfo
FGLBTextureClassifier::Classify(const TArray<UTexture*>& Textures) const
{
  FGLBTextureInfo Result;

  for (UTexture* Texture : Textures)
  {
    if (!IsValid(Texture))
    {
      continue;
    }

    Result.ReferencedTexturePaths.Add(Texture->GetPathName());
    const FString Name = Texture->GetName();

    // Technical maps must be checked before the broad "color" keyword.
    if (!Result.Normal && NameMatches(Name, Config.NormalKeywords))
    {
      Result.Normal = Texture;
      continue;
    }

    if (!Result.ORM && NameMatches(Name, Config.ORMKeywords))
    {
      Result.ORM = Texture;
      continue;
    }

    if (!Result.Roughness && NameMatches(Name, Config.RoughnessKeywords))
    {
      Result.Roughness = Texture;
      continue;
    }

    if (!Result.Metallic && NameMatches(Name, Config.MetallicKeywords))
    {
      Result.Metallic = Texture;
      continue;
    }

    if (!Result.AmbientOcclusion && NameMatches(Name, Config.AOKeywords))
    {
      Result.AmbientOcclusion = Texture;
      continue;
    }

    if (!Result.Opacity && NameMatches(Name, Config.OpacityKeywords))
    {
      Result.Opacity = Texture;
      continue;
    }

    if (!Result.BaseColor && NameMatches(Name, Config.BaseColorKeywords) &&
        !NameMatches(Name, Config.NormalKeywords) &&
        !NameMatches(Name, Config.ORMKeywords) &&
        !NameMatches(Name, Config.OpacityKeywords))
    {
      Result.BaseColor = Texture;
    }
  }

  if (!Result.BaseColor && Textures.Num() == 1 && IsValid(Textures[0]))
  {
    Result.BaseColor = Textures[0];
  }

  return Result;
}
