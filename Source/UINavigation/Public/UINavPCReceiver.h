// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UObject/Interface.h"
#include "Data/InputType.h"
#include "Types/SlateEnums.h"
#include "UINavPCReceiver.generated.h"

class UUINavWidget;

UINTERFACE(MinimalAPI, BlueprintType)
class UUINavPCReceiver : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class UINAVIGATION_API IUINavPCReceiver
{
	GENERATED_BODY()

public:

	/**
	*	Called when the root UINavWidget is added to the viewport
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = UINavController)
	void OnRootWidgetAdded();

	virtual void OnRootWidgetAdded_Implementation();

	/**
	*	Called when the root UINavWidget is removed from the viewport
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = UINavController)
	void OnRootWidgetRemoved();

	virtual void OnRootWidgetRemoved_Implementation();

	/**
	*	Called when the input type changes
	*
	*	@param From The input type being used before
	*	@param To The input type being used now
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = UINavController)
	void OnInputChanged(EInputType From, EInputType To);

	virtual void OnInputChanged_Implementation(EInputType From, EInputType To);

	/**
	*	Called when the gamepad's thumbstick moves
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = UINavController)
	void OnThumbstickCursorInput(const FVector2D& ThumbstickDelta);

	virtual void OnThumbstickCursorInput_Implementation(const FVector2D& ThumbstickDelta);

	/**
	*	Called when a controller is connected and disconnected
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = UINavController)
	void OnControllerConnectionChanged(bool bConnected, int32 UserId, int32 UserIndex);

	virtual void OnControllerConnectionChanged_Implementation(bool bConnected, int32 UserId, int32 UserIndex);

	/**
	*	Called when the active widget changes
	*
	*	@param OldActiveWidget The previously active widget
	*	@param NewActiveWidget The new active widget
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = UINavController)
	void OnActiveWidgetChanged(UUINavWidget* OldActiveWidget, UUINavWidget* NewActiveWidget);

	virtual void OnActiveWidgetChanged_Implementation(UUINavWidget* OldActiveWidget, UUINavWidget* NewActiveWidget);

	/**
	*	Called when the player navigates in a certain direction
	*
	*	@param NavigationDirection The direction of navigation
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = UINavController)
	void OnNavigated(EUINavigation NavigationDirection);

	virtual void OnNavigated_Implementation(EUINavigation NavigationDirection);

	/**
	*	Called when the player selects the current option
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = UINavController)
	void OnSelect();

	virtual void OnSelect_Implementation();

	/**
	*	Called when the player returns to the previous widget
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = UINavController)
	void OnReturn();

	virtual void OnReturn_Implementation();

	/**
	*	Called when the player navigates to the next section
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = UINavController)
	void OnNext();

	virtual void OnNext_Implementation();

	/**
	*	Called when the player navigates to the previous section
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = UINavController)
	void OnPrevious();

	virtual void OnPrevious_Implementation();

};
