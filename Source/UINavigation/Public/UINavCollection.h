// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "Data/Grid.h"
#include "UINavCollection.generated.h"

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavCollection : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, Category = UINavCollection)
		void SetupNavigation(const TArray<FButtonNavigation>& EdgeNavigations);
	virtual void SetupNavigation_Implementation(const TArray<FButtonNavigation>& EdgeNavigations);

	void Init(int StartIndex);

	void TraverseHierarquy(int StartIndex);

	//The index of the first grid in this Collection
	UPROPERTY(BlueprintReadOnly, Category = UINavCollection)
		int FirstGridIndex = -1;

	//The number of grids in this Collection
	UPROPERTY(BlueprintReadOnly, Category = UINavCollection)
		int GridCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = UINavCollection)
		class UUINavWidget* ParentWidget;

	UPROPERTY(BlueprintReadOnly, Category = UINavCollection)
		class UUINavCollection* ParentCollection;

	UPROPERTY(BlueprintReadOnly, Category = UINavCollection)
		TArray<class UUINavButton*> UINavButtons;

	UPROPERTY(BlueprintReadOnly, Category = UINavCollection)
		TArray<class UUINavComponent*> UINavComponents;

	UPROPERTY(BlueprintReadOnly, Category = UINavCollection)
		TArray<class UUINavHorizontalComponent*> UINavHorizontalComps;

	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<class UUINavCollection*> UINavCollections;

	int CollectionIndex = 0;

	UFUNCTION(BlueprintCallable, Category = UINavCollection)
		void AppendNavigationGrid1D(EGridType GridType, int Dimension, FButtonNavigation EdgeNavigation, bool bWrap);

	UFUNCTION(BlueprintCallable, Category = UINavCollection, meta=(AdvancedDisplay=4))
		void AppendNavigationGrid2D(int DimensionX, int DimensionY, FButtonNavigation EdgeNavigation, bool bWrap, int ButtonsInGrid = -1);

	UFUNCTION(BlueprintCallable, Category = UINavCollection)
		void AppendCollection(TArray<FButtonNavigation> EdgeNavigations);

	void IncrementGrids();

	//Returns a reference to the grid in this collection at the specified index
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavCollection)
		void GetGridAtIndex(int GridIndex, FGrid& Grid, bool& bIsValid);
	
};
