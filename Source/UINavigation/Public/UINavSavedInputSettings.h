// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UObject/Object.h"
#include "InputMappingContext.h"
#include "Data/InputMappingArray.h"
#include "UINavSavedInputSettings.generated.h"

/**
 *
 */
UCLASS(config = UserUINavSavedInputSettings)
class UINAVIGATION_API UUINavSavedInputSettings : public UObject
{
	GENERATED_BODY()

	UUINavSavedInputSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {}

public:
	// A map for each Input Context that's been overriden in your game and its respective Input Context Mappings
	UPROPERTY(config)
	TMap<TSoftObjectPtr<UInputMappingContext>, FInputMappingArray> SavedEnhancedInputMappings;

	UPROPERTY(config)
	uint8 InputVersion = 0;
};
