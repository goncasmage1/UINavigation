// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UObject/NoExportTypes.h"
#include "GameFramework/PlayerInput.h"
#include "InputMappingContext.h"
#include "Data/UINavEnhancedInputActions.h"
#include "UINavSettings.generated.h"

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
		bForceNavigation = true;
		bIgnoreDisabledButton = true;
		bRemoveWidgetOnReturn = true;
	}
	
public:

	UPROPERTY(config, EditAnywhere, Category = "Settings")
	bool bForceNavigation = true;

	UPROPERTY(config, EditAnywhere, Category = "Settings")
	bool bStopNextPreviousNavigation = true;

	// Whether disables buttons should be ignored for navigation
	UPROPERTY(config, EditAnywhere, Category = "Settings")
	bool bIgnoreDisabledButton = true;
	
	UPROPERTY(config, EditAnywhere, Category = "Settings")
	bool bRemoveWidgetOnReturn = true;

	UPROPERTY(config, EditAnywhere, Category = "Settings")
	TSoftObjectPtr<UInputMappingContext> EnhancedInputContext = TSoftObjectPtr<UInputMappingContext>(FSoftObjectPath("/UINavigation/Input/IC_UINav.IC_UINav"));

	UPROPERTY(config, EditAnywhere, Category = "Settings")
	TSoftObjectPtr<UUINavEnhancedInputActions> EnhancedInputActions = TSoftObjectPtr<UUINavEnhancedInputActions>(FSoftObjectPath("/UINavigation/Input/UINavEnhancedInputActions.UINavEnhancedInputActions"));
};
