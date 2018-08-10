// Copyright (C) 2018 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Engine.h"
#include "GameFramework/PlayerController.h"
#include "UINavController.generated.h"

UENUM(BlueprintType)
enum class EInputType : uint8
{
	None UMETA(DisplayName = "None"),
	Keyboard UMETA(DisplayName = "Keyboard"),
	Mouse UMETA(DisplayName = "Mouse"),
	Gamepad UMETA(DisplayName = "Gamepad")
};

UENUM(BlueprintType)
enum class ENavigationDirection : uint8
{
	None,
	Up,
	Down,
	Left,
	Right
};

UENUM(BlueprintType)
enum class ECountdownPhase : uint8
{
	None UMETA(DisplayName = "None"),
	First UMETA(DisplayName = "First"),
	Looping UMETA(DisplayName = "Looping")
};

/**
 * This class contains the logic for input-related actions with UINavWidgets
 */
UCLASS()
class UINAVIGATION_API AUINavController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(BlueprintReadOnly)
		class UUINavWidget* ActiveWidget;

	UPROPERTY(BlueprintReadOnly)
		EInputType CurrentInputType = EInputType::Keyboard;

	TMap<FString, TArray<FKey>> KeyMap = TMap<FString, TArray<FKey>>();

	ENavigationDirection Direction = ENavigationDirection::None;

	/*
	Indicates whether navigation will occur periodically after the player
	holds down a navigation key
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bChainNavigation = true;

	/*
	The amount of time the player needs to hold a key for the navigation to
	start occuring periodically
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float InputHeldWaitTime = 0.5f;

	/*
	The amount of time that passes before the navigation chains further
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float NavigationChainFrequency = 0.2f;

	ECountdownPhase CountdownPhase = ECountdownPhase::None;

	ENavigationDirection CallbackDirection;
	float TimerCounter = 0.f;

	/*************************************************************************/

	void TimerCallback();
	void SetTimer(ENavigationDirection Direction);
	void ClearTimer();

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
	EInputType GetActionInputType(FString Action);

	/**
	*	Verifies if a new input type is being used
	*
	*	@param Key The specified key
	*	@param PressedKey The pressed key
	*/
	void VerifyInputTypeChangeByKey(FKey Key);

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

	/**
	*	Notifies to the active UUINavWidget that the input type changed
	*
	*	@param Action The action's name
	*	@param bPressed Whether the action was pressed or released
	*/
	void ExecuteActionByName(FString Action, bool bPressed);

	/**
	*	Traverses the key map to find the action name associated with the given key
	*
	*	@param PressedKey The pressed key
	*	@param bPressed Whether the action was pressed or released
	*/
	void FindActionByKey(FKey PressedKey, bool bPressed);

	virtual void SetupInputComponent() override;
	virtual void Possess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

public:

	/**
	*	Checks whether a gamepad is connected
	*
	*	@return Whether a gamepad is connected
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINavigation")
		bool IsGamepadConnected();

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
	*	Changes the widget this PC will communicate with
	*
	*	@param NewWidget The new active widget
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		void SetActiveWidget(class UUINavWidget* NewWidget);

	/**
	*	Called when the root UINavWidget is removed from the viewport
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "UINavigation")
		void OnRootWidgetRemoved();
	void OnRootWidgetRemoved_Implementation();

	/**
	*	Called when the input type changes
	*
	*	@param From The input type being used before
	*	@param To The input type being used now
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "UINavigation")
		void OnInputChanged(EInputType From, EInputType To);
	virtual void OnInputChanged_Implementation(EInputType From, EInputType To);

	//Called when the player navigates in a certain direction
	/**
	*	Called when the input type changes
	*
	*	@param Direction The direction of navigation
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "UINavigation")
		void OnNavigated(ENavigationDirection Direction);
	void OnNavigated_Implementation(ENavigationDirection Direction);

	void MenuUp();
	void MenuDown();
	void MenuLeft();
	void MenuRight();
	void MenuSelect();
	void MenuReturn();

	void MenuUpRelease();
	void MenuDownRelease();
	void MenuLeftRelease();

	void StartMenuUp();
	void StartMenuDown();
	void StartMenuLeft();
	void StartMenuRight();
	void MenuRightRelease();

	FORCEINLINE EInputType GetCurrentInputType() const { return CurrentInputType; }

	FORCEINLINE UUINavWidget* GetActiveWidget() const { return ActiveWidget; }


	
};
