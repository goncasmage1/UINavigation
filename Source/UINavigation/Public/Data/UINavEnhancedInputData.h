// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#pragma once
#include "InputAction.h"
#include "UINavEnhancedInputData.generated.h"

USTRUCT(BlueprintType)
struct FUINavEnhancedInputData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = EnhancedInput)
	UInputAction* IA_MenuUp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = EnhancedInput)
	UInputAction* IA_MenuDown;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = EnhancedInput)
	UInputAction* IA_MenuLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = EnhancedInput)
	UInputAction* IA_MenuRight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = EnhancedInput)
	UInputAction* IA_MenuSelect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = EnhancedInput)
	UInputAction* IA_MenuReturn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = EnhancedInput)
	UInputAction* IA_MenuNext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = EnhancedInput)
	UInputAction* IA_MenuPrevious;
};