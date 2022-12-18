// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once
#include "GridType.h"
#include "ButtonNavigation.h"
#include "UINavButton.h"
#include "Grid.generated.h"

USTRUCT(BlueprintType)
struct FGrid
{
	GENERATED_BODY()

	FGrid()
	{

	}

	FGrid(
		const EGridType NewGridType,
		UUINavButton* NewFirstButton,
		const int NewGridIndex,
		const int NewDimensionX,
		const int NewDimensionY,
		const FButtonNavigation NewEdgeNavigation,
		const bool bShouldWrap,
		const int NewNum2DButtons = -1)
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

	void SetEdgeNavigation(const FButtonNavigation NewEdgeNavigation)
	{
		EdgeNavigation = NewEdgeNavigation;
	}

	void SetEdgeNavigationByButton(const FButtonNavigation NewEdgeNavigation)
	{
		if (NewEdgeNavigation.LeftButton != nullptr)
		{
			EdgeNavigation.LeftButton = NewEdgeNavigation.LeftButton;
		}
		if (NewEdgeNavigation.RightButton != nullptr)
		{
			EdgeNavigation.RightButton = NewEdgeNavigation.RightButton;
		}
		if (NewEdgeNavigation.UpButton != nullptr)
		{
			EdgeNavigation.UpButton = NewEdgeNavigation.UpButton;
		}
		if (NewEdgeNavigation.DownButton != nullptr)
		{
			EdgeNavigation.DownButton = NewEdgeNavigation.DownButton;
		}
	}

	void RemoveButtonFromEdgeNavigation(UUINavButton* OldButton)
	{
		if (EdgeNavigation.LeftButton == OldButton)
			EdgeNavigation.LeftButton = nullptr;

		if (EdgeNavigation.RightButton == OldButton)
			EdgeNavigation.RightButton = nullptr;

		if (EdgeNavigation.UpButton == OldButton)
			EdgeNavigation.UpButton = nullptr;

		if (EdgeNavigation.DownButton == OldButton)
			EdgeNavigation.DownButton = nullptr;
	}

	void RemoveGridFromEdgeNavigation(const int InGridIndex)
	{
		if (EdgeNavigation.LeftButton != nullptr &&
			EdgeNavigation.LeftButton->GridIndex == InGridIndex)
			EdgeNavigation.LeftButton = nullptr;

		if (EdgeNavigation.RightButton != nullptr &&
			EdgeNavigation.RightButton->GridIndex == InGridIndex)
			EdgeNavigation.RightButton = nullptr;

		if (EdgeNavigation.UpButton != nullptr &&
			EdgeNavigation.UpButton->GridIndex == InGridIndex)
			EdgeNavigation.UpButton = nullptr;

		if (EdgeNavigation.DownButton != nullptr &&
			EdgeNavigation.DownButton->GridIndex == InGridIndex)
			EdgeNavigation.DownButton = nullptr;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ButtonGrid)
	EGridType GridType = EGridType::Horizontal;

	UPROPERTY(BlueprintReadWrite, Category = ButtonGrid)
	UUINavButton* FirstButton = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ButtonGrid)
	FButtonNavigation EdgeNavigation;

	UPROPERTY(BlueprintReadWrite, Category = ButtonGrid)
	bool bWrap = false;

	UPROPERTY(BlueprintReadWrite, Category = ButtonGrid)
	int GridIndex = -1;

	UPROPERTY(BlueprintReadWrite, Category = ButtonGrid)
	int DimensionX = -1;

	UPROPERTY(BlueprintReadWrite, Category = ButtonGrid)
	int DimensionY = -1;

	UPROPERTY(BlueprintReadWrite, Category = ButtonGrid)
	int NumGrid2DButtons = -1;

};