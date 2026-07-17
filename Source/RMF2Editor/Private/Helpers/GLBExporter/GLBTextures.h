#pragma once

#include "CoreMinimal.h"

class UMaterialInterface;
class UTexture;
struct FGLBTextureConfig;

struct FGLBTextureInfo
{
  UTexture* BaseColor = nullptr;
  UTexture* Normal = nullptr;
  UTexture* ORM = nullptr;
  UTexture* Roughness = nullptr;
  UTexture* Metallic = nullptr;
  UTexture* AmbientOcclusion = nullptr;
  UTexture* Opacity = nullptr;

  TArray<FString> ReferencedTexturePaths;
};

class FGLBTextureClassifier
{
public:
  explicit FGLBTextureClassifier(const FGLBTextureConfig& InConfig);

  TArray<UTexture*> FindTextureDependencies(const UMaterialInterface* Material
  ) const;

  FGLBTextureInfo Classify(const TArray<UTexture*>& Textures) const;

private:
  static bool NameMatches(const FString& Name, const TArray<FString>& Keywords);

  FGLBTextureConfig Config;
};
