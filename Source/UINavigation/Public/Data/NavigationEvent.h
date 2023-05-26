// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "NavigationEvent.generated.h"

UENUM(BlueprintType, meta = (ScriptName = "UINavNavigationEvent"))
enum class ENavigationEvent : uint8
{
	OnSelect UMETA(DisplayName = "OnSelect"),
	OnNavigate UMETA(DisplayName = "OnNavigate"),
};