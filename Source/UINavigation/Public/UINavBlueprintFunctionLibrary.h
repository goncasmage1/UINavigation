// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/Grid.h"
#include "Data/InputRestriction.h"
#include "UINavButton.h"
#include "UINavBlueprintFunctionLibrary.generated.h"

class UInputAction;

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = UINavigationLibrary)
	static void SetSoundClassVolume(class USoundClass* TargetClass, const float NewVolume);

	UFUNCTION(BlueprintPure, Category = UINavigationLibrary)
	static float GetSoundClassVolume(class USoundClass* TargetClass);
	
	UFUNCTION(BlueprintCallable, Category = UINavigationLibrary)
	static void SetPostProcessSettings(const FString Variable, const FString Value);

	UFUNCTION(BlueprintPure, Category = UINavigationLibrary)
	static FString GetPostProcessSettings(const FString Variable);

	// Resets the input settings to their default state
	UFUNCTION(BlueprintCallable, Category = UINavInput)
	static void ResetInputSettings(APlayerController* PC = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINav Input")
	static bool RespectsRestriction(const FKey CompareKey, const EInputRestriction Restriction);

	// Checks whether a gamepad is connected
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavigationLibrary)
	static bool IsGamepadConnected();

	UFUNCTION(BlueprintCallable, Category = UINavInput)
	static bool IsUINavInputAction(const UInputAction* const InputAction);
	
	template <typename T>
	static bool ContainsArray(const TArray<T>& Array1, const TArray<T>& Array2)
	{
		if (Array1.Num() < Array2.Num()) return false;

		for (int i = 0; i < Array2.Num(); ++i)
		{
			if (Array1[i] != Array2[i]) return false;
		}

		return true;
	}

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
	static int Conv_GridToInt(const FGrid Grid);

	UFUNCTION(BlueprintPure, Category = UINavigationLibrary)
	static bool IsVRKey(const FKey Key);

	UFUNCTION(BlueprintPure, Category = UINavigationLibrary)
	static bool IsKeyInCategory(const FKey Key, const FString Category);
	
};
