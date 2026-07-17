#include "Helpers/GLBExporter/GLBMaterialOverride.h"

#include "Components/MeshComponent.h"
#include "Materials/MaterialInterface.h"

FScopedMaterialOverrides::~FScopedMaterialOverrides() { Restore(); }

bool FScopedMaterialOverrides::Override(
    UMeshComponent* Component,
    const int32 SlotIndex,
    UMaterialInterface* GeneratedMaterial
)
{
  if (!IsValid(Component) || !IsValid(GeneratedMaterial) || SlotIndex < 0 ||
      SlotIndex >= Component->GetNumMaterials())
  {
    return false;
  }

  const bool bAlreadyOverridden = Records.ContainsByPredicate(
      [Component, SlotIndex](const FGLBMaterialOverride& Record) {
        return Record.Component.Get() == Component &&
               Record.SlotIndex == SlotIndex;
      }
  );
  if (bAlreadyOverridden)
  {
    return false;
  }

  FGLBMaterialOverride& Record = Records.AddDefaulted_GetRef();
  Record.Component = Component;
  Record.OriginalMaterial = Component->GetMaterial(SlotIndex);
  Record.SlotIndex = SlotIndex;

  Component->SetMaterial(SlotIndex, GeneratedMaterial);
  return true;
}

void FScopedMaterialOverrides::Restore()
{
  for (int32 Index = Records.Num() - 1; Index >= 0; --Index)
  {
    const FGLBMaterialOverride& Record = Records[Index];
    if (UMeshComponent* Component = Record.Component.Get())
    {
      Component->SetMaterial(Record.SlotIndex, Record.OriginalMaterial);
    }
  }

  Records.Reset();
}

void FScopedMaterialOverrides::AddReferencedObjects(
    FReferenceCollector& Collector
)
{
  for (FGLBMaterialOverride& Record : Records)
  {
    Collector.AddReferencedObject(Record.OriginalMaterial);
  }
}

FString FScopedMaterialOverrides::GetReferencerName() const
{
  return TEXT("FScopedMaterialOverrides");
}
