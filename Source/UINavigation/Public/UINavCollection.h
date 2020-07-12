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

	UFUNCTION(BlueprintNativeEvent, Category = UINavCollection)
		void PreSetup();
	virtual void PreSetup_Implementation();

	void NotifyOnNavigate(int From, int To, int LocalFrom, int LocalTo);

	/**
	*	Called when the button with the specified index was navigated upon
	*	If the LocalTo or LocalFrom index don't belong to this collection,
	*	they will be -1.
	*
	*	@param	From  The global index of the button that was navigated from
	*	@param	To  The global index of the button that was navigated to
	*	@param	From  The local index of the button that was navigated from
	*	@param	To  The local index of the button that was navigated to
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavCollection)
		void OnNavigate(int From, int To, int LocalFrom, int LocalTo);
	virtual void OnNavigate_Implementation(int From, int To, int LocalFrom, int LocalTo);

	/**
	*	Notifies that a button was selected, and indicates its index
	*
	*	@param	Index  The global index of the button that was selected
	*	@param	Index  The local index of the button that was selected
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavCollection)
		void OnSelect(int Index, int LocalIndex);
	virtual void OnSelect_Implementation(int Index, int LocalIndex);

	UFUNCTION(BlueprintNativeEvent, Category = UINavCollection)
		void OnStartSelect(int Index, int LocalIndex);
	virtual void OnStartSelect_Implementation(int Index, int LocalIndex);

	UFUNCTION(BlueprintNativeEvent, Category = UINavCollection)
		void OnStopSelect(int Index, int LocalIndex);
	virtual void OnStopSelect_Implementation(int Index, int LocalIndex);

	void NotifyOnSelect(int Index, int LocalIndex);
	void NotifyOnStartSelect(int Index, int LocalIndex);
	void NotifyOnStopSelect(int Index, int LocalIndex);

	void Init(int StartIndex);

	void TraverseHierarquy(int StartIndex);

	void CallCustomInput(FName ActionName, uint8* Buffer);

	UPROPERTY(BlueprintReadOnly, Category = UINavCollection)
		int FirstButtonIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = UINavCollection)
		int LastButtonIndex = -1;

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

	UPROPERTY(BlueprintReadWrite, Category = UINavCollection)
		TArray<class UWidgetAnimation*> UINavAnimations;

	UPROPERTY(BlueprintReadOnly, Category = UINavCollection)
		TArray<class UUINavCollection*> UINavCollections;

	int CollectionIndex = 0;

	UFUNCTION(BlueprintCallable, Category = UINavCollection)
		void AppendNavigationGrid1D(EGridType GridType, int Dimension, FButtonNavigation EdgeNavigation, bool bWrap);

	UFUNCTION(BlueprintCallable, Category = UINavCollection, meta=(AdvancedDisplay=4))
		void AppendNavigationGrid2D(int DimensionX, int DimensionY, FButtonNavigation EdgeNavigation, bool bWrap, int ButtonsInGrid = -1);

	UFUNCTION(BlueprintCallable, Category = UINavCollection)
		void AppendCollection(TArray<FButtonNavigation> EdgeNavigations);

	void IncrementGrids(int Dimension);
	void UpdateCollectionLastIndex(int GridIndex, bool bAdded);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavCollection)
		int GetGlobalGridIndex(int GridIndex);

	//Returns a reference to the grid in this collection at the specified index
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavCollection)
		void GetGridAtIndex(int GridIndex, FGrid& Grid, bool& bIsValid);

	UUINavCollection* GetCollectionByIndex(int Index);
	
};
