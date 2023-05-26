// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "InputAction.h"
#include "Data/AxisType.h"
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
	UInputAction* Action = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EnhancedInput)
	EInputAxis Axis = EInputAxis::X;

	/*
	Specifies whether you want this input to be treated as a normal axis or as a positive / negative axis.
	(Only applies to Input Actions with a ValueType different than Boolean).
	If you set it to a value different than none, you should always have a 2nd box with the opposite value.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EnhancedInput)
	EAxisType AxisScale = EAxisType::None;
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
