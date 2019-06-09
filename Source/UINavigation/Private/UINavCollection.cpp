// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved


#include "UINavCollection.h"
#include "UINavWidget.h"
#include "UINavButton.h"
#include "UINavComponent.h"
#include "WidgetTree.h"
#include "UINavInputContainer.h"


void UUINavCollection::SetupNavigation_Implementation(const TArray<FButtonNavigation>& EdgeNavigations)
{
}

void UUINavCollection::Init(int StartIndex)
{
	TraverseHierarquy(StartIndex);

	if (ParentCollection != nullptr)
	{
		ParentCollection->UINavButtons.Append(UINavButtons);
		ParentCollection->UINavAnimations.Append(UINavAnimations);
		ParentCollection->UINavComponents.Append(UINavComponents);
		ParentCollection->UINavComponentBoxes.Append(UINavComponentBoxes);
		ParentCollection->UINavInputBoxes.Append(UINavInputBoxes);
	}
	else if (ParentWidget != nullptr)
	{
		ParentWidget->UINavButtons.Append(UINavButtons);
		ParentWidget->UINavAnimations.Append(UINavAnimations);
		ParentWidget->UINavComponents.Append(UINavComponents);
		ParentWidget->UINavComponentBoxes.Append(UINavComponentBoxes);
		ParentWidget->UINavInputBoxes.Append(UINavInputBoxes);
	}
	UINavButtons.Empty();
	UINavAnimations.Empty();
	UINavComponents.Empty();
	UINavComponentBoxes.Empty();
	UINavInputBoxes.Empty();
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
			DISPLAYERROR("The plugin doesn't support nested UINavWidgets");
		}

		UUINavCollection* Collection = Cast<UUINavCollection>(widget);
		if (Collection != nullptr)
		{
			Collection->ParentWidget = ParentWidget;
			Collection->ParentCollection = this;
			Collection->Init(UINavButtons.Num() + StartIndex);
			UINavCollections.Add(Collection);
		}

		UUINavInputContainer* InputContainer = Cast<UUINavInputContainer>(widget);
		if (InputContainer != nullptr)
		{
			if (ParentWidget->UINavInputContainer != nullptr)
			{
				DISPLAYERROR("Found more than 1 UINavInputContainer!");
				return;
			}

			ParentWidget->InputContainerIndex = UINavButtons.Num() + StartIndex;
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

				if (UIComp->ComponentIndex == -1) UIComp->ComponentIndex = UINavButtons.Num() + StartIndex;
				NewNavButton->ButtonIndex = UIComp->ComponentIndex;

				UINavComponents.Add(UIComp);

				UUINavComponentBox* UICompBox = Cast<UUINavComponentBox>(widget);
				if (UICompBox != nullptr)
				{
					UINavComponentBoxes.Add(UICompBox);
				}
			}
		}

		if (NewNavButton == nullptr) continue;

		if (NewNavButton->ButtonIndex == -1) NewNavButton->ButtonIndex = UINavButtons.Num() + StartIndex;

		ParentWidget->SetupUINavButtonDelegates(NewNavButton);

		UINavButtons.Add(NewNavButton);
	}

	UINavButtons.HeapSort([](const UUINavButton& Wid1, const UUINavButton& Wid2)
	{
		return Wid1.ButtonIndex < Wid2.ButtonIndex;
	});
}

void UUINavCollection::AppendNavigationGrid1D(EGridType GridType, int Dimension, FButtonNavigation EdgeNavigation, bool bWrap)
{
	ParentWidget->AppendNavigationGrid1D(GridType, Dimension, EdgeNavigation, bWrap);
}

void UUINavCollection::AppendNavigationGrid2D(int DimensionX, int DimensionY, FButtonNavigation EdgeNavigation, bool bWrap, int ButtonsInGrid)
{
	ParentWidget->AppendNavigationGrid2D(DimensionX, DimensionY, EdgeNavigation, bWrap, ButtonsInGrid);
}

void UUINavCollection::AppendCollection(TArray<FButtonNavigation> EdgeNavigations)
{
	if (CollectionIndex >= UINavCollections.Num())
	{
		DISPLAYERROR("Can't append UINavCollection to navigation, no remaining UINavCollection found!");
		return;
	}

	UINavCollections[CollectionIndex]->SetupNavigation(EdgeNavigations);

	CollectionIndex++;
}

