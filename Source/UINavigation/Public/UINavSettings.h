// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

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
		TAssetPtr<UInputMappingContext> EnhancedInputContext;

	UPROPERTY(config, EditAnywhere, Category = "Bindings")
		TAssetPtr<UUINavEnhancedInputActions> EnhancedInputActions;

	UPROPERTY(config)
		TArray<struct FInputActionKeyMapping> ActionMappings;

	UPROPERTY(config)
		TArray<struct FInputAxisKeyMapping> AxisMappings;

	// A map for each Input Context in your game and its respective Default Input Context Mappings
	UPROPERTY(config, EditAnywhere, Category = "Bindings")
		TMap<TAssetPtr<UInputMappingContext>, TAssetPtr<UInputMappingContext>> DefaultInputContexts;
};
