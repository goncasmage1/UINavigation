// Copyright (C) 2018 Gonçalo Marques - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UINavBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "UINav Sound")
		static void SetSoundClassVolume(class USoundClass* TargetClass, float NewVolume);
	UFUNCTION(BlueprintPure, Category = "UINav Sound")
		static float GetSoundClassVolume(class USoundClass* TargetClass);
	
	UFUNCTION(BlueprintCallable, Category = "UINav Graphics")
		static void SetPostProcessSettings(FString Variable, FString Value);
	UFUNCTION(BlueprintPure, Category = "UINav Graphics")
		static FString GetPostProcessSettings(FString Variable);

	//Resets the input settings to their default state
	UFUNCTION(BlueprintCallable, Category = "UINav Input")
		static void ResetInputSettings();
	
};
