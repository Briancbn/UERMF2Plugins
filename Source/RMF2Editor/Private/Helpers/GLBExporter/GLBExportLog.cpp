#include "Helpers/GLBExporter/GLBExportLog.h"

#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY(LogRMF2GLBExporter);

namespace
{
TArray<TSharedPtr<FJsonValue>> ToJsonArray(const TArray<FString>& Values)
{
  TArray<TSharedPtr<FJsonValue>> Result;
  Result.Reserve(Values.Num());

  for (const FString& Value : Values)
  {
    Result.Add(MakeShared<FJsonValueString>(Value));
  }

  return Result;
}
} // namespace

bool FGLBExportReport::WriteJson(const FString& ReportPath) const
{
  TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  Root->SetStringField(
      TEXT("status"),
      bSuccess ? TEXT("complete") : TEXT("failed")
  );
  Root->SetStringField(TEXT("output_file"), OutputFile);
  Root->SetStringField(TEXT("failure_reason"), FailureReason);
  Root->SetNumberField(TEXT("actors_scanned"), ActorsScanned);
  Root->SetNumberField(TEXT("actors_skipped"), ActorsSkipped);
  Root->SetNumberField(TEXT("actors_exported"), ActorsExported);
  Root->SetNumberField(TEXT("material_slots_replaced"), MaterialSlotsReplaced);
  Root->SetNumberField(
      TEXT("generated_materials_created"),
      GeneratedMaterialsCreated
  );
  Root->SetNumberField(
      TEXT("generated_materials_reused"),
      GeneratedMaterialsReused
  );
  Root->SetArrayField(
      TEXT("materials_without_obvious_base_color"),
      ToJsonArray(MaterialsWithoutBaseColor)
  );
  Root->SetArrayField(TEXT("suggestions"), ToJsonArray(Suggestions));
  Root->SetArrayField(TEXT("warnings"), ToJsonArray(Warnings));
  Root->SetArrayField(TEXT("errors"), ToJsonArray(Errors));

  FString Json;
  const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Json);
  if (!FJsonSerializer::Serialize(Root, Writer))
  {
    UE_LOG(
        LogRMF2GLBExporter,
        Error,
        TEXT("Failed to serialize export report: %s"),
        *ReportPath
    );
    return false;
  }

  if (!FFileHelper::SaveStringToFile(Json, *ReportPath))
  {
    UE_LOG(
        LogRMF2GLBExporter,
        Error,
        TEXT("Failed to write export report: %s"),
        *ReportPath
    );
    return false;
  }

  UE_LOG(
      LogRMF2GLBExporter,
      Display,
      TEXT("Export report written to: %s"),
      *ReportPath
  );
  return true;
}
