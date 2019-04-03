// Copyright (C) 2018 Gonçalo Marques - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "UINavButton.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomHoverDelegate, int, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomUnhoverDelegate, int, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomClickDelegate, int, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomReleaseDelegate, int, Index);


UENUM(BlueprintType)
enum class EGridType : uint8
{
	Horizontal UMETA(DisplayName = "Horizontal"),
	Vertical UMETA(DisplayName = "Vertical"),
	Grid2D UMETA(DisplayName = "Grid2D")
};

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

USTRUCT(BlueprintType)
struct FGrid
{
	GENERATED_BODY()

	FGrid()
	{

	}

	FGrid(EGridType NewGridType, class UUINavButton* NewFirstButton, int NewGridIndex, int NewDimensionX, int NewDimensionY, FButtonNavigation NewEdgeNavigation, bool bShouldWrap)
	{
		GridType = NewGridType;
		FirstButton = NewFirstButton;
		GridIndex = NewGridIndex;
		DimensionX = NewDimensionX;
		DimensionY = NewDimensionY;
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

};

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavButton : public UButton
{
	GENERATED_BODY()
	
public:

	UUINavButton();

	UPROPERTY()
		FCustomHoverDelegate CustomHover;
	UPROPERTY()
		FCustomUnhoverDelegate CustomUnhover;
	UPROPERTY()
		FCustomUnhoverDelegate CustomClick;
	UPROPERTY()
		FCustomReleaseDelegate CustomRelease;

	/*
	WARNING: Edit this index manually at your own risk. If the indices are
	not correctly alligned with the automatically set indices several bugs may appear.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavButton)
		int ButtonIndex = -1;

	//The index of the grid this button is in
	UPROPERTY(BlueprintReadOnly, Category = UINavButton)
		int GridIndex = - 1;
	//This button's index in its associated grid
	UPROPERTY(BlueprintReadOnly, Category = UINavButton)
		int IndexInGrid = - 1;

	bool bSwitchedStyle = false;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		bool IsValid();

	UFUNCTION()
		void OnHover();
	UFUNCTION()
		void OnUnhover();
	UFUNCTION()
		void OnClick();
	UFUNCTION()
		void OnRelease();
	
};
