// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UINavSettings.generated.h"

/**
 * 
 */
UCLASS(config = Input)
class UINAVIGATION_API UUINavSettings : public UObject
{
	GENERATED_BODY()
	
	UPROPERTY(config, EditAnywhere, EditFixedSize, Category = "Bindings", meta = (ToolTip = "List of Axis Properties"), AdvancedDisplay)
		TArray<struct FInputAxisConfigEntry> AxisConfig;
	
	
};
