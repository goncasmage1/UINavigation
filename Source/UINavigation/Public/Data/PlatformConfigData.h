// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "UObject/ObjectMacros.h"
#include "PlatformConfigData.generated.h"

class UDataTable;
class UInputMappingContext;

USTRUCT(BlueprintType)
struct FPlatformConfigData
{
	GENERATED_BODY()

	FPlatformConfigData() {}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlatformConfigData, meta = (RequiredAssetDataTags = "RowStructure=/Script/UINavigation.InputIconMapping"))
	UDataTable* GamepadKeyIconData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlatformConfigData, meta = (RequiredAssetDataTags = "RowStructure=/Script/UINavigation.InputNameMapping"))
	UDataTable* GamepadKeyNameData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlatformConfigData)
	UInputMappingContext* UINavInputContextOverride = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlatformConfigData)
	bool bCanUseKeyboardMouse = true;

};