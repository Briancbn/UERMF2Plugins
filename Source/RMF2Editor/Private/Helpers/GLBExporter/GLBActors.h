#pragma once

#include "CoreMinimal.h"

class AActor;
class UMeshComponent;
struct FGLBActorConfig;

class FGLBActorFilter
{
public:
  explicit FGLBActorFilter(const FGLBActorConfig& InConfig);

  bool ShouldExclude(const AActor* Actor) const;

  void GetRenderableMeshComponents(
      AActor* Actor,
      TArray<UMeshComponent*>& OutComponents
  ) const;

private:
  static bool
  ContainsExcludedText(const FString& Value, const TArray<FString>& Exclusions);

  FGLBActorConfig Config;
};
