// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once
#include "GridType.h"
#include "ButtonNavigation.h"
#include "Grid.generated.h"

USTRUCT(BlueprintType)
struct FGrid
{
	GENERATED_BODY()

	FGrid()
	{

	}

	FGrid(EGridType NewGridType, class UUINavButton* NewFirstButton, int NewGridIndex, int NewDimensionX, int NewDimensionY, FButtonNavigation NewEdgeNavigation, bool bShouldWrap, int NewNum2DButtons = -1)
	{
		GridType = NewGridType;
		FirstButton = NewFirstButton;
		GridIndex = NewGridIndex;
		DimensionX = NewDimensionX;
		DimensionY = NewDimensionY;
		if (NewNum2DButtons < 0 || NewNum2DButtons > DimensionX * DimensionY)
			NumGrid2DButtons = DimensionX * DimensionY;
		else NumGrid2DButtons = NewNum2DButtons;
		EdgeNavigation = NewEdgeNavigation;
		bWrap = bShouldWrap;
	}

	int GetDimension() const;

	int GetLastButtonIndex() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ButtonGrid)
		EGridType GridType = EGridType::Horizontal;

	UPROPERTY(BlueprintReadOnly, Category = ButtonGrid)
		class UUINavButton* FirstButton = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ButtonGrid)
		FButtonNavigation EdgeNavigation;

	UPROPERTY(BlueprintReadWrite, Category = ButtonGrid)
		bool bWrap = false;

	UPROPERTY(BlueprintReadOnly, Category = ButtonGrid)
		int GridIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = ButtonGrid)
		int DimensionX = -1;
	UPROPERTY(BlueprintReadOnly, Category = ButtonGrid)
		int DimensionY = -1;

	UPROPERTY(BlueprintReadOnly, Category = ButtonGrid)
		int NumGrid2DButtons = -1;

};