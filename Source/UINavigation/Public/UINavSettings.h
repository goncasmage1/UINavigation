// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
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
	
	
};
