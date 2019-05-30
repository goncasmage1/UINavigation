// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once
#include "InputRestriction.generated.h"

UENUM(BlueprintType)
enum class EInputRestriction : uint8
{
	None UMETA(DisplayName = "None"),
	Keyboard UMETA(DisplayName = "Keyboard"),
	Mouse UMETA(DisplayName = "Mouse"),
	Keyboard_Mouse UMETA(DisplayName = "Keyboard and Mouse"),
	Gamepad UMETA(DisplayName = "Gamepad"),
};