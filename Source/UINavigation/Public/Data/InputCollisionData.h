// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once
#include "InputCollisionData.generated.h"

USTRUCT(BlueprintType)
struct FInputCollisionData
{
	GENERATED_BODY()

	FInputCollisionData()
	{
	}

	FInputCollisionData(FText InCurrentInputText,
						FText InCollidingInputText,
						int InCollidingKeyIndex,
						FKey InCurrentInputKey,
						FKey InPressedKey) :
		CurrentInputText(InCurrentInputText),
		CollidingInputText(InCollidingInputText),
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
	int CollidingKeyIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InputCollisionData)
	FKey CurrentInputKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InputCollisionData)
	FKey PressedKey;

};