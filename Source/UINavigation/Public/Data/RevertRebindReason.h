// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#pragma once
#include "RevertRebindReason.generated.h"

UENUM(BlueprintType)
enum class ERevertRebindReason : uint8
{
	RestrictionMismatch UMETA(DisplayName = "Restriction Mismatch"),
	BlacklistedKey UMETA(DisplayName = "Blacklisted Key"),
	UsedBySameActionGroup UMETA(DisplayName = "Used By Same Action Group")
};