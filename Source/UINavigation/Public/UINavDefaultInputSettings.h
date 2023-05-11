// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UObject/NoExportTypes.h"
#include "GameFramework/PlayerInput.h"
#include "InputMappingContext.h"
#include "Data/UINavEnhancedInputActions.h"
#include "Data/UINavEnhancedActionKeyMapping.h"
#include "UINavDefaultInputSettings.generated.h"

USTRUCT(BlueprintType)
struct FInputMappingArray
{
	GENERATED_BODY()

	FInputMappingArray() {}

	FInputMappingArray(const TArray<FEnhancedActionKeyMapping>& InputMappings)
	: DefaultInputMappings(InputMappings) {}

	UPROPERTY()
	TArray<FUINavEnhancedActionKeyMapping> DefaultInputMappings;
};

/**
 *
 */
UCLASS(config = UINavDefaultInputSettings)
class UINAVIGATION_API UUINavDefaultInputSettings : public UObject
{
	GENERATED_BODY()

	UUINavDefaultInputSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {}

public:
	// A map for each Input Context in your game and its respective Default Input Context Mappings
	UPROPERTY(config)
	TMap<TSoftObjectPtr<UInputMappingContext>, FInputMappingArray> DefaultEnhancedInputMappings;
};
