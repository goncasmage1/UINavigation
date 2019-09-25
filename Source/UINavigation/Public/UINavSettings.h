// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "GameFramework/PlayerInput.h"
#include "UINavSettings.generated.h"

/**
 * 
 */
UCLASS(config = UINavInput)
class UINAVIGATION_API UUINavSettings : public UObject
{
	GENERATED_BODY()
	
public:

	UPROPERTY(config, EditAnywhere, Category = "Bindings")
		TArray<struct FInputActionKeyMapping> ActionMappings;

	UPROPERTY(config, EditAnywhere, Category = "Bindings")
		TArray<struct FInputAxisKeyMapping> AxisMappings;
	
	
};
