// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#pragma once
#include "InputAction.h"
#include "InputContainerEnhancedActionData.generated.h"

UENUM(BlueprintType, meta = (ScriptName = "UINavAxis"))
enum class EInputAxis : uint8
{
	X UMETA(DisplayName = "X"),
	Y UMETA(DisplayName = "Y"),
	Z UMETA(DisplayName = "Z")
};

USTRUCT(BlueprintType)
struct FInputContainerEnhancedActionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EnhancedInput)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EnhancedInput)
	UInputAction* Action;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EnhancedInput)
	bool bPositive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EnhancedInput)
	EInputAxis Axis = EInputAxis::X;
};

USTRUCT(BlueprintType)
struct FInputContainerEnhancedActionDataArray
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InputRebindData)
	TArray<int> InputGroups = 
	{
		-1,
	};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EnhancedInput)
	TArray<FInputContainerEnhancedActionData> Actions;
};