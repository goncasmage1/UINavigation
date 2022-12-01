// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UObject/Interface.h"
#include "Data/InputType.h"
#include "Data/NavigationDirection.h"
#include "UINavPCReceiver.generated.h"

class UUINavWidget;

UINTERFACE(MinimalAPI)
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
	*	Called when the root UINavWidget is removed from the viewport
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavController)
	void OnRootWidgetRemoved();

	virtual void OnRootWidgetRemoved_Implementation();

	/**
	*	Called when the input type changes
	*
	*	@param From The input type being used before
	*	@param To The input type being used now
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavController)
	void OnInputChanged(EInputType From, EInputType To);

	virtual void OnInputChanged_Implementation(EInputType From, EInputType To);

	/**
	*	Called when a controller is connected and disconnected
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavController)
	void OnControllerConnectionChanged(EInputDeviceConnectionState InputDeviceConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId);

	virtual void OnControllerConnectionChanged_Implementation(EInputDeviceConnectionState InputDeviceConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId);

	/**
	*	Called when the active widget changes
	*
	*	@param OldActiveWidget The previously active widget
	*	@param NewActiveWidget The new active widget
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavController)
	void OnActiveWidgetChanged(UUINavWidget* OldActiveWidget, UUINavWidget* NewActiveWidget);

	virtual void OnActiveWidgetChanged_Implementation(UUINavWidget* OldActiveWidget, UUINavWidget* NewActiveWidget);

	/**
	*	Called when the player navigates in a certain direction
	*
	*	@param NavigationDirection The direction of navigation
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavController)
	void OnNavigated(ENavigationDirection NavigationDirection);

	virtual void OnNavigated_Implementation(ENavigationDirection NavigationDirection);

	/**
	*	Called when the player selects the current option
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavController)
	void OnSelect();

	virtual void OnSelect_Implementation();

	/**
	*	Called when the player returns to the previous widget
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavController)
	void OnReturn();

	virtual void OnReturn_Implementation();

	/**
	*	Called when the player navigates to the next section
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavController)
	void OnNext();

	virtual void OnNext_Implementation();

	/**
	*	Called when the player navigates to the previous section
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavController)
	void OnPrevious();

	virtual void OnPrevious_Implementation();

};
