// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once
#include "UObject/Object.h"
#include "PromptData.generated.h"

UCLASS()
class UINAVIGATION_API UPromptDataBase : public UObject
{
	GENERATED_BODY()

public:
	UPromptDataBase() {}
};

UCLASS()
class UPromptDataBinary : public UPromptDataBase
{
	GENERATED_BODY()

public:
	UPromptDataBinary() {}

	UPromptDataBinary(const bool bInAccept) : bAccept(bInAccept) {}

	UPROPERTY(BlueprintReadOnly, Category = "Prompt Data")
	bool bAccept = true;

};