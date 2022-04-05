// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Data/CountdownPhase.h"
#include "Data/InputMode.h"
#include "Data/InputRebindData.h"
#include "Data/InputRestriction.h"
#include "Data/InputType.h"
#include "Data/NavigationDirection.h"
#include "InputCoreTypes.h"
#include "Input/Reply.h"
#include "InputAction.h"
#include "UINavPCComponent.generated.h"

DECLARE_DELEGATE_OneParam(FMouseKeyDelegate, FKey);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputTypeChangedDelegate, EInputType, InputType);

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
	class UUINavWidget* ActiveWidget = nullptr;

	//Indicates whether the player can navigate the widget
	bool bAllowDirectionalInput = true;
	//Indicates whether the player can select options in this widget
	bool bAllowSelectInput = true;
	//Indicates whether the player can return to this widget's parent
	bool bAllowReturnInput = true;
	//Indicates whether the player can switch sections using the MenuNext and MenuPrevious actions
	bool bAllowSectionInput = true;

	bool bShouldIgnoreHoverEvents = false;

	TArray<bool> bAllowCustomInputs;

	UPROPERTY()
	class APlayerController* PC = nullptr;

	TSharedPtr<class FUINavInputProcessor> SharedInputProcessor = nullptr;

	ENavigationDirection Direction = ENavigationDirection::None;

	FVector2D LeftStickDelta = FVector2D::ZeroVector;

	ECountdownPhase CountdownPhase = ECountdownPhase::None;

	ENavigationDirection CallbackDirection;
	float TimerCounter = 0.f;

	/*************************************************************************/

	void TimerCallback();
	void SetTimer(const ENavigationDirection NavigationDirection);

	void VerifyDefaultInputs();

	/**
	*	Searches all the Input Actions relevant to UINav plugin and saves them in a map
	*/
	void FetchUINavActionKeys();

	/**
	*	Returns the input type of the given key
	*
	*	@param Key The specified key	
	*	@return The input type of the given key
	*/
	static EInputType GetKeyInputType(const FKey Key);

	/**
	*	Returns the input type of the given action
	*
	*	@return The input type of the given action
	*/
	EInputType GetMenuActionInputType(const FString Action) const;

	/**
	*	Notifies to the active UUINavWidget that the input type changed
	*
	*	@param NewInputType The new input type that is being used
	*/
	void NotifyInputTypeChange(const EInputType NewInputType);

	virtual void Activate(bool bReset) override;
	
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void BindMenuInputs();
	
	UFUNCTION(BlueprintCallable, Category = UINavController)
	void UnbindMenuInputs();

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void BindMenuEnhancedInputs();

	UFUNCTION()
	void OnCustomInput(const int InputIndex, const bool bPressed);

	void CallCustomInput(const FName ActionName, const bool bPressed);

	void OnControllerConnectionChanged(bool bConnected, FPlatformUserId UserId, int32 UserIndex);

public:

	UUINavPCComponent();

	UPROPERTY(BlueprintReadOnly, Category = UINavController)
	EInputType CurrentInputType = EInputType::Keyboard;

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
	float NavigationChainFrequency = 0.2f;

	/*
	Indicates whether to automatically add and remove the UINav Input Context
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	bool bAutoAddUINavInputContext = true;

	/*
	Indicates the desired priority of the UINav Input Context
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	int UINavInputContextPriority = 0;

	/*
	Indicates whether the controller should use the left stick as mouse.
	If the active UINavWidget has this set to false, this will override that.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	bool bUseLeftThumbstickAsMouse = false;

	/*
	The sensitivity of the cursor when moved with the left stick
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	float LeftStickCursorSensitivity = 10.0f;

	/*
	The required value for an axis to be considered for rebinding
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
	float RebindThreshold = 0.5f;

	TMap<FString, TArray<FKey>> KeyMap = TMap<FString, TArray<FKey>>();

	TArray<FString> PressedActions;

	UPROPERTY(EditAnywhere, Category = UINavController)
	TArray<FName> CustomInputs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavController)
		TArray<UInputAction*> CustomEnhancedInputs;

	/*
	Holds the key icons for gamepad
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavController)
	UDataTable* GamepadKeyIconData = nullptr;
	/*
	Holds the key icons for mouse and keyboard
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavController)
	UDataTable* KeyboardMouseKeyIconData = nullptr;

	/*
	Holds the key names for gamepad
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavController)
	UDataTable* GamepadKeyNameData = nullptr;
	/*
	Holds the key names for mouse and keyboard
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavController)
	UDataTable* KeyboardMouseKeyNameData = nullptr;

	/*
	Holds all the data for each rebindable input
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavController)
	UDataTable* InputRebindDataTable = nullptr;

	TMap<FKey, FAxis2D_Keys> Axis2DToKeyMap = {
		{EKeys::Gamepad_LeftX, {EKeys::Gamepad_LeftStick_Right, EKeys::Gamepad_LeftStick_Left}},
		{EKeys::Gamepad_LeftY, {EKeys::Gamepad_LeftStick_Up, EKeys::Gamepad_LeftStick_Down}},
		{EKeys::Gamepad_RightX, {EKeys::Gamepad_RightStick_Right, EKeys::Gamepad_RightStick_Left}},
		{EKeys::Gamepad_RightY, {EKeys::Gamepad_RightStick_Up, EKeys::Gamepad_RightStick_Down}},
		/*{EKeys::Daydream_Left_Thumbstick_X, {EKeys::Daydream_Left_Thumbstick_Right, EKeys::Daydream_Left_Thumbstick_Left}},
		{EKeys::Daydream_Left_Thumbstick_Y, {EKeys::Daydream_Left_Thumbstick_Up, EKeys::Daydream_Left_Thumbstick_Down}},
		{EKeys::Daydream_Right_Thumbstick_X, {EKeys::Daydream_Right_Thumbstick_Right, EKeys::Daydream_Right_Thumbstick_Left}},
		{EKeys::Daydream_Right_Thumbstick_Y, {EKeys::Daydream_Right_Thumbstick_Up, EKeys::Daydream_Right_Thumbstick_Down}},*/
		{EKeys::MixedReality_Left_Thumbstick_X, {EKeys::MixedReality_Left_Thumbstick_Right, EKeys::MixedReality_Left_Thumbstick_Left}},
		{EKeys::MixedReality_Left_Thumbstick_Y, {EKeys::MixedReality_Left_Thumbstick_Up, EKeys::MixedReality_Left_Thumbstick_Down}},
		{EKeys::MixedReality_Right_Thumbstick_X, {EKeys::MixedReality_Right_Thumbstick_Right, EKeys::MixedReality_Right_Thumbstick_Left}},
		{EKeys::MixedReality_Right_Thumbstick_Y, {EKeys::MixedReality_Right_Thumbstick_Up, EKeys::MixedReality_Right_Thumbstick_Down}},
		/*{EKeys::OculusGo_Left_Thumbstick_X, {EKeys::OculusGo_Left_Thumbstick_Right, EKeys::OculusGo_Left_Thumbstick_Left}},
		{EKeys::OculusGo_Left_Thumbstick_Y, {EKeys::OculusGo_Left_Thumbstick_Up, EKeys::OculusGo_Left_Thumbstick_Down}},
		{EKeys::OculusGo_Right_Thumbstick_X, {EKeys::OculusGo_Right_Thumbstick_Right, EKeys::OculusGo_Right_Thumbstick_Left}},
		{EKeys::OculusGo_Right_Thumbstick_Y, {EKeys::OculusGo_Right_Thumbstick_Up, EKeys::OculusGo_Right_Thumbstick_Down}},*/
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

	UPROPERTY(BlueprintAssignable, BlueprintCallable, BlueprintReadOnly, Category = UINavController)
	FInputTypeChangedDelegate InputTypeChangedDelegate;

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

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FORCEINLINE bool AllowsCustomInputByName(const FName InputName) const
	{ 
		const int CustomInputIndex = CustomInputs.Find(InputName);
		if (CustomInputIndex < 0) return false;
		return bAllowCustomInputs[CustomInputIndex];
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FORCEINLINE bool AllowsCustomInputByAction(UInputAction* InputAction) const
	{ 
		const int CustomInputIndex = CustomEnhancedInputs.Find(InputAction);
		if (CustomInputIndex < 0) return false;
		return bAllowCustomInputs[CustomInputIndex];
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FORCEINLINE bool AllowsCustomInputByIndex(const int InputIndex) const
	{
		if (!CustomInputs.IsValidIndex(InputIndex)) return false;
		return bAllowCustomInputs[InputIndex];
	}
	

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

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void SetAllowCustomInputByName(const FName InputName, const bool bAllowInput);

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void SetAllowCustomInputByAction(UInputAction* InputAction, const bool bAllowInput);

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void SetAllowCustomInputByIndex(const int InputIndex, const bool bAllowInput);

	void HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent);
	void HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent);
	void HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent);
	void HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent);
	void HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent);
	void HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGesture);

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void SimulateMousePress();
	UFUNCTION(BlueprintCallable, Category = UINavController)
	void SimulateMouseRelease();
	UFUNCTION(BlueprintCallable, Category = UINavController)
	void SimulateMouseClick();

	void BindMouseWorkaround();
	void UnbindMouseWorkaround();

	/**
	*	Verifies if a new input type is being used
	*
	*	@param Key The specified key
	*/
	void VerifyInputTypeChangeByKey(const FKey Key);

	/**
	*	Notifies the controller that a mouse is being used
	*/
	void NotifyMouseInputType();

	/**
	*	Notifies the controller that the given key was just pressed
	*
	*	@param PressedKey The pressed key
	*/
	void NotifyKeyPressed(const FKey PressedKey);

	/**
	*	Notifies the controller that the given key was just released
	*
	*	@param ReleasedKey The released key
	*/
	void NotifyKeyReleased(const FKey ReleasedKey);

	/**
	*	Executes a Menu Action by its name
	*
	*	@param Action The action's name
	*	@param bPressed Whether the action was pressed or released
	*/
	void ExecuteActionByName(const FString Action, const bool bPressed);

	/**
	*	Executes a Menu Action by its key
	*
	*	@param ActionKey The given key
	*	@param bPressed Whether the action was pressed or released
	*/
	void ExecuteActionByKey(const FKey ActionKey, const bool bPressed);

	/**
	*	Returns the action that contains the given key
	*
	*	@param ActionKey The given key
	*/
	TArray<FString> FindActionByKey(const FKey ActionKey) const;

	FReply OnKeyPressed(const FKey PressedKey);
	FReply OnActionPressed(const FString ActionName, const FKey Key);

	FReply OnKeyReleased(const FKey PressedKey);
	FReply OnActionReleased(const FString ActionName, const FKey Key);

	//Returns the currently used input mode
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	EInputMode GetInputMode() const;

	FORCEINLINE bool ShouldIgnoreHoverEvents() const { return bShouldIgnoreHoverEvents; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FKey GetKeyFromAxis(FKey Key, bool bPositive) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	bool Is2DAxis(const FKey Key) const;

	//Receives the name of the action, or axis with a + or - suffix, and returns
	//the first key that respects the given restriction.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FKey GetInputKey(FName ActionName, const EInputRestriction InputRestriction) const;

	//Receives the action and returns the first key that respects the given restriction.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
		FKey GetEnhancedInputKey(const UInputAction* Action, const EInputRestriction InputRestriction) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	class UTexture2D* GetKeyIcon(const FKey Key) const;

	//Get first found Icon associated with the given input name
	//Will search the icon table
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	class UTexture2D* GetInputIcon(const FName ActionName, const EInputRestriction InputRestriction) const;

	//Get first found Icon associated with the given enhanced input action
	//Will search the icon table
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
		class UTexture2D* GetEnhancedInputIcon(const UInputAction* Action, const EInputRestriction InputRestriction) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FText GetKeyText(const FKey Key) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	void GetInputRebindData(const FName InputName, FInputRebindData& OutData, bool& bSuccess) const;

	//Get first found name associated with the given input name
	//Will search the name table, if name can't be found will return the key's display name
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FText GetInputText(const FName InputName) const;

	//Get all keys associated with the input with the given name
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DeprecatedFunction, DeprecationMessage = "Please use GetInputKeys instead") , Category = UINavController)
	TArray<FKey> GetInputKeysFromName(const FName InputName) const;

	//Receives the name of the action, or axis with a + or - suffix, and returns
	//all the keys associated with that input.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	void GetInputKeys(FName InputName, TArray<FKey>& OutKeys);

	//Receives the name of the enhanced input action and returns all the keys associated with that action
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	void GetEnhancedInputKeys(const UInputAction* Action, TArray<FKey>& OutKeys);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	UEnhancedInputComponent* GetEnhancedInputComponent() const;

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void SetActiveWidget(class UUINavWidget* NewActiveWidget);

	void SetActiveNestedWidget(class UUINavWidget* NewActiveWidget);

	void MenuInput(const ENavigationDirection Direction);
	void MenuSelect();
	void MenuReturn();
	void MenuNext();
	void MenuPrevious();

	void MenuUpRelease();
	void MenuDownRelease();
	void MenuLeftRelease();
	void MenuRightRelease();
	void MenuSelectRelease();
	void MenuReturnRelease();

	void MouseKeyPressed(const FKey MouseKey);

	void ClearTimer();

	void StartMenuUp();
	void StartMenuDown();
	void StartMenuLeft();
	void StartMenuRight();


	UFUNCTION(BlueprintCallable, Category = UINavController)
	FORCEINLINE APlayerController* GetPC() const { return PC; }

	UFUNCTION(BlueprintCallable, Category = UINavController)
	FORCEINLINE EInputType GetCurrentInputType() const { return CurrentInputType; }

	UFUNCTION(BlueprintCallable, Category = UINavController)
    FORCEINLINE bool IsUsingMouse() const { return CurrentInputType == EInputType::Mouse; }

	UFUNCTION(BlueprintCallable, Category = UINavController)
    FORCEINLINE bool IsUsingKeyboard() const { return CurrentInputType == EInputType::Keyboard; }
	
	UFUNCTION(BlueprintCallable, Category = UINavController)
    FORCEINLINE bool IsUsingGamepad() const { return CurrentInputType == EInputType::Gamepad; }

	UFUNCTION(BlueprintCallable, Category = UINavController)
	FORCEINLINE UUINavWidget* GetActiveWidget() const { return ActiveWidget; }

	UFUNCTION(BlueprintCallable, Category = UINavController)
    FORCEINLINE FVector2D GetLeftStickDelta() const { return LeftStickDelta; }

	UFUNCTION(BlueprintCallable, Category = UINavController)
    FORCEINLINE bool IsMovingLeftStick() const { return LeftStickDelta.X != 0.0f || LeftStickDelta.Y != 0.0f; }
		
};
