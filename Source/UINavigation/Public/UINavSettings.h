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
		bStopNextPreviousNavigation = true;
		bIgnoreDisabledButton = true;
		bRemoveWidgetOnReturn = true;
	}
	
public:

	/* 
	* Whether to force a button to be visually always, even if it's not being hovered by the mouse.
	* If set to true, when the mouse unhovers a button, it's button style will continue as hovered, and if you then use the keyboard or the mouse, navigation will happen.
	* If set to false, when the mouse unhovers a button, it's button style will be set back to normal, and if you then use the keyboard or the mouse for the first time,
	* the hovered button style will be set, and only after the second input will navigation happen.
	*/
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bForceNavigation = true;

	// Whether focus navigation should stop when using Next/Previous input
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bStopNextPreviousNavigation = true;

	// Whether disables buttons should be ignored for navigation
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bIgnoreDisabledButton = true;
	
	// Whether to call return to parent when using the Back/Return input
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bRemoveWidgetOnReturn = true;

	UPROPERTY(config, EditAnywhere, Category = "Settings")
	TSoftObjectPtr<UInputMappingContext> EnhancedInputContext = TSoftObjectPtr<UInputMappingContext>(FSoftObjectPath("/UINavigation/Input/IC_UINav.IC_UINav"));

	UPROPERTY(config, EditAnywhere, Category = "Settings")
	TSoftObjectPtr<UUINavEnhancedInputActions> EnhancedInputActions = TSoftObjectPtr<UUINavEnhancedInputActions>(FSoftObjectPath("/UINavigation/Input/UINavEnhancedInputActions.UINavEnhancedInputActions"));
};
