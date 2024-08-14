// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UObject/Object.h"
#include "InputMappingContext.h"
#include "Data/InputMappingArray.h"
#include "UINavDefaultInputSettings.generated.h"

/**
 *
 */
UCLASS(config = UserUINavDefaultInputSettings)
class UINAVIGATION_API UUINavDefaultInputSettings : public UObject
{
	GENERATED_BODY()

	UUINavDefaultInputSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {}

public:
	// A map for each Input Context in your game and its respective Default Input Context Mappings
	UPROPERTY(config)
	TMap<TSoftObjectPtr<UInputMappingContext>, FInputMappingArray> DefaultEnhancedInputMappings;

	UPROPERTY(config)
	uint8 InputVersion = 0;
};
