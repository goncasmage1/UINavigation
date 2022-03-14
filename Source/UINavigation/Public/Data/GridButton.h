// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once
#include "GridButton.generated.h"

USTRUCT(BlueprintType)
struct FGridButton
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = GridButton)
	int GridIndex = 0;

	UPROPERTY(BlueprintReadWrite, Category = GridButton)
	int IndexInGrid = 0;
};