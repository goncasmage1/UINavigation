// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/Grid.h"
#include "Data/InputRestriction.h"
#include "UINavButton.h"
#include "UINavBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = UINavigationLibrary)
		static void SetSoundClassVolume(class USoundClass* TargetClass, float NewVolume);
	UFUNCTION(BlueprintPure, Category = UINavigationLibrary)
		static float GetSoundClassVolume(class USoundClass* TargetClass);
	
	UFUNCTION(BlueprintCallable, Category = UINavigationLibrary)
		static void SetPostProcessSettings(FString Variable, FString Value);
	UFUNCTION(BlueprintPure, Category = UINavigationLibrary)
		static FString GetPostProcessSettings(FString Variable);

	// Resets the input settings to their default state
	UFUNCTION(BlueprintCallable, Category = UINavInput)
		static void ResetInputSettings();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINav Input")
		static bool RespectsRestriction(FKey CompareKey, EInputRestriction Restriction);

	// Checks whether a gamepad is connected
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavigationLibrary)
		static bool IsGamepadConnected();

	// Returns the desired grid's dimension
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		static int GetGridDimension(const FGrid Grid);

	// Returns whether the given button is valid (isn't hidden, collaped or disabled)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		static bool IsButtonValid(UUINavButton* Button);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "UINavButton To Index", CompactNodeTitle = "->", BlueprintAutocast), Category = "UINavButton")
		static int Conv_UINavButtonToInt(UUINavButton* Button);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "UINavComponent To Index", CompactNodeTitle = "->", BlueprintAutocast), Category = "UINavComponent")
		static int Conv_UINavComponentToInt(class UUINavComponent* Component);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "UINavComponent To UINavButton", CompactNodeTitle = "->", BlueprintAutocast), Category = "UINavComponent")
		static UUINavButton* Conv_UINavComponentToUINavButton(class UUINavComponent* Component);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Grid To Index", CompactNodeTitle = "->", BlueprintAutocast), Category = "Navigation Grid")
		static int Conv_GridToInt(FGrid Grid);
	
};
