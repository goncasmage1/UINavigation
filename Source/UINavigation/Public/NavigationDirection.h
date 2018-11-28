#pragma once

#include "Engine.h"

UENUM(BlueprintType)
enum class ENavigationDirection : uint8
{
	None,
	Up,
	Down,
	Left,
	Right
};