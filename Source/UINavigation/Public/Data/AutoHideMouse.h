// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "AutoHideMouse.generated.h"

UENUM(BlueprintType)
enum class EAutoHideMouse : uint8
{
	Never,
	Gamepad,
	GamepadAndKeyboard
};