// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once
#include "NavigationDirection.generated.h"

UENUM(BlueprintType, meta = (ScriptName = "UINavNavigationDirection"))
enum class ENavigationDirection : uint8
{
	None,
	Up,
	Down,
	Left,
	Right
};