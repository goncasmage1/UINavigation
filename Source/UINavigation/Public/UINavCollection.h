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

	UFUNCTION(BlueprintNativeEvent, Category = UINavCollection)
		void OnReturn();
	virtual void OnReturn_Implementation();

	/**
	*	Called when the button with the specified index was navigated upon
	*	If the LocalTo or LocalFrom index don't belong to this collection,
	*	they will be -1.
	*
	*	@param	From  The global index of the button that was navigated from
	*	@param	To  The global index of the button that was navigated to
	*	@param	LocalFrom  The local index of the button that was navigated from
	*	@param	LocalTo  The local index of the button that was navigated to
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavCollection)
		void OnNavigate(int From, int To, int LocalFrom, int LocalTo);
	virtual void OnNavigate_Implementation(int From, int To, int LocalFrom, int LocalTo);

	/**
	*	Notifies that a button was selected, and indicates its index
	*
	*	@param	Index  The global index of the button that was selected
	*	@param	LocalIndex  The local index of the button that was selected
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

	void NotifyOnReturn();

	void Init(int StartIndex);

	void IncrementGridCount();
	void SetLastButtonIndex(const int LastButtonIndex);

	void CallCustomInput(const FName ActionName, uint8* Buffer);

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
		void AppendNavigationGrid1D(const EGridType GridType, const int Dimension, const FButtonNavigation EdgeNavigation, const bool bWrap);

	UFUNCTION(BlueprintCallable, Category = UINavCollection, meta=(AdvancedDisplay=4))
		void AppendNavigationGrid2D(const int DimensionX, const int DimensionY, const FButtonNavigation EdgeNavigation, const bool bWrap, const int ButtonsInGrid = -1);

	UFUNCTION(BlueprintCallable, Category = UINavCollection)
		void AppendCollection(const TArray<FButtonNavigation> EdgeNavigations);

	void IncrementGrids(const int Dimension);
	void UpdateCollectionLastIndex(const int GridIndex, const bool bAdded);

	UFUNCTION(BlueprintCallable, Category = UINavCollection)
		void SetEdgeNavigation(const int GridIndex, const FButtonNavigation NewEdgeNavigation);

	UFUNCTION(BlueprintCallable, Category = UINavCollection)
		void SetEdgeNavigationByButton(const int GridIndex, const FButtonNavigation NewEdgeNavigation);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavCollection)
		int GetGlobalGridIndex(const int GridIndex);

	//Returns a reference to the grid in this collection at the specified index
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavCollection)
		void GetGridAtIndex(const int GridIndex, FGrid& Grid, bool& bIsValid);

	UUINavCollection* GetCollectionByIndex(const int Index);
	
};
