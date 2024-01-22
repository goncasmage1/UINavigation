// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "Containers/Array.h"
#include "Data/UINavEnhancedActionKeyMapping.h"
#include "InputMappingArray.generated.h"

USTRUCT(BlueprintType)
struct FInputMappingArray
{
	GENERATED_BODY()

	FInputMappingArray() {}

	FInputMappingArray(const TArray<FEnhancedActionKeyMapping>& InInputMappings)
	: InputMappings(InInputMappings) {}

	UPROPERTY()
	TArray<FUINavEnhancedActionKeyMapping> InputMappings;
};