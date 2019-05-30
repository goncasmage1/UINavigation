// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once
#include "ButtonNavigation.generated.h"

USTRUCT(BlueprintType)
struct FButtonNavigation
{
	GENERATED_BODY()

	FButtonNavigation()
	{

	}

	FButtonNavigation(class UUINavButton* NewUp, class UUINavButton* NewDown, class UUINavButton* NewLeft, class UUINavButton* NewRight)
	{
		UpButton = NewUp;
		DownButton = NewDown;
		LeftButton = NewLeft;
		RightButton = NewRight;
	}

	UPROPERTY(BlueprintReadWrite, Category = ButtonNavigation)
		class UUINavButton* UpButton = nullptr;
	UPROPERTY(BlueprintReadWrite, Category = ButtonNavigation)
		class UUINavButton* DownButton = nullptr;
	UPROPERTY(BlueprintReadWrite, Category = ButtonNavigation)
		class UUINavButton* LeftButton = nullptr;
	UPROPERTY(BlueprintReadWrite, Category = ButtonNavigation)
		class UUINavButton* RightButton = nullptr;
};