// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "ReceiveInputType.generated.h"

UENUM(BlueprintType, meta = (ScriptName = "UINavReceiveInputType"))
enum class EReceiveInputType : uint8
{
	None UMETA(DisplayName = "None"),
	Action UMETA(DisplayName = "Action"),
	Axis UMETA(DisplayName = "Axis")
};