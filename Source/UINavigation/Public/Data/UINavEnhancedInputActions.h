// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Engine/DataAsset.h"
#include "UINavEnhancedInputActions.generated.h"

class UInputAction;

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavEnhancedInputActions : public UDataAsset
{
	GENERATED_BODY()

public:

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
