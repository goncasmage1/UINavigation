// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "SelectorPosition.generated.h"

UENUM(BlueprintType, meta = (ScriptName = "UINavSelectorPosition"))
enum class ESelectorPosition : uint8
{
	Position_Center UMETA(DisplayName = "Center"),
	Position_Top UMETA(DisplayName = "Top"),
	Position_Bottom UMETA(DisplayName = "Bottom"),
	Position_Left UMETA(DisplayName = "Left"),
	Position_Right UMETA(DisplayName = "Right"),
	Position_Top_Right UMETA(DisplayName = "Top Right"),
	Position_Top_Left UMETA(DisplayName = "Top Left"),
	Position_Bottom_Right UMETA(DisplayName = "Bottom Right"),
	Position_Bottom_Left UMETA(DisplayName = "Bottom Left")
};