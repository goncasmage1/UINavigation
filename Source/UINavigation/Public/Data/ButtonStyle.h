// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#pragma once
#include "ButtonStyle.generated.h"

UENUM(BlueprintType)
enum class EButtonStyle : uint8
{
	None UMETA(DisplayName = "None"),
	Normal UMETA(DisplayName = "Normal"),
	Hovered UMETA(DisplayName = "Hovered"),
	Pressed UMETA(DisplayName = "Pressed")
};