// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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


/**
 * 
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
	*	Changes the active widget to the specified widget
	*
	*	@param NewWidget The new active widget
	*/
	UFUNCTION(BlueprintCallable)
		void SetActiveWidget(class UUINavWidget* NewWidget);

	FORCEINLINE UUINavWidget* GetActiveWidget() const { return ActiveWidget; }


protected:

	/*
	Whether the controller should notify the widget when the player
	started using a gamepad or keyboard that he/she wasn't using
	(i.e. notify that the player is now using a gamepad but was until now using
	a keyboard and vice-versa)
	*/
	UPROPERTY(EditDefaultsOnly)
		bool bNotifyGamepadKeyboardEvents;

	UPROPERTY()
		class UUINavWidget* ActiveWidget;

	EInputType CurrentInputType = EInputType::None;

	virtual void SetupInputComponent() override;

	virtual void MenuUp();
	virtual void MenuDown();
	virtual void MenuLeft();
	virtual void MenuRight();
	virtual void MenuSelect();
	virtual void MenuReturn();
	
	
};
