// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "InputAction.h"
#include "Data/AxisType.h"
#include "Data/InputHoldBehavior.h"
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

	// How this input action rebind should react to hold inputs: Don't allow hold input, allow hold if the key is held, or force hold even if the key isn't held.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EnhancedInput)
	EInputHoldBehavior HoldBehavior = EInputHoldBehavior::DontAllow;

	// Specify an action that is supposed to be bound to the same key as the current input action, so that it automatically gets updated during rebinding.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EnhancedInput)
	UInputAction* InputActionToUpdate = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InputRebindData)
	TArray<int> InputGroupsOverride;
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
