// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "Data/PromptData.h"
#include "Data/InputCollisionData.h"
#include "PromptDataSwapKeys.generated.h"

class UUINavInputBox;

UCLASS(BlueprintType, Blueprintable)
class UPromptDataSwapKeys : public UPromptDataBase
{
	GENERATED_BODY()

public:
	UPromptDataSwapKeys() {}

	UPromptDataSwapKeys(const bool bShouldSwap) : bShouldSwap(bShouldSwap) {}

	UPROPERTY(BlueprintReadWrite, Category = "Swap Keys Prompt Data")
	bool bShouldSwap = true;

	UPROPERTY(BlueprintReadWrite, Category = "Swap Keys Prompt Data")
	FInputCollisionData InputCollisionData;

	UPROPERTY(BlueprintReadWrite, Category = "Swap Keys Prompt Data")
	UUINavInputBox* CurrentInputBox = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Swap Keys Prompt Data")
	UUINavInputBox* CollidingInputBox = nullptr;

};