// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "CountdownPhase.generated.h"

UENUM(BlueprintType, meta = (ScriptName = "UINavCountdownPhase"))
enum class ECountdownPhase : uint8
{
	None UMETA(DisplayName = "None"),
	First UMETA(DisplayName = "First"),
	Looping UMETA(DisplayName = "Looping")
};