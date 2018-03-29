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

USTRUCT(BlueprintType)
struct FKeyContainer
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString KeyActionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FKey ContainedKey;

	FKeyContainer()
	{
	}

	FKeyContainer(const FInputActionKeyMapping& Action)
		: KeyActionName(Action.ActionName.ToString())
		, ContainedKey(Action.Key)
	{
	}
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
	*	Notifies this controller that a mouse is being used
	*/
	void NotifyMouseInputType();

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

	EInputType CurrentInputType = EInputType::None;

	TMap<FString, TArray<FKeyContainer>> KeyMap = TMap<FString, TArray<FKeyContainer>>();


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

	virtual void MenuUp();
	virtual void MenuDown();
	virtual void MenuLeft();
	virtual void MenuRight();
	virtual void MenuSelect();
	virtual void MenuReturn();
	
	
};
