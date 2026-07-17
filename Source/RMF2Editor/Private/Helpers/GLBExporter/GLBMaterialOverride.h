#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObject.h"

class UMaterialInterface;
class UMeshComponent;

struct FGLBMaterialOverride
{
  TWeakObjectPtr<UMeshComponent> Component;
  UMaterialInterface* OriginalMaterial = nullptr;
  int32 SlotIndex = INDEX_NONE;
};

class FScopedMaterialOverrides final : public FGCObject
{
public:
  FScopedMaterialOverrides() = default;
  ~FScopedMaterialOverrides();

  FScopedMaterialOverrides(const FScopedMaterialOverrides&) = delete;
  FScopedMaterialOverrides& operator=(const FScopedMaterialOverrides&) = delete;

  bool Override(
      UMeshComponent* Component,
      int32 SlotIndex,
      UMaterialInterface* GeneratedMaterial
  );

  void Restore();
  int32 Num() const { return Records.Num(); }

  virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
  virtual FString GetReferencerName() const override;

private:
  TArray<FGLBMaterialOverride> Records;
};
