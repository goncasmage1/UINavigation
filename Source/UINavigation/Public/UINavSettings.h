// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UObject/NoExportTypes.h"
#include "GameFramework/PlayerInput.h"
#include "InputMappingContext.h"
#include "Data/UINavEnhancedInputActions.h"
#include "UINavSettings.generated.h"

/**
 * 
 */
UCLASS(config = UINavInput, defaultconfig)
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
	TSoftObjectPtr<UInputMappingContext> EnhancedInputContext;

	UPROPERTY(config, EditAnywhere, Category = "Bindings")
	TSoftObjectPtr<UUINavEnhancedInputActions> EnhancedInputActions;

	UPROPERTY(config)
	TArray<FInputActionKeyMapping> ActionMappings;

	UPROPERTY(config)
	TArray<FInputAxisKeyMapping> AxisMappings;

	// A map for each Input Context in your game and its respective Default Input Context Mappings
	UPROPERTY(config, EditAnywhere, Category = "Bindings")
	TMap<TSoftObjectPtr<UInputMappingContext>, TSoftObjectPtr<UInputMappingContext>> DefaultInputContexts;
};
