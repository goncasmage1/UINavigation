// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#include "UINavWidget.h"
#include "UINavCollection.h"
#include "UINavButton.h"
#include "UINavHorizontalComponent.h"
#include "UINavComponent.h"
#include "UINavInputBox.h"
#include "UINavInputContainer.h"
#include "UINavPCComponent.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ActorComponent.h"

void UUINavWidget::NativeConstruct()
{
	Super::NativeConstruct();

	/*
	If this widget was added through a parent widget and should remove it from the viewport,
	remove that widget from viewport
	*/
	if (ParentWidget != nullptr && ParentWidget->IsInViewport() && bParentRemoved)
	{
		ParentWidget->RemoveFromParent();
	}

	//If this widget was added through a child widget, destroy it
	if (ReturnedFromWidget != nullptr)
	{
		ReturnedFromWidget->Destruct();
		ReturnedFromWidget = nullptr;
	}

	PreSetup();
}

void UUINavWidget::InitialSetup()
{
	//If widget was already setup, apply only certain steps
	if (bCompletedSetup)
	{
		ReconfigureSetup();
		return;
	}

	bSetupStarted = true;
	bIsFocusable = true;
	WidgetClass = GetClass();
	if (UINavPC == nullptr)
	{
		APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
		if (PC == nullptr)
		{
			DISPLAYERROR("Player Controller is Null!");
			return;
		}
		UINavPC = Cast<UUINavPCComponent>(PC->GetComponentByClass(UUINavPCComponent::StaticClass()));
		if (UINavPC == nullptr)
		{
			DISPLAYERROR("Player Controller doesn't have a UINavPCComponent!");
			return;
		}
	}

	FetchButtonsInHierarchy();
	ReadyForSetup();

	if (NumberOfButtonsInGrids != UINavButtons.Num())
	{
		DISPLAYERROR("Not all UINavButtons have a grid setup. Double check the Append Navigation functions.");
		return;
	}

	if (UINavAnimations.Num() > 0 && UINavAnimations.Num() != UINavButtons.Num())
	{
		DISPLAYERROR("Number of animations doesn't match number of UINavButtons.");
		return;
	}

	if (bUseTextColor) ChangeTextColorToDefault();

	//If this widget doesn't need to create the selector, skip to setup
	if (!IsSelectorValid())
	{
		UINavSetup();
		bShouldTick = false;
		return;
	}
	else
	{
		SetupSelector();
	}
}

void UUINavWidget::ReconfigureSetup()
{
	bSetupStarted = true;

	if (bUseTextColor) ChangeTextColorToDefault();

	//If this widget doesn't need to create the selector, skip to setup
	if (!IsSelectorValid())
	{
		UINavSetup();
		return;
	}
	else
	{
		SetupSelector();
	}

	bShouldTick = true;
	WaitForTick = 0;
}

void UUINavWidget::CleanSetup()
{
	//Disable all buttons (bug fix)
	for (UUINavButton* button : UINavButtons)
	{
		button->SetIsEnabled(false);
	}
	bSetupStarted = false;
}

void UUINavWidget::FetchButtonsInHierarchy()
{
	TraverseHierarquy();

	int ButtonsNum = UINavButtons.Num();
	if (FirstButtonIndex >= ButtonsNum && ButtonsNum > 0)
	{
		DISPLAYERROR("Invalid FirstButton index, can't be greater than number of buttons.");
		return;
	}

	if (FirstButtonIndex < 0) FirstButtonIndex = 0;

	ButtonIndex = FirstButtonIndex;
	if (ButtonsNum > 0) CurrentButton = UINavButtons[FirstButtonIndex];
	else return;

	bool bValid = CurrentButton->IsValid();

	if (bValid)
	{
		UUINavComponent* UINavComp = GetUINavComponentAtIndex(ButtonIndex);
		if (UINavComp != nullptr && !UINavComp->IsValid()) bValid = false;
	}

	while (!bValid)
	{
		ButtonIndex++;
		if (ButtonIndex >= UINavButtons.Num()) ButtonIndex = 0;
		
		CurrentButton = UINavButtons[ButtonIndex];
		if (ButtonIndex == FirstButtonIndex) break;

		UUINavComponent* UINavComp = GetUINavComponentAtIndex(ButtonIndex);
		if (UINavComp != nullptr && !UINavComp->IsValid()) continue;

		bValid = CurrentButton->IsValid();
	}
}

void UUINavWidget::TraverseHierarquy()
{
	//Find UINavButtons in the widget hierarchy
	TArray<UWidget*> Widgets;
	WidgetTree->GetAllWidgets(Widgets);
	for (UWidget* widget : Widgets)
	{
		UScrollBox* Scroll = Cast<UScrollBox>(widget);
		if (Scroll != nullptr)
		{
			ScrollBoxes.Add(Scroll);
		}

		UUINavWidget* UINavWidget = Cast<UUINavWidget>(widget);
		if (UINavWidget != nullptr)
		{
			DISPLAYERROR("The plugin doesn't support nested UINavWidgets. Use UINavCollections for this effect!");
		}

		UUINavCollection* Collection = Cast<UUINavCollection>(widget);
		if (Collection != nullptr)
		{
			Collection->ParentWidget = this;
			Collection->Init(UINavButtons.Num());
			UINavCollections.Add(Collection);
		}

		UUINavInputContainer* InputContainer = Cast<UUINavInputContainer>(widget);
		if (InputContainer != nullptr)
		{
			if (UINavInputContainer != nullptr)
			{
				DISPLAYERROR("You should only have 1 UINavInputContainer");
				return;
			}

			InputContainerIndex = UINavButtons.Num();
			UINavInputContainer = InputContainer;

			InputContainer->Init(this);
		}

		UUINavButton* NewNavButton = Cast<UUINavButton>(widget);

		if (NewNavButton == nullptr)
		{
			UUINavComponent* UIComp = Cast<UUINavComponent>(widget);
			if (UIComp != nullptr)
			{
				NewNavButton = Cast<UUINavButton>(UIComp->NavButton);

				if (UIComp->ComponentIndex == -1) UIComp->ComponentIndex = UINavButtons.Num();
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

		if (NewNavButton->ButtonIndex == -1) NewNavButton->ButtonIndex = UINavButtons.Num();

		SetupUINavButtonDelegates(NewNavButton);

		UINavButtons.Add(NewNavButton);
	}

	UINavButtons.HeapSort([](const UUINavButton& Wid1, const UUINavButton& Wid2)
	{
		return Wid1.ButtonIndex < Wid2.ButtonIndex;
	});
}

void UUINavWidget::ChangeTextColorToDefault()
{
	for (int j = 0; j < UINavButtons.Num(); j++) SwitchTextColorTo(j, TextDefaultColor);
}

void UUINavWidget::SetupSelector()
{
	TheSelector->SetVisibility(ESlateVisibility::Hidden);

	UCanvasPanelSlot* SelectorSlot = Cast<UCanvasPanelSlot>(TheSelector->Slot);

	SelectorSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	SelectorSlot->SetPosition(FVector2D(0.f, 0.f));
}

void UUINavWidget::UINavSetup()
{
	//Re-enable all buttons (bug fix)
	for (UUINavButton* button : UINavButtons)
	{
		button->SetIsEnabled(true);
	}

	if (UINavPC != nullptr) UINavPC->ActiveWidget = this;

	if (IsSelectorValid()) TheSelector->SetVisibility(ESlateVisibility::HitTestInvisible);

	bCompletedSetup = true;

	if (UINavButtons.Num() > 0)
	{
		DispatchNavigation(ButtonIndex);
		OnNavigate(-1, ButtonIndex);
	}

	OnSetupCompleted();
}

void UUINavWidget::ReadyForSetup_Implementation()
{

}

void UUINavWidget::NativeTick(const FGeometry & MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	if (!IsSelectorValid() || !bSetupStarted) return;

	if (bMovingSelector)
	{
		HandleSelectorMovement(DeltaTime);
	}
	else
	{
		if (!bShouldTick) return;

		if (WaitForTick == 1)
		{
			UINavSetup();
			bShouldTick = false;
			return;
		}

		WaitForTick++;
	}
}

FReply UUINavWidget::NativeOnKeyDown(const FGeometry & InGeometry, const FKeyEvent & InKeyEvent)
{
	Super::NativeOnKeyDown(InGeometry, InKeyEvent);

	if (ReceiveInputType != EReceiveInputType::None)
	{
		FKey PressedKey = InKeyEvent.GetKey();

		if ((UINavInputContainer->bCanCancelKeybind && UINavPC->IsReturnKey(PressedKey)))
		{
			CancelRebind();
			return FReply::Handled();
		}

		if (ReceiveInputType == EReceiveInputType::Axis)
		{
			FKey AxisKey = UINavInputContainer->GetAxisKeyFromActionKey(PressedKey);
			if (AxisKey.GetFName().IsEqual(FName("None")))
			{
				CancelRebind();
				return FReply::Handled();
			}
			PressedKey = AxisKey;
		}

		ProcessNonMouseKeybind(PressedKey);
		return FReply::Handled();
	}
	else
	{
		FReply reply = OnKeyPressed(InKeyEvent.GetKey());
		if (reply.IsEventHandled()) return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply UUINavWidget::NativeOnKeyUp(const FGeometry & InGeometry, const FKeyEvent & InKeyEvent)
{
	Super::NativeOnKeyUp(InGeometry, InKeyEvent);

	if (!bWaitForInput)
	{
		FReply reply = OnKeyReleased(InKeyEvent.GetKey());
		if (reply.IsEventHandled()) return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply UUINavWidget::NativeOnMouseWheel(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent)
{
	if (bWaitForInput)
	{
		ProcessMouseKeybind(InMouseEvent.GetWheelDelta() > 0.f ? FKey(EKeys::MouseScrollUp) : FKey(EKeys::MouseScrollDown));
	}
	else
	{
		if (UINavPC->GetCurrentInputType() != EInputType::Mouse)
		{
			UINavPC->NotifyMouseInputType();
		}
	}
	return FReply::Handled();
}

FReply UUINavWidget::NativeOnMouseButtonDown(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (bWaitForInput)
	{
		if (ReceiveInputType == EReceiveInputType::Axis)
		{
			CancelRebind();
			return FReply::Handled();
		}
		ProcessMouseKeybind(InMouseEvent.GetEffectingButton());
		return FReply::Handled();
	}
	else
	{
		FReply reply = OnKeyPressed(InMouseEvent.GetEffectingButton());
		if (reply.IsEventHandled()) return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply UUINavWidget::NativeOnMouseButtonUp(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent)
{
	Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	if (!bWaitForInput && InMouseEvent.GetEffectingButton().IsMouseButton())
	{
		FReply reply = OnKeyReleased(InMouseEvent.GetEffectingButton());
		if (reply.IsEventHandled()) return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply UUINavWidget::OnKeyPressed(FKey PressedKey)
{
	FString ActionName = UINavPC->FindActionByKey(PressedKey);
	if (ActionName.Equals(TEXT("")))
	{
		UINavPC->VerifyInputTypeChangeByKey(PressedKey);
		return FReply::Handled();
	}

	return UINavPC->OnActionPressed(ActionName, PressedKey);
}

FReply UUINavWidget::OnKeyReleased(FKey PressedKey)
{
	FString ActionName = UINavPC->FindActionByKey(PressedKey);
	if (ActionName.Equals(TEXT(""))) return FReply::Handled();

	return UINavPC->OnActionReleased(ActionName, PressedKey);
}

void UUINavWidget::HandleSelectorMovement(float DeltaTime)
{
	if (MoveCurve == nullptr) return;

	MovementCounter += DeltaTime;

	//Movement is finished
	if (MovementCounter >= MovementTime)
	{
		MovementCounter = 0.f;
		bMovingSelector = false;
		if (UINavPC != nullptr) UINavPC->bAllowNavigation = true;
		TheSelector->SetRenderTranslation(SelectorDestination);
		if (HaltedIndex != -1)
		{
			if (HaltedIndex == SELECT_INDEX) OnSelect(ButtonIndex);
			else if (HaltedIndex == RETURN_INDEX) OnReturn();
			else NavigateTo(HaltedIndex);

			HaltedIndex = -1;
		}
		return;
	}

	TheSelector->SetRenderTranslation(SelectorOrigin + Distance*MoveCurve->GetFloatValue(MovementCounter));
}

void UUINavWidget::AddUINavButton(UUINavButton * NewButton, int TargetGridIndex, int IndexInGrid)
{
	if (NewButton == nullptr || TargetGridIndex >= NavigationGrids.Num() || TargetGridIndex < 0) return;

	if (UINavAnimations.Num() > 0)
	{
		DISPLAYERROR("Runtime manipulation not supported with navigation using animations.");
	}

	NumberOfButtonsInGrids++;
	FGrid& TargetGrid = NavigationGrids[TargetGridIndex];

	if (IndexInGrid >= TargetGrid.GetDimension() || IndexInGrid <= -1) IndexInGrid = TargetGrid.GetDimension();

	IncrementGrid(NewButton, TargetGrid, IndexInGrid);

	NewButton->ButtonIndex = TargetGrid.FirstButton->ButtonIndex + IndexInGrid;
	NewButton->GridIndex = TargetGrid.GridIndex;
	NewButton->IndexInGrid = IndexInGrid;
	SetupUINavButtonDelegates(NewButton);
	UINavButtons.Insert(NewButton, NewButton->ButtonIndex);

	IncrementUINavButtonIndices(NewButton->ButtonIndex);
	IncrementUINavComponentIndices(NewButton->ButtonIndex);
}

void UUINavWidget::AddUINavComponent(UUINavComponent * NewComponent, int TargetGridIndex, int IndexInGrid)
{
	if (NewComponent == nullptr || TargetGridIndex >= NavigationGrids.Num() || TargetGridIndex < 0) return;

	if (UINavAnimations.Num() > 0)
	{
		DISPLAYERROR("Runtime manipulation not supported with navigation using animations.");
	}

	NumberOfButtonsInGrids++;
	FGrid& TargetGrid = NavigationGrids[TargetGridIndex];

	if (IndexInGrid >= TargetGrid.GetDimension() || IndexInGrid <= -1) IndexInGrid = TargetGrid.GetDimension();

	IncrementGrid(NewComponent->NavButton, TargetGrid, IndexInGrid);

	NewComponent->NavButton->ButtonIndex = TargetGrid.FirstButton->ButtonIndex + IndexInGrid;
	NewComponent->ComponentIndex = NewComponent->NavButton->ButtonIndex;
	NewComponent->NavButton->GridIndex = TargetGrid.GridIndex;
	NewComponent->NavButton->IndexInGrid = IndexInGrid;
	SetupUINavButtonDelegates(NewComponent->NavButton);
	
	int TargetIndex = NewComponent->ComponentIndex;
	UINavButtons.Insert(NewComponent->NavButton, NewComponent->NavButton->ButtonIndex);
	InsertNewComponent(NewComponent, TargetIndex);

	IncrementUINavButtonIndices(NewComponent->ComponentIndex);
}

void UUINavWidget::DeleteUINavElement(int Index, bool bAutoNavigate)
{
	if (Index < 0 || Index >= UINavButtons.Num()) return;

	if (Index == ButtonIndex && bAutoNavigate)
	{
		bool bValid = false;

		UUINavButton* Temp = CurrentButton;
		while (!bValid)
		{
			int NewIndex = Temp->ButtonIndex + 1;
			if (NewIndex >= UINavButtons.Num()) NewIndex = 0;

			Temp = UINavButtons[NewIndex];
			if (NewIndex == FirstButtonIndex) break;

			UUINavComponent* UINavComp = GetUINavComponentAtIndex(NewIndex);
			if (UINavComp != nullptr && !UINavComp->IsValid()) continue;

			bValid = Temp->IsValid();
		}

		if (Temp == nullptr) return;
		NavigateTo(Temp->ButtonIndex);
		ButtonIndex--;
	}

	UUINavButton* Button = UINavButtons[Index];
	DecrementUINavButtonIndices(Index);
	DecrementUINavComponentIndices(Index);

	DecrementGrid(NavigationGrids[Button->GridIndex], Button->IndexInGrid);
}

void UUINavWidget::DeleteUINavElementFromGrid(int GridIndex, int IndexInGrid, bool bAutoNavigate)
{
	if (GridIndex < 0 || GridIndex > NavigationGrids.Num())
	{
		DISPLAYERROR("Invalid GridIndex");
		return;
	}
	FGrid TargetGrid = NavigationGrids[GridIndex];
	IndexInGrid = IndexInGrid >= 0 && IndexInGrid < TargetGrid.GetDimension() ? IndexInGrid : TargetGrid.GetDimension() - 1;

	DeleteUINavElement(TargetGrid.FirstButton->ButtonIndex + IndexInGrid, bAutoNavigate);
}

void UUINavWidget::IncrementGrid(UUINavButton* NewButton, FGrid & TargetGrid, int& IndexInGrid)
{
	if (IndexInGrid == 0)
	{
		int FirstIndex = 0; TargetGrid.FirstButton != nullptr ? TargetGrid.FirstButton->ButtonIndex : GetGridStartingIndex(TargetGrid.GridIndex);
		TargetGrid.FirstButton = NewButton;
		NewButton->ButtonIndex = FirstIndex;
	}
	NewButton->GridIndex = TargetGrid.GridIndex;

	if (TargetGrid.GridType == EGridType::Horizontal) TargetGrid.DimensionX++;
	else if (TargetGrid.GridType == EGridType::Vertical) TargetGrid.DimensionY++;
	else {
		TargetGrid.NumGrid2DButtons++;
		if (TargetGrid.GetDimension() == 0)
		{
			TargetGrid.DimensionX = 1;
			TargetGrid.DimensionY = 1;
		}
		else if (TargetGrid.NumGrid2DButtons > (TargetGrid.DimensionX * TargetGrid.DimensionY))
		{
			TargetGrid.DimensionY++;
		}
	}

	for (int i = IndexInGrid + 1; i < TargetGrid.GetDimension(); i++)
	{
		if (TargetGrid.FirstButton->ButtonIndex + i >= UINavButtons.Num()) break;

		UINavButtons[TargetGrid.FirstButton->ButtonIndex + i]->IndexInGrid++;
	}
}

void UUINavWidget::DecrementGrid(FGrid & TargetGrid, int IndexInGrid)
{
	if (IndexInGrid == 0)
	{
		TargetGrid.FirstButton = TargetGrid.GetDimension() > 1 ? UINavButtons[TargetGrid.FirstButton->ButtonIndex] : nullptr;
	}

	if (TargetGrid.GridType == EGridType::Horizontal) TargetGrid.DimensionX--;
	else if (TargetGrid.GridType == EGridType::Vertical) TargetGrid.DimensionY--;
	else TargetGrid.NumGrid2DButtons--;

	for (int i = 0; i < TargetGrid.GetDimension(); i++)
	{
		UINavButtons[TargetGrid.FirstButton->ButtonIndex + i]->IndexInGrid = i;
	}
}

void UUINavWidget::InsertNewComponent(UUINavComponent* NewComponent, int TargetIndex)
{
	int FoundIndex = GetLocalComponentIndex(TargetIndex);
	if (FoundIndex != -1)
	{
		UINavComponents.Insert(NewComponent, FoundIndex);
	}
	else
	{
		if (UINavComponents.Num() > 0)
		{
			if (UINavComponents[0]->ComponentIndex > TargetIndex)
			{
				UINavComponents.Insert(NewComponent, 0);
				IncrementUINavComponentIndices(NewComponent->ComponentIndex);
			}
			else
			{
				for (int i = 0; i < UINavComponents.Num(); i++)
				{
					if (UINavComponents[i]->ComponentIndex > TargetIndex)
					{
						UINavComponents.Insert(NewComponent, i);
						IncrementUINavComponentIndices(NewComponent->ComponentIndex);
						break;
					}
				}
			}
		}
		else
		{
			UINavComponents.Add(NewComponent);
		}
	}
}

void UUINavWidget::IncrementUINavButtonIndices(int StartingIndex)
{
	for (int i = StartingIndex + 1; i < UINavButtons.Num(); i++) UINavButtons[i]->ButtonIndex = i;
	if (StartingIndex <= ButtonIndex) ButtonIndex++;
}

void UUINavWidget::IncrementUINavComponentIndices(int StartingIndex)
{
	int ValidIndex = GetLocalComponentIndex(StartingIndex);
	for (int i = (ValidIndex != -1 ? ValidIndex + 1 : 0); i < UINavComponents.Num(); i++)
	{
		if (UINavComponents[i]->ComponentIndex > StartingIndex) UINavComponents[i]->ComponentIndex++;
	}
}

void UUINavWidget::DecrementUINavButtonIndices(int StartingIndex)
{
	for (int i = StartingIndex; i < UINavButtons.Num()-1; i++)
	{
		UINavButtons[i] = UINavButtons[i + 1];
		UINavButtons[i]->ButtonIndex = i;
	}
	UINavButtons.RemoveAt(UINavButtons.Num()-1, 1, true);
}

void UUINavWidget::DecrementUINavComponentIndices(int StartingIndex)
{
	int ValidIndex = GetLocalComponentIndex(StartingIndex);
	UUINavComponent* Component = (ValidIndex != -1 ? UINavComponents[ValidIndex] : nullptr);

	for (int i = (ValidIndex != -1 ? ValidIndex + 1 : 0); i < UINavComponents.Num() - (ValidIndex != -1 ? 1 : 0); i++)
	{
		if (ValidIndex != -1) UINavComponents[i] = UINavComponents[i + 1];
		if (UINavComponents[i]->ComponentIndex > StartingIndex) UINavComponents[i]->ComponentIndex--;
	}

	if (Component != nullptr)
	{
		UINavComponents.RemoveAt(UINavComponents.Num()-1, 1, true);
	}
}

void UUINavWidget::MoveUINavElementToGrid(int Index, int TargetGridIndex, int IndexInGrid)
{
	if (Index < 0 || TargetGridIndex < 0)
	{
		DISPLAYERROR("All received indices must be greater than 0");
		return;
	}
	UUINavButton* Button = UINavButtons.Num() > Index ? UINavButtons[Index] : nullptr;
	if (Button == nullptr || TargetGridIndex >= NavigationGrids.Num()) return;

	FGrid& TargetGrid = NavigationGrids[TargetGridIndex];
	bool isLast = IndexInGrid <= -1 || IndexInGrid >= TargetGrid.GetDimension();

	if (UINavAnimations.Num() > 0) DISPLAYERROR("Runtime manipulation not supported with navigation using animations.");

	int OldGridIndex = Button->GridIndex;
	int OldIndexInGrid = Button->IndexInGrid;

	if (IndexInGrid >= TargetGrid.GetDimension() || IndexInGrid <= -1) IndexInGrid = TargetGrid.GetDimension() - 1;
	if (TargetGrid.GridIndex != Button->GridIndex)
	{
		DecrementGrid(NavigationGrids[Button->GridIndex], Button->IndexInGrid);
		IncrementGrid(Button, TargetGrid, IndexInGrid);
		if (isLast) IndexInGrid++;
	}
	
	int From = Index;
	int To = TargetGrid.FirstButton->ButtonIndex + IndexInGrid;

	if (To >= TargetGrid.GetDimension()) To = TargetGrid.GetDimension() - 1;

	if (From == To) return;

	if (Button->IndexInGrid == 0) TargetGrid.FirstButton = UINavButtons[Button->ButtonIndex+1];
	if (IndexInGrid == 0) TargetGrid.FirstButton = Button;

	Button->IndexInGrid = IndexInGrid;
	
	UpdateArrays(From, To);

	ReplaceButtonInNavigationGrid(Button, OldGridIndex, OldIndexInGrid);

	if (Button == CurrentButton) UpdateCurrentButton(Button);
	ButtonIndex = CurrentButton->ButtonIndex;
}

void UUINavWidget::MoveUINavElementToIndex(int Index, int TargetIndex)
{
	if (Index >= UINavButtons.Num() || TargetIndex >= UINavButtons.Num() || Index < 0 || TargetIndex < 0) return;

	UUINavButton* ToButton = UINavButtons[TargetIndex];
	MoveUINavElementToGrid(Index, ToButton->GridIndex, ToButton->IndexInGrid);
}

void UUINavWidget::UpdateArrays(int From, int To)
{
	UpdateButtonArray(From, To);
	UpdateComponentArray(From, To);
}

void UUINavWidget::UpdateButtonArray(int From, int To)
{
	UUINavButton* TempButton = UINavButtons[From];

	if (From < To)
	{
		for (int i = From + 1; i <= To; i++)
		{
			UINavButtons[i]->ButtonIndex--;
			UINavButtons[i]->IndexInGrid--;
			UINavButtons[i-1] = UINavButtons[i];
			if (i == To)
			{
				UINavButtons[i] = TempButton;
				UINavButtons[i]->ButtonIndex = i;
			}
		}

	}
	else
	{
		for (int i = From - 1; i >= To; i--)
		{
			UINavButtons[i]->ButtonIndex++;
			UINavButtons[i]->IndexInGrid++;
			UINavButtons[i+1] = UINavButtons[i];
			if (i == To)
			{
				UINavButtons[i] = TempButton;
				UINavButtons[i]->ButtonIndex = i;
			}
		}
	}
}

void UUINavWidget::UpdateComponentArray(int From, int To)
{
	if (UINavComponents.Num() == 0) return;

	UUINavComponent* TempComp = GetUINavComponentAtIndex(From);

	int i;
	int Start = 0;
	int End = UINavComponents.Num()-1;

	bool isLess = From < To;

	if (Start == End)
	{
		UINavComponents[Start]->ComponentIndex = UINavComponents[Start]->NavButton->ButtonIndex;
		return;
	}

	if (isLess)
	{
		if (TempComp != nullptr) Start = UINavComponents.Find(TempComp);

		if (Start == End)
		{
			UINavComponents[Start]->ComponentIndex = UINavComponents[Start]->NavButton->ButtonIndex;
			return;
		}

		if (UINavComponents[0]->ComponentIndex < From)
		{
			for (i = 0; i < UINavComponents.Num(); i++)
			{
				if (Start == 0 && UINavComponents[i]->ComponentIndex > From)
				{
					Start = i;
				}
				if (End == UINavComponents.Num() && UINavComponents[i]->ComponentIndex > To)
				{
					End = i;
					break;
				}
			}
		}

		if (Start == End)
		{
			UINavComponents[Start]->ComponentIndex = UINavComponents[Start]->NavButton->ButtonIndex;
			return;
		}

		for (i = Start+1; i <= End; i++)
		{
			UINavComponents[i]->ComponentIndex = UINavComponents[i]->NavButton->ButtonIndex;
			UINavComponents[i-1] = UINavComponents[i];
			if (i == End)
			{
				UINavComponents[i] = TempComp;
				UINavComponents[i]->ComponentIndex =  UINavComponents[i]->NavButton->ButtonIndex;
			}
		}
	}
	else
	{
		if (TempComp != nullptr) End = UINavComponents.Find(TempComp);

		if (Start == End)
		{
			UINavComponents[Start]->ComponentIndex = UINavComponents[Start]->NavButton->ButtonIndex;
			return;
		}

		if (UINavComponents[0]->ComponentIndex < To)
		{
			for (i = 0; i < UINavComponents.Num(); i++)
			{
				if (Start == 0 && UINavComponents[i]->ComponentIndex > To)
				{
					Start = i;
				}
				if (End == UINavComponents.Num() && UINavComponents[i]->ComponentIndex > From)
				{
					End = i;
					break;
				}
			}
		}

		if (Start == End)
		{
			UINavComponents[Start]->ComponentIndex = UINavComponents[Start]->NavButton->ButtonIndex;
			return;
		}

		for (i = End-1; i >= Start; i--)
		{
			UINavComponents[i]->ComponentIndex = UINavComponents[i]->NavButton->ButtonIndex;
			UINavComponents[i+1] = UINavComponents[i];
			if (i == Start)
			{
				UINavComponents[i] = TempComp;
				UINavComponents[i]->ComponentIndex =  UINavComponents[i]->NavButton->ButtonIndex;
			}
		}
	}
}

void UUINavWidget::ReplaceButtonInNavigationGrid(UUINavButton * ButtonToReplace, int GridIndex, int IndexInGrid)
{
	UUINavButton* NewButton = NavigationGrids[GridIndex].GetDimension() > IndexInGrid ? GetButtonAtGridIndex(NavigationGrids[GridIndex], IndexInGrid) : nullptr;
	for (int i = 0; i < NavigationGrids.Num(); i++)
	{
		if (NavigationGrids[i].EdgeNavigation.DownButton == ButtonToReplace) NavigationGrids[i].EdgeNavigation.DownButton = NewButton;
		if (NavigationGrids[i].EdgeNavigation.UpButton == ButtonToReplace) NavigationGrids[i].EdgeNavigation.UpButton = NewButton;
		if (NavigationGrids[i].EdgeNavigation.LeftButton == ButtonToReplace) NavigationGrids[i].EdgeNavigation.LeftButton = NewButton;
		if (NavigationGrids[i].EdgeNavigation.RightButton == ButtonToReplace) NavigationGrids[i].EdgeNavigation.RightButton = NewButton;
	}
}

void UUINavWidget::UpdateCurrentButton(UUINavButton * NewCurrentButton)
{
	ButtonIndex = NewCurrentButton->ButtonIndex;
	if (IsSelectorValid())
	{
		if (MoveCurve != nullptr) BeginSelectorMovement(NewCurrentButton->ButtonIndex);
		else UpdateSelectorLocation(NewCurrentButton->ButtonIndex);
	}

	for (int i = 0; i < ScrollBoxes.Num(); ++i)
	{
		ScrollBoxes[i]->ScrollWidgetIntoView(NewCurrentButton);
	}
}

void UUINavWidget::AppendNavigationGrid1D(EGridType GridType, int Dimension, FButtonNavigation EdgeNavigation, bool bWrap)
{
	if (UINavButtons.Num() == 0)
	{
		DISPLAYERROR("Make sure to call the Append Navigation functions only during the ReadyForSetup event!");
		return;
	}
	else if (NumberOfButtonsInGrids >= UINavButtons.Num())
	{
		DISPLAYERROR("Not enough UINavButtons to append this navigation grid!");
		return;
	}

	if (Dimension < 0) Dimension = UINavButtons.Num();
	if (GridType == EGridType::Grid2D)
	{
		DISPLAYERROR("Append Navigation Grid 1D Type shouldn't be 2D");
		return;
	}

	bool bFoundContainer = InputContainerIndex >= NumberOfButtonsInGrids && InputContainerIndex <= NumberOfButtonsInGrids + Dimension - 1;
	if (bFoundContainer)
	{
		DISPLAYERROR("In order to append InputContainer navigation, use Append Navigation Grid 2D");
		return;
	}

	Add1DGrid(GridType, UINavButtons[NumberOfButtonsInGrids], NavigationGrids.Num(), Dimension, EdgeNavigation, bWrap);

	int GridIndex = NavigationGrids.Num() - 1;
	for (int i = 0; i < Dimension; i++)
	{
		if (NumberOfButtonsInGrids + i >= UINavButtons.Num()) break;

		UINavButtons[NumberOfButtonsInGrids + i]->GridIndex = GridIndex;
		UINavButtons[NumberOfButtonsInGrids + i]->IndexInGrid = i;
	}

	NumberOfButtonsInGrids += Dimension;
}

void UUINavWidget::AppendNavigationGrid2D(int DimensionX, int DimensionY, FButtonNavigation EdgeNavigation, bool bWrap, int ButtonsInGrid)
{
	if (DimensionX <= 0 || DimensionY <= 0)
	{
		DISPLAYERROR("AppendNavigationGrid2D Dimensions should be greater than 0");
		return;
	}
	if (UINavButtons.Num() == 0)
	{
		DISPLAYERROR("Make sure to call the Append Navigation functions only during the ReadyForSetup event!");
		return;
	}
	else if (NumberOfButtonsInGrids >= UINavButtons.Num())
	{
		DISPLAYERROR("Not enough UINavButtons to append this navigation grid!");
		return;
	}

	FButtonNavigation NewNav;

	if (NumberOfButtonsInGrids >= UINavButtons.Num() || UINavButtons.Num() == 0)
	{
		DISPLAYERROR("Not enough UINavButtons to add specified navigation dimensions!");
		return;
	}

	FGrid NewGrid = FGrid(EGridType::Grid2D, UINavButtons[NumberOfButtonsInGrids], NavigationGrids.Num(), DimensionX, DimensionY, EdgeNavigation, bWrap, ButtonsInGrid);
	NavigationGrids.Add(NewGrid);

	int GridIndex = NavigationGrids.Num() - 1;
	int Iterations = NewGrid.NumGrid2DButtons;
	for (int i = 0; i < Iterations; i++)
	{
		if (NumberOfButtonsInGrids + i >= UINavButtons.Num()) break;

		UINavButtons[NumberOfButtonsInGrids + i]->GridIndex = GridIndex;
		UINavButtons[NumberOfButtonsInGrids + i]->IndexInGrid = i;
	}

	NumberOfButtonsInGrids = NumberOfButtonsInGrids + Iterations;
}

void UUINavWidget::AppendCollection(const TArray<FButtonNavigation>& EdgeNavigations)
{
	if (CollectionIndex >= UINavCollections.Num())
	{
		DISPLAYERROR("Can't append UINavCollection to navigation, no remaining UINavCollection found!");
		return;
	}

	UINavCollections[CollectionIndex]->FirstGridIndex = NavigationGrids.Num();
	UINavCollections[CollectionIndex]->SetupNavigation(EdgeNavigations);

	CollectionIndex++;
}

void UUINavWidget::AppendHorizontalNavigation(int Dimension, FButtonNavigation EdgeNavigation, bool bWrap)
{
	AppendNavigationGrid1D(EGridType::Horizontal, Dimension, EdgeNavigation, bWrap);
}

void UUINavWidget::AppendVerticalNavigation(int Dimension, FButtonNavigation EdgeNavigation, bool bWrap)
{
	AppendNavigationGrid1D(EGridType::Vertical, Dimension, EdgeNavigation, bWrap);
}

void UUINavWidget::AppendGridNavigation(int DimensionX, int DimensionY, FButtonNavigation EdgeNavigation, bool bWrap)
{
	AppendNavigationGrid2D(DimensionX, DimensionY, EdgeNavigation, bWrap);
}

void UUINavWidget::Add1DGrid(EGridType GridType, UUINavButton * FirstButton, int StartingIndex, int Dimension, FButtonNavigation EdgeNavigation, bool bWrap)
{
	if (GridType == EGridType::Vertical)
	{
		NavigationGrids.Add(FGrid(EGridType::Vertical, FirstButton, StartingIndex, 0, Dimension, EdgeNavigation, bWrap));
	}
	else if (GridType == EGridType::Horizontal)
	{
		NavigationGrids.Add(FGrid(EGridType::Horizontal, FirstButton, StartingIndex, Dimension, 0, EdgeNavigation, bWrap));
	}
}

void UUINavWidget::UpdateSelectorLocation(int Index)
{
	if (TheSelector == nullptr) return;
	TheSelector->SetRenderTranslation(GetButtonLocation(Index));
}

FVector2D UUINavWidget::GetButtonLocation(int Index)
{
	FGeometry Geom = UINavButtons[Index]->GetCachedGeometry();
	FVector2D LocalSize = Geom.GetLocalSize();
	FVector2D LocalPosition;
	switch (SelectorPositioning)
	{
		case ESelectorPosition::Position_Center:
			LocalPosition = LocalSize / 2;
			break;
		case ESelectorPosition::Position_Top:
			LocalPosition = FVector2D(LocalSize.X / 2, 0.f);
			break;
		case ESelectorPosition::Position_Bottom:
			LocalPosition = FVector2D(LocalSize.X / 2, LocalSize.Y);
			break;
		case ESelectorPosition::Position_Left:
			LocalPosition = FVector2D(0.f, LocalSize.Y / 2);
			break;
		case ESelectorPosition::Position_Right:
			LocalPosition = FVector2D(LocalSize.X, LocalSize.Y / 2);
			break;
		case ESelectorPosition::Position_Top_Right:
			LocalPosition = FVector2D(LocalSize.X, 0.f);
			break;
		case ESelectorPosition::Position_Top_Left:
			LocalPosition = FVector2D(0.f, 0.f);
			break;
		case ESelectorPosition::Position_Bottom_Right:
			LocalPosition = FVector2D(LocalSize.X, LocalSize.Y);
			break;
		case ESelectorPosition::Position_Bottom_Left:
			LocalPosition = FVector2D(0.f, LocalSize.Y);
			break;
	}
	
	FVector2D PixelPos, ViewportPos;
	USlateBlueprintLibrary::LocalToViewport(GetWorld(), Geom, LocalPosition, PixelPos, ViewportPos);
	ViewportPos += SelectorOffset;
	return ViewportPos;
}

void UUINavWidget::ExecuteAnimations(int From, int To)
{
	if (From != -1 && UINavAnimations.Num() > From)
	{
		if (IsAnimationPlaying(UINavAnimations[From]))
		{
			ReverseAnimation(UINavAnimations[From]);
		}
		else
		{
			PlayAnimation(UINavAnimations[From], 0.0f, 1, EUMGSequencePlayMode::Reverse, AnimationPlaybackSpeed);
		}
	}

	if (UINavAnimations.Num() <= To) return;
	if (IsAnimationPlaying(UINavAnimations[To]))
	{
		ReverseAnimation(UINavAnimations[To]);
	}
	else
	{
		PlayAnimation(UINavAnimations[To], 0.0f, 1, EUMGSequencePlayMode::Forward, AnimationPlaybackSpeed);
	}
}

void UUINavWidget::UpdateTextColor(int Index)
{
	SwitchTextColorTo(ButtonIndex, TextDefaultColor);
	SwitchTextColorTo(Index, TextNavigatedColor);
}

void UUINavWidget::SwitchTextColorTo(int Index, FLinearColor Color)
{
	UTextBlock* NewText = nullptr;
	int NewComponentIndex = GetLocalComponentIndex(Index);
	if (NewComponentIndex != -1)
	{
		NewText = UINavComponents[NewComponentIndex]->NavText;
		if (NewText == nullptr)
		{
			DISPLAYERROR("When UseTextColor is true, UINavComponent should have a valid TextBlock called NavText.");
			return;
		}
	}
	else
	{
		NewText = Cast<UTextBlock>(UINavButtons[Index]->GetChildAt(0));
		if (NewText == nullptr)
		{
			DISPLAYERROR("When UseTextColor is true, UINavButton should have a TextBlock as its child.");
			return;
		}
	}
	NewText->SetColorAndOpacity(Color);
}

void UUINavWidget::UpdateButtonsStates(int Index, bool bHovered)
{
	UUINavButton* ToButton = UINavButtons[Index];
	//Update new button state
	if (!(bHovered ^ ToButton->bSwitchedStyle))
	{
		SwitchButtonStyle(Index);
		ToButton->bSwitchedStyle = !bHovered;
	}

	if (ButtonIndex == Index) return;

	//Update previous button state
	if (!(bHovered ^ !ToButton->bSwitchedStyle))
	{
		SwitchButtonStyle(ButtonIndex);
		CurrentButton->bSwitchedStyle = false;
	}
}

void UUINavWidget::SwitchButtonStyle(int Index)
{
	UUINavButton* TheButton = UINavButtons[Index];
	FButtonStyle NewStile = TheButton->WidgetStyle;
	FSlateBrush TempState;
	TempState = NewStile.Hovered;
	NewStile.Hovered = NewStile.Normal;
	NewStile.Normal = TempState;
	TheButton->SetStyle(NewStile);
}

void UUINavWidget::SetSelectorScale(FVector2D NewScale)
{
	if (TheSelector == nullptr) return;
	TheSelector->SetRenderScale(NewScale);
}

void UUINavWidget::SetSelectorVisibility(bool bVisible)
{
	if (TheSelector == nullptr) return;
	ESlateVisibility Vis = bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
	TheSelector->SetVisibility(Vis);
}

bool UUINavWidget::IsSelectorVisible()
{
	if (TheSelector == nullptr) return false;
	return TheSelector->GetVisibility() == ESlateVisibility::HitTestInvisible;
}

void UUINavWidget::NavigateTo(int Index, bool bHoverEvent)
{
	if (Index >= UINavButtons.Num() || Index == ButtonIndex) return;

	DispatchNavigation(Index, bHoverEvent);
	OnNavigate(ButtonIndex, Index);

	ButtonIndex = Index;
	CurrentButton = UINavButtons[ButtonIndex];
}

void UUINavWidget::DispatchNavigation(int Index, bool bHoverEvent)
{
	//Update all the possible scroll boxes in the widget
	for (int i = 0; i < ScrollBoxes.Num(); ++i)
	{
		ScrollBoxes[i]->ScrollWidgetIntoView(UINavButtons[Index], bAnimateScrollBoxes);
	}

	if (bUseButtonStates) UpdateButtonsStates(Index, bHoverEvent);

	if (IsSelectorValid())
	{
		if (MoveCurve != nullptr) BeginSelectorMovement(Index);
		else UpdateSelectorLocation(Index);
	}

	if (bUseTextColor) UpdateTextColor(Index);

	UUINavComponent* FromComponent = GetUINavComponentAtIndex(ButtonIndex);
	UUINavComponent* ToComponent = GetUINavComponentAtIndex(Index);
	if (FromComponent != nullptr) FromComponent->OnNavigatedFrom();
	if (ToComponent != nullptr) ToComponent->OnNavigatedTo();

	if (UINavAnimations.Num() > 0) ExecuteAnimations(ButtonIndex, Index);
}

void UUINavWidget::BeginSelectorMovement(int Index)
{
	if (MoveCurve == nullptr) return;

	SelectorOrigin = bMovingSelector ? TheSelector->RenderTransform.Translation : GetButtonLocation(ButtonIndex);
	SelectorDestination = GetButtonLocation(Index);
	Distance = SelectorDestination - SelectorOrigin;

	float MinTime, MaxTime;
	MoveCurve->GetTimeRange(MinTime, MaxTime);
	MovementTime = MaxTime - MinTime;
	MovementCounter = 0.0f;

	bMovingSelector = true;
	if (UINavPC != nullptr) UINavPC->bAllowNavigation = false;
}

void UUINavWidget::OnNavigate_Implementation(int From, int To)
{

}

void UUINavWidget::OnSelect_Implementation(int Index)
{

}

void UUINavWidget::OnPreSelect(int Index)
{
	if (UINavInputContainer != nullptr && Index >= UINavInputContainer->FirstButtonIndex && Index <= UINavInputContainer->LastButtonIndex)
	{
		InputBoxIndex = Index - UINavInputContainer->FirstButtonIndex;
		int KeysPerInput = UINavInputContainer->KeysPerInput;
		UINavInputBoxes[InputBoxIndex / KeysPerInput]->NotifySelected(InputBoxIndex % KeysPerInput);
		ReceiveInputType = UINavInputBoxes[InputBoxIndex / KeysPerInput]->bIsAxis ? EReceiveInputType::Axis : EReceiveInputType::Action;
		SetUserFocus(Cast<APlayerController>(UINavPC->GetOwner()));
		bWaitForInput = true;
	}
	else
	{
		OnSelect(Index);
	}
}

void UUINavWidget::OnReturn_Implementation()
{
	ReturnToParent();
}

void UUINavWidget::OnNavigatedDirection_Implementation(ENavigationDirection Direction)
{
}

void UUINavWidget::OnInputChanged_Implementation(EInputType From, EInputType To)
{

}

void UUINavWidget::PreSetup_Implementation()
{
	InitialSetup();
}

void UUINavWidget::OnSetupCompleted_Implementation()
{

}

void UUINavWidget::OnHorizCompNavigateLeft_Implementation(int Index)
{
}

void UUINavWidget::OnHorizCompNavigateRight_Implementation(int Index)
{
}

UWidget* UUINavWidget::GoToWidget(TSubclassOf<UUINavWidget> NewWidgetClass, bool bRemoveParent, int ZOrder)
{
	if (NewWidgetClass == nullptr)
	{
		DISPLAYERROR("GoToWidget: No Widget Class found");
		return nullptr;
	}

	APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
	UUINavWidget* NewWidget = CreateWidget<UUINavWidget>(PC, NewWidgetClass);
	return GoToBuiltWidget(NewWidget, bRemoveParent, ZOrder);
}

UWidget * UUINavWidget::GoToBuiltWidget(UUINavWidget* NewWidget, bool bRemoveParent, int ZOrder)
{
	if (NewWidget == nullptr) return nullptr;
	APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
	NewWidget->ParentWidget = this;
	NewWidget->bParentRemoved = bRemoveParent;
	if (HasUserFocus(PC))
		NewWidget->SetUserFocus(PC);
	NewWidget->AddToViewport(ZOrder);
	CleanSetup();
	return NewWidget;
}

void UUINavWidget::ReturnToParent()
{
	if (ParentWidget == nullptr)
	{
		if (bAllowRemoveIfRoot)
		{
			IUINavPCReceiver::Execute_OnRootWidgetRemoved(UINavPC->GetOwner());
			RemoveFromParent();
		}
		return;
	}

	if (HasUserFocus(Cast<APlayerController>(UINavPC->GetOwner())))
		ParentWidget->SetUserFocus(Cast<APlayerController>(UINavPC->GetOwner()));

	//If parent was removed, add it to viewport
	if (bParentRemoved)
	{
		ParentWidget->ReturnedFromWidget = this;
		ParentWidget->AddToViewport();
	}
	else
	{
		UINavPC->ActiveWidget = ParentWidget;
		ParentWidget->ReconfigureSetup();
	}
	RemoveFromParent();
}

void UUINavWidget::MenuNavigate(ENavigationDirection Direction)
{
	UUINavButton* NewButton = FindNextButton(CurrentButton, Direction);
	if (NewButton == nullptr) return;
	NavigateTo(NewButton->ButtonIndex);
}

int UUINavWidget::GetLocalComponentIndex(int Index)
{
	for (int i = 0; i < UINavComponents.Num(); i++)
	{
		if (UINavComponents[i]->ComponentIndex == Index) return i;
		if (UINavComponents[i]->ComponentIndex > Index) return -1;
	}
	return -1;
}

int UUINavWidget::GetLocalHorizontalCompIndex(int Index)
{
	for (int i = 0; i < UINavHorizontalComps.Num(); i++)
	{
		if (UINavHorizontalComps[i]->ComponentIndex == Index) return i;
		if (UINavHorizontalComps[i]->ComponentIndex > Index) return -1;
	}
	return -1;
}

bool UUINavWidget::IsSelectorValid()
{
	return  TheSelector != nullptr &&
			TheSelector->bIsEnabled;
}

UUINavButton* UUINavWidget::FindNextButton(UUINavButton* Button, ENavigationDirection Direction)
{
	if (Button == nullptr || Direction == ENavigationDirection::None) return nullptr;

	UUINavButton* NewButton = FetchButtonByDirection(Direction, Button);
	if (NewButton == nullptr || NewButton == Button) return nullptr;

	//Check if the button is visible, if not, skip to next button
	bool bValid = NewButton->IsValid();
	if (bValid)
	{
		UUINavComponent* UINavComp = GetUINavComponentAtIndex(NewButton->ButtonIndex);
		if (UINavComp != nullptr && !UINavComp->IsValid()) bValid = false;
	}
	while (!bValid)
	{
		bValid = false;
		NewButton = FetchButtonByDirection(Direction, NewButton);
		if (NewButton == nullptr) return nullptr;
		UUINavComponent* UINavComp = GetUINavComponentAtIndex(NewButton->ButtonIndex);
		if (UINavComp != nullptr && !UINavComp->IsValid()) continue;
		if (NewButton == nullptr || NewButton == UINavButtons[ButtonIndex]) return nullptr;

		bValid = NewButton->IsValid();
	}
	return NewButton;
}

UUINavButton* UUINavWidget::FetchButtonByDirection(ENavigationDirection Direction, UUINavButton* Button)
{
	UUINavButton* NextButton = nullptr;

	FGrid ButtonGrid;
	bool bIsValid;
	GetButtonGrid(Button, ButtonGrid, bIsValid);

	if (!bIsValid || ButtonGrid.FirstButton == nullptr) return nullptr;

	switch (ButtonGrid.GridType)
	{
		case EGridType::Horizontal:
			switch (Direction)
			{
				case ENavigationDirection::Up:
					if (ButtonGrid.EdgeNavigation.UpButton != nullptr) NextButton = ButtonGrid.EdgeNavigation.UpButton;
					else NextButton = nullptr;
					break;
				case ENavigationDirection::Down:
					if (ButtonGrid.EdgeNavigation.DownButton != nullptr) NextButton = ButtonGrid.EdgeNavigation.DownButton;
					else NextButton = nullptr;
					break;
				case ENavigationDirection::Left:
					if (Button->IndexInGrid == 0)
					{
						if (ButtonGrid.EdgeNavigation.LeftButton != nullptr) NextButton = ButtonGrid.EdgeNavigation.LeftButton;
						else if (ButtonGrid.bWrap) NextButton = UINavButtons[ButtonGrid.FirstButton->ButtonIndex + ButtonGrid.DimensionX - 1];
						else NextButton = nullptr;
					}
					else
					{
						NextButton = UINavButtons[Button->ButtonIndex - 1];
					}
					break;
				case ENavigationDirection::Right:
					if (Button->IndexInGrid+1 >= ButtonGrid.DimensionX)
					{
						if (ButtonGrid.EdgeNavigation.RightButton != nullptr) NextButton = ButtonGrid.EdgeNavigation.RightButton;
						else if (ButtonGrid.bWrap) NextButton = ButtonGrid.FirstButton;
						else NextButton = nullptr;
					}
					else NextButton = UINavButtons[Button->ButtonIndex + 1];
					break;
			}
			break;
		case EGridType::Vertical:
			switch (Direction)
			{
				case ENavigationDirection::Up:
					if (Button->IndexInGrid == 0)
					{
						if (ButtonGrid.EdgeNavigation.UpButton != nullptr) NextButton = ButtonGrid.EdgeNavigation.UpButton;
						else if (ButtonGrid.bWrap) NextButton = UINavButtons[ButtonGrid.FirstButton->ButtonIndex + ButtonGrid.DimensionY - 1];
						else NextButton = nullptr;
					}
					else
					{
						NextButton = UINavButtons[Button->ButtonIndex - 1];
					}
					break;
				case ENavigationDirection::Down:
					if (Button->IndexInGrid+1 >= ButtonGrid.DimensionY)
					{
						if (ButtonGrid.EdgeNavigation.DownButton != nullptr) NextButton = ButtonGrid.EdgeNavigation.DownButton;
						else if (ButtonGrid.bWrap) NextButton = ButtonGrid.FirstButton;
						else NextButton = nullptr;
					}
					else NextButton = UINavButtons[Button->ButtonIndex + 1];
					break;
				case ENavigationDirection::Left:
					if (ButtonGrid.EdgeNavigation.LeftButton != nullptr) NextButton = ButtonGrid.EdgeNavigation.LeftButton;
					else NextButton = nullptr;
					break;
				case ENavigationDirection::Right:
					if (ButtonGrid.EdgeNavigation.RightButton != nullptr) NextButton = ButtonGrid.EdgeNavigation.RightButton;
					else NextButton = nullptr;
					break;
			}
			break;
		case EGridType::Grid2D:
			switch (Direction)
			{
				case ENavigationDirection::Up:
					if (Button->IndexInGrid < ButtonGrid.DimensionX)
					{
						if (ButtonGrid.EdgeNavigation.UpButton != nullptr) NextButton = ButtonGrid.EdgeNavigation.UpButton;
						else if (ButtonGrid.bWrap)
						{
							int Offset = ButtonGrid.DimensionX * (ButtonGrid.DimensionY - 1) + Button->IndexInGrid;
							NextButton = UINavButtons[ButtonGrid.FirstButton->ButtonIndex + (Offset >= ButtonGrid.NumGrid2DButtons ? Offset - ButtonGrid.DimensionX : Offset)];
						}
						else NextButton = nullptr;
					}
					else NextButton = UINavButtons[Button->ButtonIndex - ButtonGrid.DimensionX];
					break;
				case ENavigationDirection::Down:
					if (Button->IndexInGrid + ButtonGrid.DimensionX >= ButtonGrid.GetDimension())
					{
						if (ButtonGrid.EdgeNavigation.DownButton != nullptr) NextButton = ButtonGrid.EdgeNavigation.DownButton;
						else if (ButtonGrid.bWrap) NextButton = UINavButtons[ButtonGrid.FirstButton->ButtonIndex + (Button->IndexInGrid % ButtonGrid.DimensionX)];
						else NextButton = nullptr;
					}
					else NextButton = UINavButtons[Button->ButtonIndex + ButtonGrid.DimensionX];
					break;
				case ENavigationDirection::Left:
					if (Button->IndexInGrid % ButtonGrid.DimensionX == 0)
					{
						if (ButtonGrid.EdgeNavigation.LeftButton != nullptr) NextButton = ButtonGrid.EdgeNavigation.LeftButton;
						else if (ButtonGrid.bWrap) NextButton = UINavButtons[FMath::Min(Button->ButtonIndex - 1 + ButtonGrid.DimensionX, ButtonGrid.GetLastButtonIndex())];
						else NextButton = nullptr;
					}
					else NextButton = UINavButtons[Button->ButtonIndex - 1];
					break;
				case ENavigationDirection::Right:
					if ((Button->IndexInGrid + 1) % ButtonGrid.DimensionX == 0)
					{
						if (ButtonGrid.EdgeNavigation.RightButton != nullptr) NextButton = ButtonGrid.EdgeNavigation.RightButton;
						else if (ButtonGrid.bWrap) NextButton = UINavButtons[Button->ButtonIndex + 1 - ButtonGrid.DimensionX];
						else NextButton = nullptr;
					}
					else if ((Button->IndexInGrid + 1) >= ButtonGrid.NumGrid2DButtons)
					{
						if (ButtonGrid.EdgeNavigation.RightButton != nullptr) NextButton = ButtonGrid.EdgeNavigation.RightButton;
						else if (ButtonGrid.bWrap) NextButton = UINavButtons[Button->ButtonIndex + 1 - (Button->IndexInGrid + 1) % ButtonGrid.DimensionX];
						else NextButton = nullptr;
					}
					else NextButton = UINavButtons[Button->ButtonIndex + 1];
					break;
			}
			break;
	}
	return NextButton;
}

void UUINavWidget::GetButtonGrid(UUINavButton * Button, FGrid& ButtonGrid, bool& IsValid)
{
	if (NavigationGrids.Num() > Button->GridIndex)
	{
		IsValid = true;
		ButtonGrid = NavigationGrids[Button->GridIndex];
		return;
	}
	else
	{
		IsValid = false;
		return;
	}
}

int UUINavWidget::GetGridStartingIndex(int GridIndex)
{
	if (GridIndex < 0 || GridIndex >= NavigationGrids.Num()) return -1;

	if (GridIndex == 0) return 0;
	else return (NavigationGrids[GridIndex - 1].GetLastButtonIndex() + 1);
}

UUINavButton * UUINavWidget::GetLastButtonInGrid(const FGrid Grid)
{
	int LastIndex = Grid.GetLastButtonIndex();

	return UINavButtons.Num() > LastIndex ? UINavButtons[LastIndex] : nullptr;
}

UUINavButton * UUINavWidget::GetButtonAtGridIndex(const FGrid ButtonGrid, const int GridIndex)
{
	if (ButtonGrid.FirstButton == nullptr || GridIndex < 0) return nullptr;
	int NewIndex = ButtonGrid.FirstButton->ButtonIndex + GridIndex;

	if (NewIndex >= UINavButtons.Num()) return nullptr;

	return UINavButtons[NewIndex];
}

bool UUINavWidget::IsButtonInGrid(UUINavButton * Button, const FGrid Grid)
{
	if (Button == nullptr) return false;
	return Button->GridIndex == Grid.GridIndex;
}

void UUINavWidget::GetCoordinatesInGrid2D_FromIndex(const int Index, int & XCoord, int & YCoord)
{
	XCoord = -1;
	YCoord = -1;
	if (ButtonIndex < 0 || ButtonIndex > UINavButtons.Num()) return;

	GetCoordinatesInGrid2D_FromButton(UINavButtons[ButtonIndex], XCoord, YCoord);
}

void UUINavWidget::GetCoordinatesInGrid2D_FromButton(UUINavButton * Button, int & XCoord, int & YCoord)
{
	XCoord = -1;
	YCoord = -1;
	if (Button == nullptr) return;

	FGrid Grid = NavigationGrids[Button->GridIndex];
	if (Grid.GridType != EGridType::Grid2D) return;

	YCoord = Button->IndexInGrid / Grid.DimensionX;
	XCoord = Button->IndexInGrid - (YCoord * Grid.DimensionX);
}

UUINavButton * UUINavWidget::GetButtonFromCoordinatesInGrid2D(const FGrid Grid, int XCoord, int YCoord)
{
	int Index = GetButtonIndexFromCoordinatesInGrid2D(Grid, XCoord, YCoord);
	if (Index == -1) return nullptr;

	return UINavButtons[Index];
}

int UUINavWidget::GetButtonIndexFromCoordinatesInGrid2D(const FGrid Grid, int XCoord, int YCoord)
{
	if (Grid.GridType != EGridType::Grid2D ||
		Grid.FirstButton == nullptr ||
		XCoord < 0 ||
		YCoord < 0 ||
		XCoord >= Grid.DimensionX ||
		YCoord >= Grid.DimensionY ||
		XCoord * YCoord + XCoord > Grid.NumGrid2DButtons)
		return -1;

	int Index = Grid.FirstButton->ButtonIndex + YCoord * Grid.DimensionX + XCoord;
	if (Index >= UINavButtons.Num()) return -1;

	return Index;
}

UUINavComponent * UUINavWidget::GetUINavComponentAtIndex(int Index)
{
	int ValidIndex = GetLocalComponentIndex(Index);
	if (ValidIndex == -1) return nullptr;
	
	return UINavComponents[ValidIndex];
}

UUINavHorizontalComponent * UUINavWidget::GetUINavHorizontalCompAtIndex(int Index)
{
	int ValidIndex = GetLocalHorizontalCompIndex(Index);
	if (ValidIndex == -1) return nullptr;
	
	return UINavHorizontalComps[ValidIndex];
}

void UUINavWidget::HoverEvent(int Index)
{
	if (bWaitForInput)
	{
		bWaitForInput = false;
		UINavInputBoxes[InputBoxIndex / UINavInputContainer->KeysPerInput]->RevertToActionText(InputBoxIndex % UINavInputContainer->KeysPerInput);
	}

	if (UINavPC->GetCurrentInputType() != EInputType::Mouse || Index == ButtonIndex)
	{
		if (bUseButtonStates) SwitchButtonStyle(Index);
		return;
	}

	UINavPC->ClearTimer();
	NavigateTo(Index, true);
}

void UUINavWidget::UnhoverEvent(int Index)
{
	if (bWaitForInput)
	{
		bWaitForInput = false;
		UINavInputBoxes[InputBoxIndex / UINavInputContainer->KeysPerInput]->RevertToActionText(InputBoxIndex % UINavInputContainer->KeysPerInput);
	}

	if (bUseButtonStates)
	{
		/*
		If the button didn't switch style, switch style to make sure it's still selected
		Otherwise, if the button is the selected button, also switch style to make sure it's still selected
		*/
		UUINavButton* ToButton = UINavButtons[Index];
		if (!ToButton->bSwitchedStyle || (ToButton->bSwitchedStyle && ButtonIndex == Index))
		{
			SwitchButtonStyle(Index);
			ToButton->bSwitchedStyle = ButtonIndex == Index;
		}

		ButtonIndex = ToButton->bSwitchedStyle ? Index : ButtonIndex;
	}
}

void UUINavWidget::ClickEvent(int Index)
{
	if (bWaitForInput)
	{
		if (ReceiveInputType == EReceiveInputType::Axis) CancelRebind();
		else ProcessMouseKeybind(FKey(EKeys::LeftMouseButton));
	}
	else
	{
		UINavPC->NotifyMouseInputType();

		if (!UINavPC->bAllowNavigation) return;

		OnPreSelect(Index);

		if (Index != ButtonIndex) NavigateTo(Index);
	}
}

void UUINavWidget::ReleaseEvent(int Index)
{
	if (!UINavButtons[Index]->IsHovered()) SwitchButtonStyle(Index);
}

void UUINavWidget::SetupUINavButtonDelegates(UUINavButton * NewButton)
{
	NewButton->CustomHover.AddDynamic(this, &UUINavWidget::HoverEvent);
	NewButton->CustomUnhover.AddDynamic(this, &UUINavWidget::UnhoverEvent);
	NewButton->CustomClick.AddDynamic(this, &UUINavWidget::ClickEvent);
	NewButton->CustomRelease.AddDynamic(this, &UUINavWidget::ReleaseEvent);
}

void UUINavWidget::ProcessNonMouseKeybind(FKey PressedKey)
{
	int KeysPerInput = UINavInputContainer->KeysPerInput;
	UINavInputBoxes[InputBoxIndex / KeysPerInput]->UpdateInputKey(PressedKey, InputBoxIndex % KeysPerInput);
	bWaitForInput = false;
	ReceiveInputType = EReceiveInputType::None;
}

void UUINavWidget::ProcessMouseKeybind(FKey PressedMouseKey)
{
	int KeysPerInput = UINavInputContainer->KeysPerInput;
	if (ReceiveInputType == EReceiveInputType::Axis) PressedMouseKey = FKey(FName("MouseWheelAxis"));
	UINavInputBoxes[InputBoxIndex / KeysPerInput]->UpdateInputKey(PressedMouseKey, InputBoxIndex % KeysPerInput);
	bWaitForInput = false;
	ReceiveInputType = EReceiveInputType::None;
}

void UUINavWidget::CancelRebind()
{
	bWaitForInput = false;
	int KeysPerInput = UINavInputContainer->KeysPerInput;
	UINavInputBoxes[InputBoxIndex / KeysPerInput]->RevertToActionText(InputBoxIndex % KeysPerInput);
	ReceiveInputType = EReceiveInputType::None;
}

void UUINavWidget::NavigateInDirection(ENavigationDirection Direction)
{
	if (Direction == ENavigationDirection::None) return;

	if (NumberOfButtonsInGrids == 0)
	{
		OnNavigatedDirection(Direction);
		return;
	}

	if (!UINavPC->bAllowNavigation)
	{
		UUINavButton* NextButton = FindNextButton(CurrentButton, Direction);
		HaltedIndex = NextButton != nullptr ? NextButton->ButtonIndex : -1;
		return;
	}

	OnNavigatedDirection(Direction);

	UUINavHorizontalComponent* HorizComp = GetUINavHorizontalCompAtIndex(ButtonIndex);
	if (HorizComp != nullptr && (Direction == ENavigationDirection::Left || Direction == ENavigationDirection::Right))
	{
		if (Direction == ENavigationDirection::Left)
		{
			HorizComp->NavigateLeft();
			OnHorizCompNavigateLeft(ButtonIndex);
		}
		else if (Direction == ENavigationDirection::Right)
		{
			HorizComp->NavigateRight();
			OnHorizCompNavigateRight(ButtonIndex);
		}
	}
	else MenuNavigate(Direction);
}

void UUINavWidget::MenuSelect()
{
	if (!UINavPC->bAllowNavigation)
	{
		HaltedIndex = SELECT_INDEX;
		return;
	}
	UINavPC->ClearTimer();
	OnPreSelect(ButtonIndex);
}

void UUINavWidget::MenuReturn()
{
	if (!UINavPC->bAllowNavigation)
	{
		HaltedIndex = RETURN_INDEX;
		return;
	}
	UINavPC->ClearTimer();
	OnReturn();
}
