// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "InputAction.h"
#include "Data/AxisType.h"
#include "InputContainerEnhancedActionData.generated.h"

UENUM(BlueprintType, meta = (ScriptName = "UINavAxis"))
enum class EInputAxis : uint8
{
	X UMETA(DisplayName = "X"),
	Y UMETA(DisplayName = "Y"),
	Z UMETA(DisplayName = "Z")
};
