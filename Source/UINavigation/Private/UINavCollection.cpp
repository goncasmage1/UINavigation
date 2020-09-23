// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved


#include "UINavCollection.h"
#include "UINavWidget.h"
#include "UINavButton.h"
#include "UINavComponent.h"
#include "UINavComponentWrapper.h"
#include "UINavHorizontalComponent.h"
#include "Blueprint/WidgetTree.h"
#include "UINavInputContainer.h"


void UUINavCollection::SetupNavigation_Implementation(const TArray<FButtonNavigation>& EdgeNavigations)
{
}

void UUINavCollection::PreSetup_Implementation()
{
}

void UUINavCollection::NotifyOnNavigate(int From, int To, int LocalFrom, int LocalTo)
{
	bool bFoundFrom = false;
	bool bFoundTo = false;
	for (UUINavCollection* Collection : UINavCollections)
	{
		int CollectionFromIndex = ParentWidget->GetCollectionButtonIndex(Collection, From);
		int CollectionToIndex = ParentWidget->GetCollectionButtonIndex(Collection, To);

		bool bValidFrom = CollectionFromIndex != -1;
		bool bValidTo = CollectionToIndex != -1;
		if (bValidFrom || bValidTo)
		{
			if (!bFoundFrom) bFoundFrom = bValidFrom;
			if (!bFoundTo) bFoundTo = bValidTo;

			Collection->NotifyOnNavigate(From, To, CollectionFromIndex, CollectionToIndex);
		}

		if (bFoundFrom && bFoundTo) break;
	}

	OnNavigate(From, To, LocalFrom, LocalTo);
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

void UUINavCollection::NotifyOnSelect(int Index, int LocalIndex)
{
	for (UUINavCollection* Collection : UINavCollections)
	{
		int CollectionButtonIndex = ParentWidget->GetCollectionButtonIndex(Collection, Index);
		if (CollectionButtonIndex != -1)
		{
			Collection->OnSelect(Index, CollectionButtonIndex);
			break;
		}
	}
	OnSelect(Index, LocalIndex);
}

void UUINavCollection::NotifyOnStartSelect(int Index, int LocalIndex)
{
	for (UUINavCollection* Collection : UINavCollections)
	{
		int CollectionButtonIndex = ParentWidget->GetCollectionButtonIndex(Collection, Index);
		if (CollectionButtonIndex != -1)
		{
			Collection->OnStartSelect(Index, CollectionButtonIndex);
			break;
		}
	}
	OnStartSelect(Index, LocalIndex);
}

void UUINavCollection::NotifyOnStopSelect(int Index, int LocalIndex)
{
	for (UUINavCollection* Collection : UINavCollections)
	{
		int CollectionButtonIndex = ParentWidget->GetCollectionButtonIndex(Collection, Index);
		if (CollectionButtonIndex != -1)
		{
			Collection->OnStopSelect(Index, CollectionButtonIndex);
			break;
		}
	}
	OnStopSelect(Index, LocalIndex);
}

void UUINavCollection::Init(int StartIndex)
{
	FirstButtonIndex = StartIndex;
	LastButtonIndex = StartIndex - 1;
	PreSetup();

	TraverseHierarquy(StartIndex);

	if (UINavAnimations.Num() > 0)
	{
		if (ParentCollection != nullptr)
		{
			int Index = FirstButtonIndex - ParentCollection->FirstButtonIndex;
			if (ParentCollection->UINavAnimations.Num() >= Index)
			{
				ParentCollection->UINavAnimations.Insert(UINavAnimations, Index);
			}
			UINavAnimations.Empty();
		}
		else if (ParentWidget != nullptr)
		{
			int Index = FirstButtonIndex;
			if (ParentWidget->UINavAnimations.Num() >= Index)
			{
				ParentWidget->UINavAnimations.Insert(UINavAnimations, Index);
			}
			UINavAnimations.Empty();
		}
	}
}

void UUINavCollection::TraverseHierarquy(int StartIndex)
{
	//Find UINavButtons in the widget hierarchy
	TArray<UWidget*> Widgets;
	WidgetTree->GetAllWidgets(Widgets);
	for (UWidget* widget : Widgets)
	{
		UScrollBox* Scroll = Cast<UScrollBox>(widget);
		if (Scroll != nullptr)
		{
			ParentWidget->ScrollBoxes.Add(Scroll);
			continue;
		}

		UUINavWidget* UINavWidget = Cast<UUINavWidget>(widget);
		if (UINavWidget != nullptr)
		{
			DISPLAYERROR("The plugin doesn't support nested UINavWidgets. Use UINavCollections for this effect!");
			return;
		}

		UUINavCollection* Collection = Cast<UUINavCollection>(widget);
		if (Collection != nullptr)
		{
			Collection->ParentWidget = ParentWidget;
			Collection->ParentCollection = this;
			Collection->Init(ParentWidget->UINavButtons.Num());
			UINavCollections.Add(Collection);
			continue;
		}

		UUINavInputContainer* InputContainer = Cast<UUINavInputContainer>(widget);
		if (InputContainer != nullptr)
		{
			if (ParentWidget->UINavInputContainer != nullptr)
			{
				DISPLAYERROR("You should only have 1 UINavInputContainer!");
				return;
			}

			ParentWidget->InputContainerIndex = ParentWidget->UINavButtons.Num();
			ParentWidget->UINavInputContainer = InputContainer;

			InputContainer->Init(ParentWidget);
			continue;
		}

		UUINavButton* NewNavButton = Cast<UUINavButton>(widget);
		if (NewNavButton == nullptr)
		{
			UUINavComponent* UIComp = Cast<UUINavComponent>(widget);
			if (UIComp == nullptr)
			{
				UUINavComponentWrapper* UICompWrapper = Cast<UUINavComponentWrapper>(widget);
				if (UICompWrapper != nullptr)
				{
					UIComp = UICompWrapper->GetUINavComponent();
				}
			}

			if (UIComp != nullptr)
			{
				NewNavButton = Cast<UUINavButton>(UIComp->NavButton);

				if (UIComp->ComponentIndex == -1) UIComp->ComponentIndex = ParentWidget->UINavButtons.Num();
				NewNavButton->ButtonIndex = UIComp->ComponentIndex;

				ParentWidget->UINavComponents.Add(UIComp);

				UUINavHorizontalComponent* HorizComp = Cast<UUINavHorizontalComponent>(UIComp);
				if (HorizComp != nullptr)
				{
					HorizComp->ParentWidget = ParentWidget;
					ParentWidget->UINavHorizontalComps.Add(HorizComp);
				}
			}
		}

		if (NewNavButton == nullptr) continue;

		if (NewNavButton->ButtonIndex == -1) NewNavButton->ButtonIndex = ParentWidget->UINavButtons.Num();

		ParentWidget->SetupUINavButtonDelegates(NewNavButton);

		ParentWidget->UINavButtons.Add(NewNavButton);
		ParentWidget->RevertButtonStyle(ParentWidget->UINavButtons.Num() - 1);
	}
}

void UUINavCollection::CallCustomInput(FName ActionName, uint8* Buffer)
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

void UUINavCollection::AppendNavigationGrid1D(EGridType GridType, int Dimension, FButtonNavigation EdgeNavigation, bool bWrap)
{
	ParentWidget->AppendNavigationGrid1D(GridType, Dimension, EdgeNavigation, bWrap);
	IncrementGrids(Dimension);
}

void UUINavCollection::AppendNavigationGrid2D(int DimensionX, int DimensionY, FButtonNavigation EdgeNavigation, bool bWrap, int ButtonsInGrid)
{
	ParentWidget->AppendNavigationGrid2D(DimensionX, DimensionY, EdgeNavigation, bWrap, ButtonsInGrid);
	IncrementGrids((ButtonsInGrid == -1 ? (DimensionX * DimensionY) : ButtonsInGrid));
}

void UUINavCollection::AppendCollection(TArray<FButtonNavigation> EdgeNavigations)
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

void UUINavCollection::IncrementGrids(int Dimension)
{
	GridCount++;
	LastButtonIndex += Dimension;
	if (ParentCollection != nullptr) ParentCollection->IncrementGrids(Dimension);
}

void UUINavCollection::UpdateCollectionLastIndex(int GridIndex, bool bAdded)
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

int UUINavCollection::GetGlobalGridIndex(int GridIndex)
{
	return FirstGridIndex + GridIndex;
}

void UUINavCollection::GetGridAtIndex(int GridIndex, FGrid& Grid, bool& bIsValid)
{
	bIsValid = false;
	int ActualIndex = FirstGridIndex + GridIndex;

	if (ParentWidget == nullptr || ParentWidget->NavigationGrids.Num() <= ActualIndex || ActualIndex < 0) return;

	bIsValid = true;
	Grid = ParentWidget->NavigationGrids[ActualIndex];
}

UUINavCollection* UUINavCollection::GetCollectionByIndex(int Index)
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

