// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "Engine/DataTable.h"
#include "InputIconMapping.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FInputIconMapping : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UINav Input")
	TSoftObjectPtr<class UTexture2D> InputIcon;
};