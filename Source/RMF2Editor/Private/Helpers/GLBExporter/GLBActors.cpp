#include "Helpers/GLBExporter/GLBActors.h"

#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Helpers/GLBExporter/GLBExportLog.h"

FGLBActorFilter::FGLBActorFilter(const FGLBActorConfig& InConfig)
    : Config(InConfig)
{
}

bool FGLBActorFilter::ContainsExcludedText(
    const FString& Value,
    const TArray<FString>& Exclusions
)
{
  return Exclusions.ContainsByPredicate(
      [&Value](const FString& Exclusion)
      { return Value.Contains(Exclusion, ESearchCase::IgnoreCase); }
  );
}

bool FGLBActorFilter::ShouldExclude(const AActor* Actor) const
{
  if (!IsValid(Actor))
  {
    return true;
  }

  const FString ActorIdentity = FString::Printf(
      TEXT("%s %s"),
      *Actor->GetName(),
      *Actor->GetActorLabel()
  );

  if (ContainsExcludedText(ActorIdentity, Config.ExcludedNameParts))
  {
    return true;
  }

  if (ContainsExcludedText(
          Actor->GetClass()->GetName(),
          Config.ExcludedClassParts
      ))
  {
    return true;
  }

  return Config.bSkipHiddenActors && Actor->IsHiddenEd();
}

void FGLBActorFilter::GetRenderableMeshComponents(
    AActor* Actor,
    TArray<UMeshComponent*>& OutComponents
) const
{
  OutComponents.Reset();
  if (!IsValid(Actor))
  {
    return;
  }

  Actor->GetComponents<UMeshComponent>(OutComponents);
  OutComponents.RemoveAll(
      [](UMeshComponent* Component)
      {
        if (!IsValid(Component))
        {
          return true;
        }

        if (const UStaticMeshComponent* StaticMeshComponent =
                Cast<UStaticMeshComponent>(Component))
        {
          return StaticMeshComponent->GetStaticMesh() == nullptr;
        }

        if (const USkeletalMeshComponent* SkeletalMeshComponent =
                Cast<USkeletalMeshComponent>(Component))
        {
          return SkeletalMeshComponent->GetSkeletalMeshAsset() == nullptr;
        }

        return Component->GetNumMaterials() <= 0;
      }
  );
}
