// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved


#include "UINavCollection.h"
#include "UINavWidget.h"
#include "UINavButton.h"
#include "UINavComponent.h"
#include "UINavHorizontalComponent.h"
#include "Blueprint/WidgetTree.h"
#include "UINavInputContainer.h"


void UUINavCollection::SetupNavigation_Implementation(const TArray<FButtonNavigation>& EdgeNavigations)
{
}

void UUINavCollection::PreSetup_Implementation()
{
}

void UUINavCollection::Init(int StartIndex)
{
	PreSetup();

	TraverseHierarquy(StartIndex);

	if (ParentWidget != nullptr)
	{
		ParentWidget->UINavButtons.Append(UINavButtons);
		UINavButtons.Empty();
		ParentWidget->UINavComponents.Append(UINavComponents);
		UINavComponents.Empty();
		ParentWidget->UINavHorizontalComps.Append(UINavHorizontalComps);
		UINavHorizontalComps.Empty();
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
		}

		UUINavWidget* UINavWidget = Cast<UUINavWidget>(widget);
		if (UINavWidget != nullptr)
		{
			DISPLAYERROR("The plugin doesn't support nested UINavWidgets. Use UINavCollections for this effect!");
		}

		UUINavCollection* Collection = Cast<UUINavCollection>(widget);
		if (Collection != nullptr)
		{
			Collection->ParentWidget = ParentWidget;
			Collection->ParentCollection = this;
			Collection->Init(ParentWidget->UINavButtons.Num() + UINavButtons.Num());
			UINavCollections.Add(Collection);
		}

		UUINavInputContainer* InputContainer = Cast<UUINavInputContainer>(widget);
		if (InputContainer != nullptr)
		{
			if (ParentWidget->UINavInputContainer != nullptr)
			{
				DISPLAYERROR("You should only have 1 UINavInputContainer!");
				return;
			}

			ParentWidget->InputContainerIndex = ParentWidget->UINavButtons.Num() + UINavButtons.Num();
			ParentWidget->UINavInputContainer = InputContainer;

			InputContainer->Init(ParentWidget);
		}

		UUINavButton* NewNavButton = Cast<UUINavButton>(widget);

		if (NewNavButton == nullptr)
		{
			UUINavComponent* UIComp = Cast<UUINavComponent>(widget);
			if (UIComp != nullptr)
			{
				NewNavButton = Cast<UUINavButton>(UIComp->NavButton);

				if (UIComp->ComponentIndex == -1) UIComp->ComponentIndex = ParentWidget->UINavButtons.Num() + UINavButtons.Num();
				NewNavButton->ButtonIndex = UIComp->ComponentIndex;

				UINavComponents.Add(UIComp);

				UUINavHorizontalComponent* HorizComp = Cast<UUINavHorizontalComponent>(widget);
				if (HorizComp != nullptr)
				{
					UINavHorizontalComps.Add(HorizComp);
				}
			}
		}

		if (NewNavButton == nullptr) continue;

		if (NewNavButton->ButtonIndex == -1) NewNavButton->ButtonIndex = ParentWidget->UINavButtons.Num() + UINavButtons.Num();

		ParentWidget->SetupUINavButtonDelegates(NewNavButton);

		UINavButtons.Add(NewNavButton);
	}
}

void UUINavCollection::AppendNavigationGrid1D(EGridType GridType, int Dimension, FButtonNavigation EdgeNavigation, bool bWrap)
{
	ParentWidget->AppendNavigationGrid1D(GridType, Dimension, EdgeNavigation, bWrap);
	IncrementGrids();
}

void UUINavCollection::AppendNavigationGrid2D(int DimensionX, int DimensionY, FButtonNavigation EdgeNavigation, bool bWrap, int ButtonsInGrid)
{
	ParentWidget->AppendNavigationGrid2D(DimensionX, DimensionY, EdgeNavigation, bWrap, ButtonsInGrid);
	IncrementGrids();
}

void UUINavCollection::AppendCollection(TArray<FButtonNavigation> EdgeNavigations)
{
	if (CollectionIndex >= UINavCollections.Num())
	{
		DISPLAYERROR("Can't append UINavCollection to navigation, no remaining UINavCollection found!");
		return;
	}

	UINavCollections[CollectionIndex]->FirstGridIndex = FirstGridIndex + GridCount;
	UINavCollections[CollectionIndex]->SetupNavigation(EdgeNavigations);

	CollectionIndex++;
}

void UUINavCollection::IncrementGrids()
{
	GridCount++;
	if (ParentCollection != nullptr) ParentCollection->IncrementGrids();
}

void UUINavCollection::GetGridAtIndex(int GridIndex, FGrid& Grid, bool& bIsValid)
{
	bIsValid = false;
	int ActualIndex = FirstGridIndex + GridIndex;

	if (ParentWidget == nullptr || ParentWidget->NavigationGrids.Num() <= ActualIndex || ActualIndex < 0) return;

	bIsValid = true;
	Grid = ParentWidget->NavigationGrids[ActualIndex];
}

