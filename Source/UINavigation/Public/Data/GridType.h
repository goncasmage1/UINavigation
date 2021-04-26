// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once
#include "GridType.generated.h"

UENUM(BlueprintType, meta = (ScriptName = "UINavGridType"))
enum class EGridType : uint8
{
	Horizontal UMETA(DisplayName = "Horizontal"),
	Vertical UMETA(DisplayName = "Vertical"),
	Grid2D UMETA(DisplayName = "Grid2D")
};