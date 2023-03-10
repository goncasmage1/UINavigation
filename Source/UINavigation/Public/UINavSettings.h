// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UObject/NoExportTypes.h"
#include "GameFramework/PlayerInput.h"
#include "InputMappingContext.h"
#include "Data/UINavEnhancedInputActions.h"
#include "UINavSettings.generated.h"

USTRUCT(BlueprintType)
struct FInputMappingArray
{
	GENERATED_BODY()

	FInputMappingArray() {}

	FInputMappingArray(const TArray<FEnhancedActionKeyMapping>& InputMappings)
	: DefaultInputMappings(InputMappings) {}

	UPROPERTY(config)
	TArray<FEnhancedActionKeyMapping> DefaultInputMappings;
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

	UPROPERTY(config)
	TArray<struct FInputActionKeyMapping> DefaultActionMappings;

	UPROPERTY(config)
	TArray<struct FInputAxisKeyMapping> DefaultAxisMappings;

	// A map for each Input Context in your game and its respective Default Input Context Mappings
	UPROPERTY(config)
	TMap<TSoftObjectPtr<UInputMappingContext>, FInputMappingArray> DefaultEnhancedInputMappings;
};

/**
 * 
 */
UCLASS(config = UINavSettings, defaultconfig)
class UINAVIGATION_API UUINavSettings : public UObject
{
	GENERATED_BODY()

	UUINavSettings(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
	{
		bIgnoreDisabledUINavButton = true;
		bRemoveWidgetOnReturn = true;
	}
	
public:

	UPROPERTY(config, EditAnywhere, Category = "Settings")
	bool bIgnoreDisabledUINavButton;
	
	UPROPERTY(config, EditAnywhere, Category = "Settings")
	bool bRemoveWidgetOnReturn;

	UPROPERTY(config, EditAnywhere, Category = "Bindings")
	TSoftObjectPtr<UInputMappingContext> EnhancedInputContext = TSoftObjectPtr<UInputMappingContext>(FSoftObjectPath("/UINavigation/Input/IC_UINav.IC_UINav"));

	UPROPERTY(config, EditAnywhere, Category = "Bindings")
	TSoftObjectPtr<UUINavEnhancedInputActions> EnhancedInputActions = TSoftObjectPtr<UUINavEnhancedInputActions>(FSoftObjectPath("/UINavigation/Input/UINavEnhancedInputActions.UINavEnhancedInputActions"));
};
