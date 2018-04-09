// Fill out your copyright notice in the Description page of Project Settings.

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
enum class EInputDirection : uint8
{
	None UMETA(DisplayName = "None"),
	Up UMETA(DisplayName = "Up"),
	Down UMETA(DisplayName = "Down"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

/**
 * This class contains the logic for input-related actions with UINavWidgets
 */
UCLASS()
class UINAVIGATION_API AUINavController : public APlayerController
{
	GENERATED_BODY()
	
public:

	/**
	*	Checks whether a gamepad is connected
	*
	*	@return Whether a gamepad is connected
	*/
	UFUNCTION(BlueprintCallable)
		bool IsGamepadConnected();

	/**
	*	Notifies this controller that a mouse is being used
	*/
	void NotifyMouseInputType();

	/**
	*	Notifies this controller that a mouse is being used
	*/
	void NotifyInputTypeChange(EInputType NewInputType);

	/**
	*	Changes the widget this PC will communicate with
	*
	*	@param NewWidget The new active widget
	*/
	UFUNCTION(BlueprintCallable)
		void SetActiveWidget(class UUINavWidget* NewWidget);

	FORCEINLINE UUINavWidget* GetActiveWidget() const { return ActiveWidget; }


protected:

	UPROPERTY()
		class UUINavWidget* ActiveWidget;

	EInputType CurrentInputType = EInputType::Keyboard;

	TMap<FString, TArray<FKey>> KeyMap = TMap<FString, TArray<FKey>>();

	EInputDirection Direction = EInputDirection::None;


	/*
	Indicates whether navigation will occur periodically after the player
	holds down a navigation key
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
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

	FTimerHandle NavChainHandle;
	FTimerDelegate NavChainDelegate;


	/*************************************************************************/


	/**
	*	Searches all the Input Actions relevant to UINav plugin and saves them in a map
	*/
	void FetchUINavActionKeys();

	/**
	*	Returns the type of input that was just executed
	*
	*	@return The input type that was just used
	*/
	EInputType GetLastInputType(FString ActionName);

	/**
	*	Verifies if a new input type is being used
	*/
	void VerifyInputType(FString ActionName);

	virtual void SetupInputComponent() override;
	virtual void Possess(APawn* InPawn) override;

	void MenuUp();
	void MenuDown();
	void MenuLeft();
	void MenuRight();
	void MenuSelect();
	void MenuReturn();

	void MenuUpRelease();
	void MenuDownRelease();
	void MenuLeftRelease();
	void MenuRightRelease();

	void StartMenuUp();
	void StartMenuDown();
	void StartMenuLeft();
	void StartMenuRight();
	
	
};
