#pragma once

#include "CoreMinimal.h"

class UGLTFExportOptions;

class FGLBExportOptionsFactory
{
public:
  static UGLTFExportOptions* Create(UObject* Outer);
};