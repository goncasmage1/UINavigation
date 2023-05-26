// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/InputRestriction.h"
#include "UINavBlueprintFunctionLibrary.generated.h"

class UInputAction;
class UPromptDataBinary;
class UWidget;

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

	UFUNCTION(BlueprintCallable, Category = UINavigationLibrary)
	static UPromptDataBinary* CreateBinaryPromptData(const bool bAccept);
	
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

	// Returns the child of the given panel widget at the given index
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
	static UWidget* GetPanelWidgetChild(const UWidget* const Widget, const int ChildIndex);

	// Returns the child of the given uniform grid panel widget with the given column and row
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
	static UWidget* GetUniformGridChild(const UWidget* const Widget, const int Column, const int Row);

	// Returns the index of the given widget in its parent panel widget
	// Will assume UPanelWidget if PanelWidgetSubclass is null
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
	static int GetIndexInPanelWidget(const UWidget* const Widget, TSubclassOf<UPanelWidget> PanelWidgetSubclass);

	// Returns the first parent of this widget thats a Panel Widget of the specified class
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
	static UPanelWidget* GetParentPanelWidget(const UWidget* const Widget, TSubclassOf<UPanelWidget> PanelWidgetSubclass);

	// Returns the index of the given widget in its parent panel widget
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
	static void GetIndexInUniformGridWidget(const UWidget* const Widget, int& Column, int& Row);

	UFUNCTION(BlueprintPure, Category = UINavigationLibrary)
	static bool IsVRKey(const FKey Key);

	UFUNCTION(BlueprintPure, Category = UINavigationLibrary)
	static bool IsKeyInCategory(const FKey Key, const FString Category);
	
};
