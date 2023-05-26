// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "AxisType.generated.h"

UENUM(BlueprintType, meta = (ScriptName = "UINavAxisType"))
enum class EAxisType : uint8
{
	None UMETA(DisplayName = "None"),
	Positive UMETA(DisplayName = "Positive"),
	Negative UMETA(DisplayName = "Negative")
};