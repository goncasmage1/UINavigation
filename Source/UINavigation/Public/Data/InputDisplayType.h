// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "InputDisplayType.generated.h"

UENUM(BlueprintType)
enum class EInputDisplayType : uint8
{
	Icon,
	Text,
	Both
};