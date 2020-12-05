// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once
#include "NavigationEvent.generated.h"

UENUM(BlueprintType)
enum class ENavigationEvent : uint8
{
	OnSelect UMETA(DisplayName = "OnSelect"),
	OnNavigate UMETA(DisplayName = "OnNavigate"),
};