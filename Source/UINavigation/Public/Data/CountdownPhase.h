// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once
#include "CountdownPhase.generated.h"

UENUM(BlueprintType)
enum class ECountdownPhase : uint8
{
	None UMETA(DisplayName = "None"),
	First UMETA(DisplayName = "First"),
	Looping UMETA(DisplayName = "Looping")
};