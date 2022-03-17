// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved


#include "UINavCollection.h"
#include "UINavWidget.h"
#include "UINavMacros.h"

void UUINavCollection::SetupNavigation_Implementation(const TArray<FButtonNavigation>& EdgeNavigations)
{
}

void UUINavCollection::PreSetup_Implementation()
{
}

void UUINavCollection::OnReturn_Implementation()
{
}

void UUINavCollection::OnNavigate_Implementation(int From, int To, int LocalFrom, int LocalTo)
{
}

void UUINavCollection::OnSelect_Implementation(int Index, int LocalIndex)
{
}

void UUINavCollection::OnStartSelect_Implementation(int Index, int LocalIndex)
{
}

void UUINavCollection::OnStopSelect_Implementation(int Index, int LocalIndex)
{
}

void UUINavCollection::NotifyOnReturn()
{
	for (UUINavCollection* Collection : UINavCollections)
	{
		Collection->NotifyOnReturn();
	}
	
	OnReturn();
}

void UUINavCollection::Init(int StartIndex)
{
	FirstButtonIndex = StartIndex;
	LastButtonIndex = StartIndex - 1;
	PreSetup();

	UUINavWidget::TraverseHierarquy(ParentWidget, this);

	if (UINavAnimations.Num() > 0)
	{
		if (ParentCollection != nullptr)
		{
			const int Index = FirstButtonIndex - ParentCollection->FirstButtonIndex;
			if (ParentCollection->UINavAnimations.Num() >= Index)
			{
				ParentCollection->UINavAnimations.Insert(UINavAnimations, Index);
			}
			UINavAnimations.Empty();
		}
		else if (ParentWidget != nullptr)
		{
			const int Index = FirstButtonIndex;
			if (ParentWidget->UINavAnimations.Num() >= Index)
			{
				ParentWidget->UINavAnimations.Insert(UINavAnimations, Index);
			}
			UINavAnimations.Empty();
		}
	}
}

void UUINavCollection::IncrementGridCount()
{
	GridCount++;
	if (ParentCollection != nullptr)
	{
		ParentCollection->IncrementGridCount();
	}
}

void UUINavCollection::SetLastButtonIndex(const int InLastButtonIndex)
{
	LastButtonIndex = InLastButtonIndex;
	if (ParentCollection != nullptr)
	{
		ParentCollection->SetLastButtonIndex(InLastButtonIndex);
	}
}

void UUINavCollection::CallCustomInput(const FName ActionName, uint8* Buffer)
{
	UFunction* CustomFunction = FindFunction(ActionName);
	if (CustomFunction != nullptr)
	{
		if (CustomFunction->ParmsSize == sizeof(bool))
		{
			ProcessEvent(CustomFunction, Buffer);
		}
		else
		{
			DISPLAYERROR(FString::Printf(TEXT("%s Custom Event should have one boolean parameter!"), *ActionName.ToString()));
		}
	}

	for (UUINavCollection* Collection : UINavCollections)
	{
		Collection->CallCustomInput(ActionName, Buffer);
	}
}

void UUINavCollection::AppendNavigationGrid1D(const EGridType GridType, const int Dimension, const FButtonNavigation EdgeNavigation, const bool bWrap)
{
	ParentWidget->AppendNavigationGrid1D(GridType, Dimension, EdgeNavigation, bWrap);
	IncrementGrids(Dimension);
}

void UUINavCollection::AppendNavigationGrid2D(const int DimensionX, const int DimensionY, const FButtonNavigation EdgeNavigation, const bool bWrap, const int ButtonsInGrid)
{
	ParentWidget->AppendNavigationGrid2D(DimensionX, DimensionY, EdgeNavigation, bWrap, ButtonsInGrid);
	IncrementGrids((ButtonsInGrid == -1 ? (DimensionX * DimensionY) : ButtonsInGrid));
}

void UUINavCollection::AppendCollection(const TArray<FButtonNavigation> EdgeNavigations)
{
	if (CollectionIndex >= UINavCollections.Num())
	{
		DISPLAYERROR("Can't append UINavCollection to navigation, no remaining UINavCollection found!");
		return;
	}

	UUINavCollection* Collection = UINavCollections[CollectionIndex];
	Collection->FirstGridIndex = FirstGridIndex + GridCount;
	Collection->SetupNavigation(EdgeNavigations);
	if (Collection->LastButtonIndex < Collection->FirstButtonIndex)
	{
		Collection->LastButtonIndex = Collection->FirstButtonIndex;
	}
	CollectionIndex++;
}

void UUINavCollection::IncrementGrids(const int Dimension)
{
	GridCount++;
	LastButtonIndex += Dimension;
	if (ParentCollection != nullptr) ParentCollection->IncrementGrids(Dimension);
}

void UUINavCollection::UpdateCollectionLastIndex(const int GridIndex, const bool bAdded)
{
	for (UUINavCollection* Collection : UINavCollections)
	{
		if (Collection->FirstGridIndex >= GridIndex &&
			Collection->FirstGridIndex + Collection->GridCount <= GridIndex)
		{
			Collection->UpdateCollectionLastIndex(GridIndex, bAdded);
			break;
		}
	}
	LastButtonIndex--;
}

void UUINavCollection::SetEdgeNavigation(const int GridIndex, const FButtonNavigation NewEdgeNavigation)
{
	ParentWidget->SetEdgeNavigation(FirstGridIndex + GridIndex, NewEdgeNavigation);
}

void UUINavCollection::SetEdgeNavigationByButton(const int GridIndex, const FButtonNavigation NewEdgeNavigation)
{
	ParentWidget->SetEdgeNavigationByButton(FirstGridIndex + GridIndex, NewEdgeNavigation);
}

int UUINavCollection::GetGlobalGridIndex(const int GridIndex)
{
	return FirstGridIndex + GridIndex;
}

void UUINavCollection::GetGridAtIndex(const int GridIndex, FGrid& Grid, bool& bIsValid)
{
	bIsValid = false;
	const int ActualIndex = FirstGridIndex + GridIndex;

	if (ParentWidget == nullptr || ParentWidget->NavigationGrids.Num() <= ActualIndex || ActualIndex < 0) return;

	bIsValid = true;
	Grid = ParentWidget->NavigationGrids[ActualIndex];
}

UUINavCollection* UUINavCollection::GetCollectionByIndex(const int Index)
{
	for (UUINavCollection* Collection : UINavCollections)
	{
		if (Collection->FirstButtonIndex <= Index && Collection->LastButtonIndex >= Index)
		{
			return Collection->GetCollectionByIndex(Index);
		}
	}

	return this;
}

