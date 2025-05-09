// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "InputCoreTypes.h"
#include "Internationalization/Text.h"
#include "InputCollisionData.generated.h"

USTRUCT(BlueprintType)
struct FInputCollisionData
{
	GENERATED_BODY()

	FInputCollisionData()
	{
	}

	FInputCollisionData(const FText InCurrentInputText,
						const FText InCollidingInputText,
						const int InCurrentKeyIndex,
						const int InCollidingKeyIndex,
						const FKey InCurrentInputKey,
						const FKey InPressedKey) :
		CurrentInputText(InCurrentInputText),
		CollidingInputText(InCollidingInputText),
		CurrentKeyIndex(InCurrentKeyIndex),
		CollidingKeyIndex(InCollidingKeyIndex),
		CurrentInputKey(InCurrentInputKey),
		PressedKey(InPressedKey)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InputCollisionData)
	FText CurrentInputText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InputCollisionData)
	FText CollidingInputText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InputCollisionData)
	int CurrentKeyIndex = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InputCollisionData)
	int CollidingKeyIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InputCollisionData)
	FKey CurrentInputKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InputCollisionData)
	FKey PressedKey;

};