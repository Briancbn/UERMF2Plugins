#pragma once

#include "CoreMinimal.h"

class UMaterial;
class UMaterialExpression;
class UMaterialInterface;
class UTexture;
class AActor;
class UActorComponent;

struct FGLBMaterialConfig;
struct FGLBTextureInfo;

class FGLBMaterialBuilder
{
public:
  explicit FGLBMaterialBuilder(const FGLBMaterialConfig& InConfig);

  UMaterial* CreateOrReuse(
      UMaterialInterface* OriginalMaterial,
      const FString& GeneratedName,
      const FGLBTextureInfo& TextureInfo,
      bool& bOutCreated
  ) const;

  FString MakeGeneratedName(
      const UMaterialInterface* OriginalMaterial,
      const AActor* Actor,
      const UActorComponent* Component,
      int32 SlotIndex
  ) const;

private:
  UMaterialExpression* CreateTextureSample(
      UMaterial* Material,
      UTexture* Texture,
      int32 X,
      int32 Y,
      bool bNormalMap = false
  ) const;

  UMaterialExpression*
  CreateScalar(UMaterial* Material, float Value, int32 X, int32 Y) const;

  UMaterialExpression*
  CreateColor(UMaterial* Material, const FLinearColor& Color, int32 X, int32 Y)
      const;

  void CopyBasicProperties(
      const UMaterialInterface* Original,
      UMaterial* Generated
  ) const;

  static FString SanitizeAssetName(const FString& Name);

  FGLBMaterialConfig Config;
};
