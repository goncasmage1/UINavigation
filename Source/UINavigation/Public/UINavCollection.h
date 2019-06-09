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

	UPROPERTY(BlueprintReadOnly, Category = UINavCollection)
		class UUINavCollection* ParentCollection;

	UPROPERTY(BlueprintReadOnly, Category = UINavCollection)
		class UUINavWidget* ParentWidget;

	UPROPERTY(BlueprintReadOnly, Category = UINavCollection)
		TArray<FGrid> NavigationGrids;

	UPROPERTY(BlueprintReadOnly, Category = UINavCollection)
		TArray<class UUINavButton*> UINavButtons;

	UPROPERTY(BlueprintReadWrite, Category = UINavCollection)
		TArray<class UWidgetAnimation*> UINavAnimations;

	UPROPERTY(BlueprintReadOnly, Category = UINavCollection)
		TArray<class UUINavComponent*> UINavComponents;

	UPROPERTY(BlueprintReadOnly, Category = UINavCollection)
		TArray<class UUINavComponentBox*> UINavComponentBoxes;

	UPROPERTY(BlueprintReadOnly, Category = UINavCollection)
		TArray<class UUINavInputBox*> UINavInputBoxes;

	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<class UUINavCollection*> UINavCollections;

	int CollectionIndex = 0;

	UFUNCTION(BlueprintCallable, Category = UINavCollection)
		void AppendNavigationGrid1D(EGridType GridType, int Dimension, FButtonNavigation EdgeNavigation, bool bWrap);

	UFUNCTION(BlueprintCallable, Category = UINavCollection, meta=(AdvancedDisplay=4))
		void AppendNavigationGrid2D(int DimensionX, int DimensionY, FButtonNavigation EdgeNavigation, bool bWrap, int ButtonsInGrid = -1);

	UFUNCTION(BlueprintCallable, Category = UINavCollection)
		void AppendCollection(TArray<FButtonNavigation> EdgeNavigations);
	
};
