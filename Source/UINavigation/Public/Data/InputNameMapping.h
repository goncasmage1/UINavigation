// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once
#include "Engine/DataTable.h"
#include "InputNameMapping.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FInputNameMapping : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UINav Input")
		FText InputText;
};