#include "Helpers/GLBExporter/GLBMaterials.h"

#include "AssetToolsModule.h"
#include "EditorAssetLibrary.h"
#include "Factories/MaterialFactoryNew.h"
#include "GameFramework/Actor.h"
#include "Helpers/GLBExporter/GLBExportLog.h"
#include "Helpers/GLBExporter/GLBTextures.h"
#include "IAssetTools.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialInterface.h"
#include "Modules/ModuleManager.h"

FGLBMaterialBuilder::FGLBMaterialBuilder(const FGLBMaterialConfig& InConfig)
    : Config(InConfig)
{
}

FString FGLBMaterialBuilder::SanitizeAssetName(const FString& Name)
{
  FString Result;
  Result.Reserve(Name.Len());

  bool bPreviousWasUnderscore = false;
  for (const TCHAR Character : Name)
  {
    const bool bAllowed = FChar::IsAlnum(Character) || Character == TEXT('_');
    const TCHAR Output = bAllowed ? Character : TEXT('_');
    if (Output == TEXT('_') && bPreviousWasUnderscore)
    {
      continue;
    }

    Result.AppendChar(Output);
    bPreviousWasUnderscore = Output == TEXT('_');
  }

  while (Result.StartsWith(TEXT("_")))
  {
    Result.RemoveAt(0, 1, EAllowShrinking::No);
  }
  while (Result.EndsWith(TEXT("_")))
  {
    Result.RemoveAt(Result.Len() - 1, 1, EAllowShrinking::No);
  }
  return Result.IsEmpty() ? TEXT("Unnamed") : Result;
}

FString FGLBMaterialBuilder::MakeGeneratedName(
    const UMaterialInterface* OriginalMaterial,
    const AActor* Actor,
    const UActorComponent* Component,
    const int32 SlotIndex
) const
{
  const FString OriginalName =
      IsValid(OriginalMaterial) ? SanitizeAssetName(OriginalMaterial->GetName())
                                : TEXT("MissingMaterial");

  if (Config.bUniqueMaterialPerSlot && IsValid(Actor) && IsValid(Component))
  {
    return FString::Printf(
        TEXT("M_GLTF_Auto_%s_%s_%d_%s"),
        *SanitizeAssetName(Actor->GetActorLabel()),
        *SanitizeAssetName(Component->GetName()),
        SlotIndex,
        *OriginalName
    );
  }

  return FString::Printf(TEXT("M_GLTF_Auto_%s"), *OriginalName);
}

UMaterialExpression* FGLBMaterialBuilder::CreateTextureSample(
    UMaterial* Material,
    UTexture* Texture,
    const int32 X,
    const int32 Y,
    const bool bNormalMap
) const
{
  UMaterialExpressionTextureSample* Expression =
      Cast<UMaterialExpressionTextureSample>(
          UMaterialEditingLibrary::CreateMaterialExpression(
              Material,
              UMaterialExpressionTextureSample::StaticClass(),
              X,
              Y
          )
      );
  if (!Expression)
  {
    return nullptr;
  }

  Expression->Texture = Texture;
  if (bNormalMap)
  {
    Expression->SamplerType = SAMPLERTYPE_Normal;
  }
  return Expression;
}

UMaterialExpression* FGLBMaterialBuilder::CreateScalar(
    UMaterial* Material,
    const float Value,
    const int32 X,
    const int32 Y
) const
{
  UMaterialExpressionConstant* Expression = Cast<UMaterialExpressionConstant>(
      UMaterialEditingLibrary::CreateMaterialExpression(
          Material,
          UMaterialExpressionConstant::StaticClass(),
          X,
          Y
      )
  );
  if (Expression)
  {
    Expression->R = Value;
  }
  return Expression;
}

UMaterialExpression* FGLBMaterialBuilder::CreateColor(
    UMaterial* Material,
    const FLinearColor& Color,
    const int32 X,
    const int32 Y
) const
{
  UMaterialExpressionConstant3Vector* Expression =
      Cast<UMaterialExpressionConstant3Vector>(
          UMaterialEditingLibrary::CreateMaterialExpression(
              Material,
              UMaterialExpressionConstant3Vector::StaticClass(),
              X,
              Y
          )
      );
  if (Expression)
  {
    Expression->Constant = Color;
  }
  return Expression;
}

void FGLBMaterialBuilder::CopyBasicProperties(
    const UMaterialInterface* Original,
    UMaterial* Generated
) const
{
  if (!IsValid(Original) || !IsValid(Generated))
  {
    return;
  }

  const UMaterial* OriginalBase = Original->GetMaterial();
  if (!IsValid(OriginalBase))
  {
    return;
  }

  Generated->BlendMode = OriginalBase->BlendMode;
  Generated->TwoSided = OriginalBase->TwoSided;
  Generated->OpacityMaskClipValue = OriginalBase->OpacityMaskClipValue;
  Generated->SetShadingModel(
      OriginalBase->GetShadingModels().GetFirstShadingModel()
  );
}

UMaterial* FGLBMaterialBuilder::CreateOrReuse(
    UMaterialInterface* OriginalMaterial,
    const FString& GeneratedName,
    const FGLBTextureInfo& TextureInfo,
    bool& bOutCreated
) const
{
  bOutCreated = false;
  if (!IsValid(OriginalMaterial))
  {
    return nullptr;
  }

  if (!UEditorAssetLibrary::DoesDirectoryExist(Config.GeneratedFolder))
  {
    UEditorAssetLibrary::MakeDirectory(Config.GeneratedFolder);
  }

  const FString ObjectPath = FString::Printf(
      TEXT("%s/%s.%s"),
      *Config.GeneratedFolder,
      *GeneratedName,
      *GeneratedName
  );

  if (UEditorAssetLibrary::DoesAssetExist(ObjectPath))
  {
    if (Config.bReuseGeneratedMaterials)
    {
      if (UMaterial* Existing = LoadObject<UMaterial>(nullptr, *ObjectPath))
      {
        UE_LOG(
            LogRMF2GLBExporter,
            Display,
            TEXT("Reusing generated material: %s"),
            *ObjectPath
        );
        return Existing;
      }
    }

    if (!UEditorAssetLibrary::DeleteAsset(ObjectPath))
    {
      UE_LOG(
          LogRMF2GLBExporter,
          Error,
          TEXT("Could not replace generated material: %s"),
          *ObjectPath
      );
      return nullptr;
    }
  }

  IAssetTools& AssetTools =
      FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"))
          .Get();
  UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
  UMaterial* Material = Cast<UMaterial>(AssetTools.CreateAsset(
      GeneratedName,
      Config.GeneratedFolder,
      UMaterial::StaticClass(),
      Factory
  ));
  if (!Material)
  {
    UE_LOG(
        LogRMF2GLBExporter,
        Error,
        TEXT("Failed to create generated material: %s"),
        *ObjectPath
    );
    return nullptr;
  }

  CopyBasicProperties(OriginalMaterial, Material);

  UMaterialExpression* BaseColor =
      TextureInfo.BaseColor
          ? CreateTextureSample(Material, TextureInfo.BaseColor, -700, -250)
          : CreateColor(Material, FLinearColor(0.5f, 0.5f, 0.5f), -700, -250);
  if (BaseColor)
  {
    UMaterialEditingLibrary::ConnectMaterialProperty(
        BaseColor,
        TextureInfo.BaseColor ? TEXT("RGB") : TEXT(""),
        MP_BaseColor
    );
  }

  if (TextureInfo.Normal)
  {
    if (UMaterialExpression* Normal =
            CreateTextureSample(Material, TextureInfo.Normal, -700, 50, true))
    {
      UMaterialEditingLibrary::ConnectMaterialProperty(
          Normal,
          TEXT("RGB"),
          MP_Normal
      );
    }
  }

  if (TextureInfo.ORM)
  {
    if (UMaterialExpression* ORM =
            CreateTextureSample(Material, TextureInfo.ORM, -700, 350))
    {
      UMaterialEditingLibrary::ConnectMaterialProperty(
          ORM,
          TEXT("R"),
          MP_AmbientOcclusion
      );
      UMaterialEditingLibrary::ConnectMaterialProperty(
          ORM,
          TEXT("G"),
          MP_Roughness
      );
      UMaterialEditingLibrary::ConnectMaterialProperty(
          ORM,
          TEXT("B"),
          MP_Metallic
      );
    }
  }
  else
  {
    UMaterialExpression* Roughness =
        TextureInfo.Roughness
            ? CreateTextureSample(Material, TextureInfo.Roughness, -700, 350)
            : CreateScalar(Material, 0.5f, -700, 350);
    if (Roughness)
    {
      UMaterialEditingLibrary::ConnectMaterialProperty(
          Roughness,
          TextureInfo.Roughness ? TEXT("R") : TEXT(""),
          MP_Roughness
      );
    }

    UMaterialExpression* Metallic =
        TextureInfo.Metallic
            ? CreateTextureSample(Material, TextureInfo.Metallic, -700, 500)
            : CreateScalar(Material, 0.0f, -700, 500);
    if (Metallic)
    {
      UMaterialEditingLibrary::ConnectMaterialProperty(
          Metallic,
          TextureInfo.Metallic ? TEXT("R") : TEXT(""),
          MP_Metallic
      );
    }

    if (TextureInfo.AmbientOcclusion)
    {
      if (UMaterialExpression* AO = CreateTextureSample(
              Material,
              TextureInfo.AmbientOcclusion,
              -700,
              650
          ))
      {
        UMaterialEditingLibrary::ConnectMaterialProperty(
            AO,
            TEXT("R"),
            MP_AmbientOcclusion
        );
      }
    }
  }

  const UMaterial* OriginalBase = OriginalMaterial->GetMaterial();
  const EBlendMode BlendMode =
      OriginalBase ? OriginalBase->BlendMode : BLEND_Opaque;
  if (BlendMode == BLEND_Masked || BlendMode == BLEND_Translucent)
  {
    UTexture* OpacityTexture =
        TextureInfo.Opacity ? TextureInfo.Opacity : TextureInfo.BaseColor;
    if (OpacityTexture)
    {
      if (UMaterialExpression* Opacity =
              CreateTextureSample(Material, OpacityTexture, -700, 800))
      {
        const bool bUseBaseColorAlpha =
            !TextureInfo.Opacity && TextureInfo.BaseColor;
        UMaterialEditingLibrary::ConnectMaterialProperty(
            Opacity,
            bUseBaseColorAlpha ? TEXT("A") : TEXT("R"),
            BlendMode == BLEND_Masked ? MP_OpacityMask : MP_Opacity
        );
      }
    }
  }

  UMaterialEditingLibrary::LayoutMaterialExpressions(Material);
  UMaterialEditingLibrary::RecompileMaterial(Material);
  UEditorAssetLibrary::SaveLoadedAsset(Material);

  bOutCreated = true;
  UE_LOG(
      LogRMF2GLBExporter,
      Display,
      TEXT("Created generated material: %s"),
      *ObjectPath
  );
  return Material;
}
