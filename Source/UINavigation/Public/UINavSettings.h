// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UObject/NoExportTypes.h"
#include "GameFramework/PlayerInput.h"
#include "InputMappingContext.h"
#include "Data/UINavEnhancedInputActions.h"
#include "Data/PlatformConfigData.h"
#include "Data/SelectorPosition.h"
#include "Math/MathFwd.h"
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
	* Whether to force a button to always be visually shown as navigated, even if it's not being hovered by the mouse when using the mouse.
	* If set to true, when the mouse unhovers a button, it's button style will continue as hovered, and if you then use the keyboard or the mouse, navigation will happen.
	* If set to false, when the mouse unhovers a button, it's button style will be set back to normal, and if you then use the keyboard or the mouse for the first time,
	* the hovered button style will be set, and only after the second input will navigation happen.
	*/
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bForceNavigation = true;

	// Whether navigation-relevant keys events will be consumed by the plugin
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bConsumeNavigationInputs = false;
	
	// Whether navigation is allowed while pressing a button
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bAllowNavigationWhilePressing = false;

	// Whether a UINavComponent that's released should gain focus at that moment
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bSetFocusOnRelease = true;

	// Whether focus navigation should stop when using Next/Previous input
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bStopNextPreviousNavigation = true;

	// Whether disabled buttons should be ignored for navigation
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bIgnoreDisabledButton = true;

	// Whether to call the OnReturn event when you press or release the MenuReturn key
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bReturnOnPress = false;
	
	// Whether to call return to parent when using the Back/Return input
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bRemoveWidgetOnReturn = true;

	// Whether to remove active UINavWidgets on EndPlay event
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bRemoveActiveWidgetsOnEndPlay = true;

	// Whether to allow a UINavComponent to lose focus to the viewport when in Input Mode GameAndUI
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bAllowFocusOnViewportInGameAndUI = false;

	// Whether to load the input icons asynchronously, in order to prevent Load Flushes
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bLoadInputIconsAsync = false;

	// The amount of mouse movement delta that will trigger a rebind attempt when listening to a new key for input rebinding
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	float MouseMoveRebindThreshold = 1.0f;

	// The amount of mouse movement delta that will trigger the input type being changed to mouse
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	float MouseInputChangeThreshold = 0.5f;

	// The amount of analog movement that will trigger the input type being changed to gamepad
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	float AnalogInputChangeThreshold = 0.1f;

	// What relative button position to move the mouse cursor to when navigating to that button (None if you don't want this to happen).
	/*
	* What relative button position to move the mouse cursor to when navigating to that button (None if you don't want this to happen).
	* This can be useful to displaying button tooltips when using the gamepad/keyboard.
	*/
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	ESelectorPosition MoveMouseToButtonPosition = ESelectorPosition::None;

	// The offset to use when moving the mouse cursor to the button's position.
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FVector2D MoveMouseToButtonOffset = FVector2D(-1.0f, 1.0f);

	// The list of widget types (Slate names) to allow to be focused
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TArray<FString> AllowedWidgetTypesToFocus = {
		TEXT("SObjectWidget"),
		TEXT("SButton"),
		TEXT("SUINavButton"),
		TEXT("SSpinBox"),
		TEXT("SEditableText"),
		TEXT("SMultilineEditableText")
	};

	// Increment by 1 everytime your project's default inputs change
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 CurrentInputVersion = 0;

	// The input data used for each platform
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TMap<FString, FPlatformConfigData> PlatformConfigData;

	UPROPERTY(config, EditAnywhere, Category = "Settings")
	TSoftObjectPtr<UInputMappingContext> EnhancedInputContext = TSoftObjectPtr<UInputMappingContext>(FSoftObjectPath("/UINavigation/Input/IC_UINav.IC_UINav"));

	UPROPERTY(config, EditAnywhere, Category = "Settings")
	TSoftObjectPtr<UUINavEnhancedInputActions> EnhancedInputActions = TSoftObjectPtr<UUINavEnhancedInputActions>(FSoftObjectPath("/UINavigation/Input/UINavEnhancedInputActions.UINavEnhancedInputActions"));
};
