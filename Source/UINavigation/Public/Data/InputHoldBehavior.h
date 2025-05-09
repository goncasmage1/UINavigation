// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "InputHoldBehavior.generated.h"

UENUM(BlueprintType)
enum class EInputHoldBehavior : uint8
{
	DontAllow,
	AllowIfHeld,
	Force
};