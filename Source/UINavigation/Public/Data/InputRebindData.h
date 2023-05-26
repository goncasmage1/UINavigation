// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "Engine/DataTable.h"
#include "InputRebindData.generated.h"

USTRUCT(BlueprintType)
struct FInputRebindData : public FTableRowBase
{
	GENERATED_BODY()

	FInputRebindData()
	{

	}

	FInputRebindData(const FText InInputText, const TArray<int>& InInputGroups)
	{
		InputText = InInputText;
		InputGroups = InInputGroups;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InputRebindData)
	FText InputText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InputRebindData)
	TArray<int> InputGroups = 
	{
		-1,
	};

};