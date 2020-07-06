// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

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
#include "UINavPCComponent.generated.h"

DECLARE_DELEGATE_OneParam(FMouseKeyDelegate, FKey);

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
		class UUINavWidget* ActiveWidget;

	//Indicates whether the player can navigate the widget
	bool bAllowDirectionalInput = true;
	//Indicates whether the player can select options in this widget
	bool bAllowSelectInput = true;
	//Indicates whether the player can return to this widget's parent
	bool bAllowReturnInput = true;
	//Indicates whether the player can switch sections using the MenuNext and MenuPrevious actions
	bool bAllowSectionInput = true;

	TArray<bool> bAllowCustomInputs;

	class APlayerController* PC;

	ENavigationDirection Direction = ENavigationDirection::None;

	float PreviousX = -1.f;
	float PreviousY = -1.f;

	ECountdownPhase CountdownPhase = ECountdownPhase::None;

	ENavigationDirection CallbackDirection;
	float TimerCounter = 0.f;

	/*************************************************************************/

	void TimerCallback();
	void SetTimer(ENavigationDirection NavigationDirection);

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
	EInputType GetKeyInputType(FKey Key);

	/**
	*	Returns the input type of the given action
	*
	*	@return The input type of the given action
	*/
	EInputType GetMenuActionInputType(FString Action);

	/**
	*	Verifies if a new input type is being used
	*
	*	@param Action The specified action
	*/
	void VerifyInputTypeChangeByAction(FString Action);

	/**
	*	Notifies to the active UUINavWidget that the input type changed
	*
	*	@param NewInputType The new input type that is being used
	*/
	void NotifyInputTypeChange(EInputType NewInputType);

	virtual void Activate(bool bReset) override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = UINavController)
	void BindMenuInputs(); 
	UFUNCTION(BlueprintCallable, Category = UINavController)
	void UnbindMenuInputs();

	UFUNCTION()
	void OnCustomInput(int InputIndex, bool bPressed);

	void CallCustomInput(FName ActionName, bool bPressed);


public:

	UUINavPCComponent();

	UPROPERTY(BlueprintReadWrite, Category = UINavController)
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
	The required value for an axis to be considered for rebinding
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavController)
		float RebindThreshold = 0.5f;

	TMap<FString, TArray<FKey>> KeyMap = TMap<FString, TArray<FKey>>();

	TArray<FString> PressedActions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavController)
		TArray<FName> CustomInputs;

	/*
	Holds the key icons for gamepad
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavController)
		UDataTable* GamepadKeyIconData;
	/*
	Holds the key icons for mouse and keyboard
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavController)
		UDataTable* KeyboardMouseKeyIconData;

	/*
	Holds the key names for gamepad
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavController)
		UDataTable* GamepadKeyNameData;
	/*
	Holds the key names for mouse and keyboard
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavController)
		UDataTable* KeyboardMouseKeyNameData;

	/*
	Holds all the data for each rebindable input
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavController)
		UDataTable* InputRebindDataTable;

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
	FORCEINLINE bool AllowsCustomInputByName(FName InputName) const
	{ 
		int CustomInputIndex = CustomInputs.Find(InputName);
		if (CustomInputIndex < 0) return false;
		return bAllowCustomInputs[CustomInputIndex];
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
	FORCEINLINE bool AllowsCustomInputByIndex(int InputIndex) const
	{
		if (InputIndex < 0 || InputIndex >= CustomInputs.Num()) return false;
		return bAllowCustomInputs[InputIndex];
	}
	

	UFUNCTION(BlueprintCallable, Category = UINavController)
		void SetAllowAllMenuInput(bool bAllowInput);

	UFUNCTION(BlueprintCallable, Category = UINavController)
		void SetAllowDirectionalInput(bool bAllowInput);

	UFUNCTION(BlueprintCallable, Category = UINavController)
		void SetAllowSelectInput(bool bAllowInput);

	UFUNCTION(BlueprintCallable, Category = UINavController)
		void SetAllowReturnInput(bool bAllowInput);

	UFUNCTION(BlueprintCallable, Category = UINavController)
		void SetAllowSectionInput(bool bAllowInput);

	UFUNCTION(BlueprintCallable, Category = UINavController)
		void SetAllowCustomInputByName(FName InputName, bool bAllowInput);

	UFUNCTION(BlueprintCallable, Category = UINavController)
		void SetAllowCustomInputByIndex(int InputIndex, bool bAllowInput);


	void BindMouseWorkaround();
	void UnbindMouseWorkaround();

	/**
	*	Verifies if a new input type is being used
	*
	*	@param Key The specified key
	*	@param PressedKey The pressed key
	*/
	void VerifyInputTypeChangeByKey(FKey Key);

	/**
	*	Notifies the controller that a mouse is being used
	*/
	void NotifyMouseInputType();

	/**
	*	Notifies the controller that the given key was just pressed
	*
	*	@param PressedKey The pressed key
	*/
	void NotifyKeyPressed(FKey PressedKey);

	/**
	*	Notifies the controller that the given key was just released
	*
	*	@param ReleasedKey The released key
	*/
	void NotifyKeyReleased(FKey ReleasedKey);

	/**
	*	Executes a Menu Action by its name
	*
	*	@param Action The action's name
	*	@param bPressed Whether the action was pressed or released
	*/
	void ExecuteActionByName(FString Action, bool bPressed);

	/**
	*	Executes a Menu Action by its key
	*
	*	@param PressedKey The given key
	*	@param bPressed Whether the action was pressed or released
	*/
	void ExecuteActionByKey(FKey ActionKey, bool bPressed);

	/**
	*	Returns the action that contains the given key
	*
	*	@param PressedKey The given key
	*/
	FString FindActionByKey(FKey ActionKey);

	/**
	*	Called when an action key is pressed
	*
	*	@param ActionName The name of the action
	*/
	FReply OnActionPressed(FString ActionName, FKey Key);

	/**
	*	Called when an action key is released
	*
	*	@param ActionName The name of the action
	*/
	FReply OnActionReleased(FString ActionName, FKey Key);

	//Returns the currently used input mode
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
		EInputMode GetInputMode();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
		FKey GetKeyFromAxis(FKey Key, bool bPositive);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
		bool Is2DAxis(FKey Key);

	//Receives the name of the action, or axis with a + or - suffix, and returns
	//the first key that respects the given restriction.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
		FKey GetInputKey(FName ActionName, EInputRestriction InputRestriction);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
		class UTexture2D* GetKeyIcon(FKey Key);

	//Get first found Icon associated with the given input name
	//Will search the icon table
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
		class UTexture2D* GetInputIcon(FName ActionName, EInputRestriction InputRestriction);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
		FText GetKeyText(FKey Key);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
		void GetInputRebindData(FName InputName, FInputRebindData& OutData, bool& bSuccess);

	//Get first found name associated with the given input name
	//Will search the name table, if name can't be found will return the key's display name
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
		FText GetInputText(FName InputName);

	//Get all keys associated with the input with the given name
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DeprecatedFunction, DeprecationMessage = "Please use GetInputKeys instead") , Category = UINavController)
		TArray<FKey> GetInputKeysFromName(FName InputName);

	//Receives the name of the action, or axis with a + or - suffix, and returns
	//all the keys associated with that input.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavController)
		void GetInputKeys(FName ActionName, TArray<FKey>& OutKeys);

	UFUNCTION(BlueprintCallable, Category = UINavController)
		void SetActiveWidget(class UUINavWidget* NewActiveWidget);

	void MenuInput(ENavigationDirection Direction);
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

	void MouseKeyPressed(FKey MouseKey);

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
		FORCEINLINE UUINavWidget* GetActiveWidget() const { return ActiveWidget; }

		
};
