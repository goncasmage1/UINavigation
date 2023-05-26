// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "TargetColumn.generated.h"

UENUM(BlueprintType, meta = (ScriptName = "UINavTargetColumn"))
enum class ETargetColumn : uint8
{
	Left UMETA(DisplayName = "Left"),
	Middle UMETA(DisplayName = "Middle"),
	Right UMETA(DisplayName = "Right"),
};