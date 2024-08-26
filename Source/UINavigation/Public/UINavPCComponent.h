﻿// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "EnhancedActionKeyMapping.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Data/CountdownPhase.h"
#include "Data/InputMode.h"
#include "Data/InputRebindData.h"
#include "Data/InputRestriction.h"
#include "Data/InputType.h"
#include "Data/ThumbstickAsMouse.h"
#include "Data/AutoHideMouse.h"
#include "Types/SlateEnums.h"
#include "InputCoreTypes.h"
#include "Input/Reply.h"
#include "InputAction.h"
#include "Data/InputContainerEnhancedActionData.h"
#include "Data/PlatformConfigData.h"
#include "Delegates/DelegateCombinations.h"
#include "Misc/CoreMiscDefines.h"
#include "UObject/SoftObjectPtr.h"
#include "Data/PromptData.h"
#include "UINavPCComponent.generated.h"

class APlayerController;
class FUINavInputProcessor;
class UUINavInputBox;
class UTexture2D;
class UUINavWidget;
class UUINavPromptWidget;
class UInputMappingContext;
class UCurveFloat;
class FText;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputTypeChangedDelegate, EInputType, InputType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUpdateInputIconsDelegate);

USTRUCT(BlueprintType)
struct FAxis2D_Keys
{
	GENERATED_BODY()

public:

	FAxis2D_Keys()
	{

	}

	FAxis2D_Keys(FKey InPositiveKey, FKey InNegativeKey)
	{
		PositiveKey = InPositiveKey;
		NegativeKey = InNegativeKey;
	}

	FKey PositiveKey;
	FKey NegativeKey;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UINAVIGATION_API UUINavPCComponent : public UActorComponent
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadOnly, Category = UINavController)
	UUINavWidget* ActiveWidget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = UINavController)
	UUINavWidget* ActiveSubWidget = nullptr;

	//Indicates whether the player can navigate the widget
	bool bAllowDirectionalInput = true;
	//Indicates whether the player can select options in this widget
	bool bAllowSelectInput = true;
	//Indicates whether the player can return to this widget's parent
	bool bAllowReturnInput = true;
	//Indicates whether the player can switch sections using the MenuNext and MenuPrevious actions
	bool bAllowSectionInput = true;

	bool bIgnoreSelectRelease = false;
	bool bIgnoreMousePress = false;
	bool bIgnoreMouseRelease = false;

	bool bAutomaticNavigation = false;

	bool bUsingThumbstickAsMouse = false;

	UPROPERTY()
	APlayerController* PC = nullptr;

	TSharedPtr<FUINavInputProcessor> SharedInputProcessor = nullptr;

	FVector2D ThumbstickDelta = FVector2D::ZeroVector;

	ECountdownPhase CountdownPhase = ECountdownPhase::None;

	EUINavigation AllowDirection = EUINavigation::Invalid;

	UPROPERTY()
	UUINavInputBox* ListeningInputBox = nullptr;

	EUINavigation CallbackDirection;
	float TimerCounter = 0.f;

	bool bIgnoreNavigationKey = true;

	bool bReceivedAnalogInput = false;

	bool bIgnoreFocusByNavigation = false;

	bool bOverrideConsiderHover = false;

	UPROPERTY()
	TArray<const UInputMappingContext*> CachedInputContexts;

	UPROPERTY()
	TMap<const UInputMappingContext*, uint8> AppliedInputContexts;

	/*************************************************************************/

	void SetTimer(const EUINavigation NavigationDirection);

	void CacheGameInputContexts();

	void TryResetDefaultInputs();

	void InitPlatformData();

	/**
	*	Returns the input type of the given key
	*
	*	@param Key The specified key	
	*	@return The input type of the given key
	*/
	static EInputType GetKeyInputType(const FKey& Key);

	bool IsNavigationKeyEvent(const FKeyEvent& Key) const;

	/**
	*	Notifies to the active UUINavWidget that the input type changed
	*
	*	@param NewInputType The new input type that is being used
	*/
	void NotifyInputTypeChange(const EInputType NewInputType, const bool bAttemptUnforceNavigation = true);

	virtual void Activate(bool bReset) override;
	
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void OnControllerConnectionChanged(EInputDeviceConnectionState NewConnectionState, FPlatformUserId UserId, FInputDeviceId UserIndex);

	UUINavWidget* GetFirstCommonParent(UUINavWidget* const Widget1, UUINavWidget* const Widget2);

public:

	UUINavPCComponent();

	UPROPERTY(BlueprintReadOnly, Category = UINavController)
	EInputType CurrentInputType = EInputType::Mouse;

	UPROPERTY(BlueprintReadOnly, Category = UINavController)
	EInputType CurrentNavOnlyInputType = EInputType::Mouse;

	/*
	Indicates whether the left analog stick should be used for directional navigation
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	bool bUseAnalogDirectionalInput = true;

	/*
	Indicates whether navigation will occur periodically after the player
	holds down a navigation key
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	bool bChainNavigation = true;

	/*
	The amount of time the player needs to hold a key for the navigation to
	start occuring periodically
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	float InputHeldWaitTime = 0.5f;

	/*
	The amount of time that passes before the navigation chains further
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	float NavigationChainFrequency = 0.15f;

	/*
	Indicates whether the controller should use the left or right stick as mouse.
	If the active UINavWidget has this set to a value different than None, it will override this one.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	EThumbstickAsMouse UseThumbstickAsMouse = EThumbstickAsMouse::None;

	/*
	The sensitivity of the cursor when moved with the left stick
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	float ThumbstickCursorSensitivity = 10.0f;

	/*
	The deadzone of the thumbstick when using it as a mouse cursor.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	float ThumbstickCursorDeadzone = 0.1f;

	/*
	A float curve that dictates how much the analog stick moves the mouse in relation to the sensitivity
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	UCurveFloat* ThumbstickCursorCurve;

	/*
	Indicates whether you can scroll through scroll boxes using the gamepad's right thumbstick
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	bool bScrollWithRightThumbstick = true;

	/*
	Indicates the deadzone to use when scrolling with the right thumbstick
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	float RightThumbstickScrollDeadzone = 0.1f;

	/*
	Indicates whether the controller should automatically hide the mouse cursor when using certain input types.
	In order for this to work, you have to set your GameViewportClient to UINavGameViewportClient in the Project Settings,
	or your custom ViewportClient class must inherit from UINavGameViewportClient and not override the GetCursor function.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	EAutoHideMouse AutoHideMouse = EAutoHideMouse::Never;

	/*
	If true, only perform the Mouse->Keyboard input mode transition on navigation keys, not any key press.
	This can help avoid flip-flopping on UIs which expect both mouse and keyboard usage.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	bool bPreferMouse = true;

	/*
	The sensitivity of scrolling when using the right thumbstick
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	float RightThumbstickScrollSensitivity = 10.0f;

	/*
	The required value for an axis to be considered for rebinding
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	float RebindThreshold = 0.5f;

	TMap<EUINavigation, TArray<FKey>> PressedNavigationDirections;

	/*
	Holds the key icons for gamepad
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavController, meta = (RequiredAssetDataTags = "RowStructure=/Script/UINavigation.InputIconMapping"))
	UDataTable* GamepadKeyIconData = nullptr;
	/*
	Holds the key icons for mouse and keyboard
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavController, meta = (RequiredAssetDataTags = "RowStructure=/Script/UINavigation.InputIconMapping"))
	UDataTable* KeyboardMouseKeyIconData = nullptr;

	/*
	Holds the key names for gamepad
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavController, meta = (RequiredAssetDataTags = "RowStructure=/Script/UINavigation.InputNameMapping"))
	UDataTable* GamepadKeyNameData = nullptr;
	/*
	Holds the key names for mouse and keyboard
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavController, meta = (RequiredAssetDataTags = "RowStructure=/Script/UINavigation.InputNameMapping"))
	UDataTable* KeyboardMouseKeyNameData = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	UInputMappingContext* UINavInputContextRef;

	FPlatformConfigData CurrentPlatformData;

	FKey LastPressedKey;
	int32 LastPressedKeyUserIndex;

	FKey LastReleasedKey;
	int32 LastReleasedKeyUserIndex;

	static const FKey MouseUp;
	static const FKey MouseDown;
	static const FKey MouseRight;
	static const FKey MouseLeft;

	static bool bInitialized;

	TMap<FKey, FAxis2D_Keys> Axis2DToAxis1DMap = {
		{EKeys::Gamepad_Left2D, {EKeys::Gamepad_LeftX, EKeys::Gamepad_LeftY}},
		{EKeys::Gamepad_Right2D, {EKeys::Gamepad_RightX, EKeys::Gamepad_RightY}},
		{EKeys::Mouse2D, {EKeys::MouseX, EKeys::MouseY}},
	};
	
	TMap<FKey, FAxis2D_Keys> AxisToKeyMap = {
		{EKeys::Gamepad_LeftX, {EKeys::Gamepad_LeftStick_Right, EKeys::Gamepad_LeftStick_Left}},
		{EKeys::Gamepad_LeftY, {EKeys::Gamepad_LeftStick_Up, EKeys::Gamepad_LeftStick_Down}},
		{EKeys::Gamepad_RightX, {EKeys::Gamepad_RightStick_Right, EKeys::Gamepad_RightStick_Left}},
		{EKeys::Gamepad_RightY, {EKeys::Gamepad_RightStick_Up, EKeys::Gamepad_RightStick_Down}},
		{EKeys::MouseX, {MouseRight, MouseLeft}},
		{EKeys::MouseY, {MouseUp, MouseDown}},
		{EKeys::MouseWheelAxis, {EKeys::MouseScrollUp, EKeys::MouseScrollDown}},
		{EKeys::MixedReality_Left_Thumbstick_X, {EKeys::MixedReality_Left_Thumbstick_Right, EKeys::MixedReality_Left_Thumbstick_Left}},
		{EKeys::MixedReality_Left_Thumbstick_Y, {EKeys::MixedReality_Left_Thumbstick_Up, EKeys::MixedReality_Left_Thumbstick_Down}},
		{EKeys::MixedReality_Right_Thumbstick_X, {EKeys::MixedReality_Right_Thumbstick_Right, EKeys::MixedReality_Right_Thumbstick_Left}},
		{EKeys::MixedReality_Right_Thumbstick_Y, {EKeys::MixedReality_Right_Thumbstick_Up, EKeys::MixedReality_Right_Thumbstick_Down}},
		{EKeys::OculusTouch_Left_Thumbstick_X, {EKeys::OculusTouch_Left_Thumbstick_Right, EKeys::OculusTouch_Left_Thumbstick_Left}},
		{EKeys::OculusTouch_Left_Thumbstick_Y, {EKeys::OculusTouch_Left_Thumbstick_Up, EKeys::OculusTouch_Left_Thumbstick_Down}},
		{EKeys::OculusTouch_Right_Thumbstick_X, {EKeys::OculusTouch_Right_Thumbstick_Right, EKeys::OculusTouch_Right_Thumbstick_Left}},
		{EKeys::OculusTouch_Right_Thumbstick_Y, {EKeys::OculusTouch_Right_Thumbstick_Up, EKeys::OculusTouch_Right_Thumbstick_Down}},
		{EKeys::ValveIndex_Left_Thumbstick_X, {EKeys::ValveIndex_Left_Thumbstick_Right, EKeys::ValveIndex_Left_Thumbstick_Left}},
		{EKeys::ValveIndex_Left_Thumbstick_Y, {EKeys::ValveIndex_Left_Thumbstick_Up, EKeys::ValveIndex_Left_Thumbstick_Down}},
		{EKeys::ValveIndex_Right_Thumbstick_X, {EKeys::ValveIndex_Right_Thumbstick_Right, EKeys::ValveIndex_Right_Thumbstick_Left}},
		{EKeys::ValveIndex_Right_Thumbstick_Y, {EKeys::ValveIndex_Right_Thumbstick_Up, EKeys::ValveIndex_Right_Thumbstick_Down}},
		{EKeys::Vive_Left_Trackpad_X, {EKeys::Vive_Left_Trackpad_Right, EKeys::Vive_Left_Trackpad_Left}},
		{EKeys::Vive_Left_Trackpad_Y, {EKeys::Vive_Left_Trackpad_Up, EKeys::Vive_Left_Trackpad_Down}},
		{EKeys::Vive_Right_Trackpad_X, {EKeys::Vive_Right_Trackpad_Right, EKeys::Vive_Right_Trackpad_Left}},
		{EKeys::Vive_Right_Trackpad_Y, {EKeys::Vive_Right_Trackpad_Up, EKeys::Vive_Right_Trackpad_Down}},
	};

	TMap<FKey, FKey> KeyToAxisMap = {
		{EKeys::Gamepad_LeftTrigger, EKeys::Gamepad_LeftTriggerAxis},
		{EKeys::Gamepad_RightTrigger, EKeys::Gamepad_RightTriggerAxis},
		{EKeys::MixedReality_Left_Trigger_Click, EKeys::MixedReality_Left_Trigger_Axis},
		{EKeys::MixedReality_Right_Trigger_Click, EKeys::MixedReality_Right_Trigger_Axis},
		{EKeys::OculusTouch_Left_Grip_Click, EKeys::OculusTouch_Left_Grip_Axis},
		{EKeys::OculusTouch_Right_Grip_Click, EKeys::OculusTouch_Right_Grip_Axis},
		{EKeys::ValveIndex_Left_Trigger_Click, EKeys::ValveIndex_Left_Trigger_Axis},
		{EKeys::ValveIndex_Right_Trigger_Click, EKeys::ValveIndex_Right_Trigger_Axis},
		{EKeys::Vive_Left_Trigger_Click, EKeys::Vive_Left_Trigger_Axis},
		{EKeys::Vive_Right_Trigger_Click, EKeys::Vive_Right_Trigger_Axis},
	};

	UPROPERTY(BlueprintAssignable, BlueprintCallable, BlueprintReadOnly, Category = UINavController)
	FInputTypeChangedDelegate InputTypeChangedDelegate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, BlueprintReadOnly, Category = UINavController)
	FUpdateInputIconsDelegate UpdateInputIconsDelegate;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FORCEINLINE bool AllowsAllMenuInput() const { return bAllowDirectionalInput && bAllowSelectInput && bAllowReturnInput && bAllowSectionInput; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FORCEINLINE bool AllowsDirectionalInput() const { return bAllowDirectionalInput; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FORCEINLINE bool AllowsSelectInput() const { return bAllowSelectInput; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FORCEINLINE bool AllowsReturnInput() const { return bAllowReturnInput; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FORCEINLINE bool AllowsSectionInput() const { return bAllowSectionInput; }

	FORCEINLINE bool AllowsNavigatingDirection(const EUINavigation Direction) const { return AllowsDirectionalInput() || (AllowDirection != EUINavigation::Invalid && AllowDirection != Direction); }
	
	EThumbstickAsMouse UsingThumbstickAsMouse() const;

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void SetShowMouseCursor(const bool bShowMouse);

	UFUNCTION(BlueprintCallable, Category = UINavController)
	bool HidingMouseCursor() const;

	bool ShouldHideMouseCursor() const;

	bool OverrideConsiderHover() const { return bOverrideConsiderHover; }

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void RefreshNavigationKeys();

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void SetAllowAllMenuInput(const bool bAllowInput);

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void SetAllowDirectionalInput(const bool bAllowInput);

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void SetAllowSelectInput(const bool bAllowInput);

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void SetAllowReturnInput(const bool bAllowInput);

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void SetAllowSectionInput(const bool bAllowInput);

	void SetIgnoreFocusByNavigation(const bool bIgnore) { bIgnoreFocusByNavigation = bIgnore; }
	bool IgnoreFocusByNavigation() const { return bIgnoreFocusByNavigation; }

	void RequestRebuildMappings();

	void AddInputContextFromUINavWidget(const UUINavWidget* const UINavWidget, const UUINavWidget* const ParentLimit = nullptr);
	void RemoveInputContextFromUINavWidget(const UUINavWidget* const UINavWidget, const UUINavWidget* const ParentLimit = nullptr);
	
	void AddInputContextForMenu(const TObjectPtr<UInputMappingContext> Context);

	void RemoveInputContextFromMenu(const TObjectPtr<UInputMappingContext> Context);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void AddInputContext(const UInputMappingContext* const Context, const int32 Priority = 0);
	UFUNCTION(BlueprintCallable, Category = "Input")
	void RemoveInputContext(const UInputMappingContext* const Context);
		
	void HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent);
	void HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent);
	void HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent);
	void HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent);
	void HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent);
	void HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent);
	void HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGesture);

	UFUNCTION()
	void OnControlMappingsRebuilt();
	
	UFUNCTION(BlueprintCallable, Category = UINavController)
	void SimulateMousePress();
	UFUNCTION(BlueprintCallable, Category = UINavController)
	void SimulateMouseRelease();
	UFUNCTION(BlueprintCallable, Category = UINavController)
	void SimulateMouseClick();

	FKey GetKeyUsedForNavigation(const EUINavigation Direction) const;
	FKey GetMostRecentlyPressedKey(const ENavigationGenesis Genesis) const;

	void ProcessRebind(const FKeyEvent& KeyEvent);
	void CancelRebind();
	
	/**
	*	Verifies if a new input type is being used
	*
	*	@param Key The specified key
	*/
	void VerifyInputTypeChangeByKey(const FKeyEvent& KeyEvent, const bool bAttemptUnforceNavigation = true);

	/**
	*	Notifies the controller that a mouse is being used
	*/
	void NotifyMouseInputType();

	void ListenToInputRebind(UUINavInputBox* InputBox);

	bool GetAndConsumeIgnoreSelectRelease();

	bool IsListeningToInputRebind() const;

	//Returns the currently used input mode
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	EInputMode GetInputMode() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	const FKey GetKeyFromAxis(const FKey& Key, bool bPositive, const EInputAxis Axis = EInputAxis::X) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	const FKey GetAxisFromScaledKey(const FKey& Key, bool& OutbPositive) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	const FKey GetAxisFromKey(const FKey& Key) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	const FKey GetAxis1DFromAxis2D(const FKey& Key, const EInputAxis Axis) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	const FKey GetAxis2DFromAxis1D(const FKey& Key) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	const FKey GetOppositeAxisKey(const FKey& Key, bool& bOutIsPositive) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	const FKey GetOppositeAxis2DAxis(const FKey& Key) const;

	void GetAxisPropertiesFromMapping(const FEnhancedActionKeyMapping& ActionMapping, bool& bOutPositive, EInputAxis& OutAxis) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	bool IsAxis2D(const FKey& Key) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	bool IsAxis(const FKey& Key) const;

	//Receives the action and returns the first key that respects the given restriction.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FKey GetEnhancedInputKey(const UInputAction* Action, const EInputAxis Axis = EInputAxis::X, const EAxisType Scale = EAxisType::None, const EInputRestriction InputRestriction = EInputRestriction::None) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	UTexture2D* GetKeyIcon(const FKey Key) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	TSoftObjectPtr<UTexture2D> GetSoftKeyIcon(const FKey Key) const;

	//Get first found Icon associated with the given enhanced input action
	//Will search the icon table
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	UTexture2D* GetEnhancedInputIcon(const UInputAction* Action, const EInputAxis Axis = EInputAxis::X, const EAxisType Scale = EAxisType::None, const EInputRestriction InputRestriction = EInputRestriction::None) const;

	//Get first found Icon associated with the given enhanced input action
	//Will search the icon table
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	TSoftObjectPtr<UTexture2D> GetSoftEnhancedInputIcon(const UInputAction* Action, const EInputAxis Axis = EInputAxis::X, const EAxisType Scale = EAxisType::None, const EInputRestriction InputRestriction = EInputRestriction::None) const;

	//Get first found Icon associated with the given enhanced input action
	//Will search the icon table
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FText GetEnhancedInputText(const UInputAction* Action, const EInputAxis Axis = EInputAxis::X, const EAxisType Scale = EAxisType::None, const EInputRestriction InputRestriction = EInputRestriction::None) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FText GetKeyText(const FKey Key) const;

	//Receives the enhanced input action and returns all the keys associated with that action
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	void GetEnhancedInputKeys(const UInputAction* Action, TArray<FKey>& OutKeys);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FKey GetCurrentKey(const FEnhancedActionKeyMapping& Mapping) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	UEnhancedInputComponent* GetEnhancedInputComponent() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	UInputMappingContext* GetUINavInputContext() const;

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void SetActiveWidget(UUINavWidget* NewActiveWidget);

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void NotifyNavigatedTo(UUINavWidget* NavigatedWidget);

	/**
	*	Adds given widget to screen (strongly recommended over manual alternative)
	*
	*	@param	NewWidgetClass  The class of the widget to add to the screen
	*	@param	bRemoveParent  Whether to remove the parent widget (this widget) from the viewport
	*	@param  bDestroyParent  Whether to destruct the parent widget (this widget)
	*	@param  ZOrder Order to display the widget
	*/
	UFUNCTION(BlueprintCallable, Category = UINavController, meta = (AdvancedDisplay = 2, DeterminesOutputType = "NewWidgetClass"))
	UUINavWidget* GoToWidget(TSubclassOf<UUINavWidget> NewWidgetClass, const bool bRemoveParent, const bool bDestroyParent = false, const int ZOrder = 0);

	/**
	*	Adds given widget to screen (strongly recommended over manual alternative)
	*
	*	@param	NewWidgetClass  The class of the widget to add to the screen
	*	@param	bRemoveParent  Whether to remove the parent widget (this widget) from the viewport
	*	@param  bDestroyParent  Whether to destruct the parent widget (this widget)
	*	@param  ZOrder Order to display the widget
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta = (AdvancedDisplay = 4, DeterminesOutputType = "NewWidgetClass"))
	UUINavWidget* GoToPromptWidget(TSubclassOf<UUINavPromptWidget> NewWidgetClass, const FPromptWidgetDecided& Event, const FText Title = FText(), const FText Message = FText(), const bool bRemoveParent = false, const int ZOrder = 0);

	/**
	*	Adds given widget to screen (strongly recommended over manual alternative)
	*
	*	@param	NewWidget  Object instance of the UINavWidget to add to the screen
	*	@param	bRemoveParent  Whether to remove the parent widget (this widget) from the viewport
	*	@param  bDestroyParent  Whether to destruct the parent widget (this widget)
	*	@param  ZOrder Order to display the widget
	*/
	UFUNCTION(BlueprintCallable, Category = UINavController, meta = (AdvancedDisplay = 2, DeterminesOutputType = "NewWidgetClass"))
	UUINavWidget* GoToBuiltWidget(UUINavWidget* NewWidget, const bool bRemoveParent, const bool bDestroyParent = false, const int ZOrder = 0);

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void NavigateInDirection(const EUINavigation Direction);
	void MenuNext();
	void MenuPrevious();

	void NotifyNavigationKeyPressed(const FKey& Key, const EUINavigation Direction);
	void NotifyNavigationKeyReleased(const FKey& Key, const EUINavigation Direction);

	bool TryNavigateInDirection(const EUINavigation Direction, const ENavigationGenesis Genesis);

	void ClearAnalogKeysFromPressedKeys(const FKey& PressedKey);

	void ClearNavigationTimer();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FORCEINLINE APlayerController* GetPC() const { return PC; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FORCEINLINE EInputType GetCurrentInputType() const { return CurrentInputType; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	const FPlatformConfigData& GetCurrentPlatformData() const { return CurrentPlatformData; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
    FORCEINLINE bool IsUsingMouse() const { return CurrentInputType == EInputType::Mouse; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
    FORCEINLINE bool IsUsingKeyboard() const { return CurrentInputType == EInputType::Keyboard; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
    FORCEINLINE bool IsUsingGamepad() const { return CurrentInputType == EInputType::Gamepad; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FORCEINLINE UUINavWidget* GetActiveWidget() const { return ActiveWidget; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FORCEINLINE UUINavWidget* GetActiveSubWidget() const { return ActiveSubWidget != nullptr ? ActiveSubWidget : ActiveWidget; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	bool IsWidgetActive(const UUINavWidget* const UINavWidget) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	bool IsWidgetChild(const UUINavWidget* const ParentWidget, const UUINavWidget* const ChildWidget) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
    FORCEINLINE FVector2D GetThumbstickDelta() const { return ThumbstickDelta; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
    FORCEINLINE bool IsMovingThumbstick() const { return ThumbstickDelta.X != 0.0f || ThumbstickDelta.Y != 0.0f; }
		
};
