// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#include "UINavWidget.h"
#include "UINavCollection.h"
#include "UINavButton.h"
#include "UINavHorizontalComponent.h"
#include "UINavComponent.h"
#include "UINavComponentWrapper.h"
#include "UINavInputBox.h"
#include "UINavInputContainer.h"
#include "UINavPCComponent.h"
#include "UINavPCReceiver.h"
#include "UINavWidgetComponent.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/OverlaySlot.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ActorComponent.h"

UUINavWidget::UUINavWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bIsFocusable = true;
}

void UUINavWidget::NativeConstruct()
{
	/*
	If this widget was added through a parent widget and should remove it from the viewport,
	remove that widget from viewport
	*/
	if (ParentWidget != nullptr && ParentWidget->IsInViewport() && bParentRemoved)
	{
		ParentWidget->RemoveFromParent();

		if (bShouldDestroyParent)
		{
			ParentWidget->Destruct();
			ParentWidget = nullptr;
		}
	}

	//If this widget was added through a child widget, destroy it
	if (ReturnedFromWidget != nullptr)
	{
		APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
		if (ReturnedFromWidget->HasUserFocus(PC))
		{
			SetUserFocus(PC);
			if (UINavPC->GetInputMode() == EInputMode::UI)
			{
				SetKeyboardFocus();
			}
		}

		if (WidgetComp == nullptr) ReturnedFromWidget->Destruct();
		ReturnedFromWidget = nullptr;
	}

	PreSetup(!bCompletedSetup);
	InitialSetup();

	Super::NativeConstruct();
}

void UUINavWidget::InitialSetup(bool bRebuilding)
{
	if (!bRebuilding)
	{
		//If widget was already setup, apply only certain steps
		if (bCompletedSetup)
		{
			ReconfigureSetup();
			return;
		}

		bSetupStarted = true;
		WidgetClass = GetClass();
		if (UINavPC == nullptr)
		{
			ConfigureUINavPC();
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
		button->bAutoCollapse = button->bIsEnabled;
		if (button->bIsEnabled)
		{
			button->SetIsEnabled(false);
		}

	}
	bSetupStarted = false;
	SelectedButtonIndex = -1;
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

void UUINavWidget::ConfigureUINavPC()
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
			Collection->ParentWidget = this;
			Collection->Init(UINavButtons.Num());
			UINavCollections.Add(Collection);
			continue;
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

				if (UIComp->ComponentIndex == -1) UIComp->ComponentIndex = UINavButtons.Num();
				NewNavButton->ButtonIndex = UIComp->ComponentIndex;

				UINavComponents.Add(UIComp);

				UUINavHorizontalComponent* HorizComp = Cast<UUINavHorizontalComponent>(UIComp);
				if (HorizComp != nullptr)
				{
					HorizComp->ParentWidget = this;
					UINavHorizontalComps.Add(HorizComp);
				}
			}
		}

		if (NewNavButton == nullptr) continue;

		if (NewNavButton->ButtonIndex == -1) NewNavButton->ButtonIndex = UINavButtons.Num();

		SetupUINavButtonDelegates(NewNavButton);

		NewNavButton->bAutoCollapse = NewNavButton->bIsEnabled;
		UINavButtons.Add(NewNavButton);
		RevertButtonStyle(UINavButtons.Num() - 1);
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

void UUINavWidget::RebuildNavigation(int NewButtonIndex)
{
	bCompletedSetup = false;
	bMovingSelector = false;
	bIgnoreMouseEvent = false;
	bReturning = false;
	bWaitForInput = false;
	bShouldTick = true;
	ReceiveInputType = EReceiveInputType::None;
	WaitForTick = 0;
	HaltedIndex = -1;
	SelectedButtonIndex = -1;
	SelectCount = 0;
	InputBoxIndex = -1;
	NumberOfButtonsInGrids = 0;
	CollectionIndex = 0;
	FirstButtonIndex = NewButtonIndex > -1 ? NewButtonIndex : FirstButtonIndex;
	UINavInputContainer = nullptr;
	CurrentButton = nullptr;

	NavigationGrids.Reset();
	UINavAnimations.Reset();
	ScrollBoxes.Reset();
	UINavButtons.Reset();
	UINavComponents.Reset();
	UINavHorizontalComps.Reset();
	UINavInputBoxes.Reset();
	UINavCollections.Reset();

	InitialSetup(true);
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
	if (UINavPC == nullptr) return;

	UINavPC->SetActiveWidget(this);
	if (UINavPC->GetInputMode() == EInputMode::UI)
	{
		SetUserFocus(UINavPC->GetPC());
		SetKeyboardFocus();
	}

	//Re-enable all buttons (bug fix)
	for (UUINavButton* button : UINavButtons)
	{
		if (button->bAutoCollapse)
		{
			button->SetIsEnabled(true);
		}
	}

	if (IsSelectorValid()) TheSelector->SetVisibility(ESlateVisibility::HitTestInvisible);

	bCompletedSetup = true;

	if (UINavButtons.Num() > 0)
	{
		DispatchNavigation(ButtonIndex);
		OnNavigate(-1, ButtonIndex);
		CollectionNavigateTo(ButtonIndex);

		bIgnoreMouseEvent = true;
		CurrentButton->OnHovered.Broadcast();
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
	if (ReceiveInputType != EReceiveInputType::None)
	{
		FKey PressedKey = InKeyEvent.GetKey();

		if (ReceiveInputType == EReceiveInputType::Axis)
		{
			PressedKey = UINavInputContainer->GetAxisFromKey(PressedKey);
		}

		ProcessKeybind(PressedKey);
		return FReply::Handled();
	}
	else
	{
		//Allow fullscreen by pressing F11 or Alt+Enter
		if (GEngine->GameViewport->TryToggleFullscreenOnInputKey(InKeyEvent.GetKey(), IE_Pressed))
		{
			return FReply::Handled();
		}

		if (OnKeyPressed(InKeyEvent.GetKey()).IsEventHandled())
		{
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UUINavWidget::NativeOnKeyUp(const FGeometry & InGeometry, const FKeyEvent & InKeyEvent)
{
	if (!bWaitForInput)
	{
		if (OnKeyReleased(InKeyEvent.GetKey()).IsEventHandled())
		{
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyUp(InGeometry, InKeyEvent);
}

FReply UUINavWidget::NativeOnMouseWheel(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent)
{
	if (bWaitForInput)
	{
		FKey PressedMouseKey = InMouseEvent.GetWheelDelta() > 0.f ? EKeys::MouseScrollUp : EKeys::MouseScrollDown;
		if (ReceiveInputType == EReceiveInputType::Axis) PressedMouseKey = EKeys::MouseWheelAxis;
		ProcessKeybind(PressedMouseKey);
		return FReply::Handled();
	}
	else
	{
		if (UINavPC->GetCurrentInputType() != EInputType::Mouse)
		{
			UINavPC->NotifyMouseInputType();
		}
	}
	return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
}

FReply UUINavWidget::NativeOnMouseButtonDown(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent)
{
	if (bWaitForInput)
	{
		if (ReceiveInputType == EReceiveInputType::Axis)
		{
			CancelRebind();
			return FReply::Handled();
		}
		ProcessKeybind(InMouseEvent.GetEffectingButton());
		return FReply::Handled();
	}
	else
	{
		if (OnKeyPressed(InMouseEvent.GetEffectingButton()).IsEventHandled())
		{
			return FReply::Handled();
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UUINavWidget::NativeOnMouseButtonUp(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent)
{
	Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	if (!bWaitForInput && InMouseEvent.GetEffectingButton().IsMouseButton())
	{
		OnKeyReleased(InMouseEvent.GetEffectingButton());
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UUINavWidget::OnKeyPressed(FKey PressedKey)
{
	FString ActionName = UINavPC->FindActionByKey(PressedKey);
	if (ActionName.Equals(TEXT("")))
	{
		UINavPC->VerifyInputTypeChangeByKey(PressedKey);
		return FReply::Unhandled();
	}

	return UINavPC->OnActionPressed(ActionName, PressedKey);
}

FReply UUINavWidget::OnKeyReleased(FKey PressedKey)
{
	FString ActionName = UINavPC->FindActionByKey(PressedKey);
	if (ActionName.Equals(TEXT(""))) return FReply::Unhandled();

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
		TheSelector->SetRenderTranslation(SelectorDestination);
		if (HaltedIndex != -1)
		{
			if (HaltedIndex == SELECT_INDEX) OnPreSelect(ButtonIndex);
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

	IncrementUINavButtonIndices(NewButton->ButtonIndex, TargetGridIndex);
	IncrementUINavComponentIndices(NewButton->ButtonIndex);

	if (UINavButtons.Num() == 1)
	{
		ButtonIndex = 0;
		CurrentButton = UINavButtons[0];
		DispatchNavigation(ButtonIndex);
		OnNavigate(-1, ButtonIndex);
	}
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

	NewComponent->NavButton->ButtonIndex = GetGridStartingIndex(TargetGridIndex) + IndexInGrid;
	NewComponent->ComponentIndex = NewComponent->NavButton->ButtonIndex;
	NewComponent->NavButton->GridIndex = TargetGrid.GridIndex;
	NewComponent->NavButton->IndexInGrid = IndexInGrid;
	SetupUINavButtonDelegates(NewComponent->NavButton);
	
	int TargetIndex = NewComponent->ComponentIndex;
	UINavButtons.Insert(NewComponent->NavButton, NewComponent->NavButton->ButtonIndex);
	InsertNewComponent(NewComponent, TargetIndex);

	IncrementUINavButtonIndices(NewComponent->ComponentIndex, TargetGridIndex);

	if (UINavButtons.Num() == 1)
	{
		ButtonIndex = 0;
		CurrentButton = UINavButtons[0];
		DispatchNavigation(ButtonIndex);
		OnNavigate(-1, ButtonIndex);
	}
}

void UUINavWidget::DeleteUINavElement(int Index, bool bAutoNavigate)
{
	if (Index < 0 || Index >= UINavButtons.Num()) return;

	if (Index == ButtonIndex)
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

		if (!bValid)
		{
			CurrentButton = nullptr;
			ButtonIndex = 0;
		}
		else if (Temp != nullptr && bAutoNavigate)
		{
			NavigateTo(Temp->ButtonIndex);
		}
	}
	else
	{
		ButtonIndex = CurrentButton->ButtonIndex;
	}

	UUINavButton* Button = UINavButtons[Index];
	DecrementGrid(NavigationGrids[Button->GridIndex], Button->IndexInGrid);

	DecrementUINavButtonIndices(Index, Button->GridIndex);
	DecrementUINavComponentIndices(Index);

	DeleteButtonEdgeNavigationRefs(Button);
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
	int FirstIndex = -1;
	if (IndexInGrid == 0)
	{
		FirstIndex = TargetGrid.FirstButton != nullptr ? TargetGrid.FirstButton->ButtonIndex : GetGridStartingIndex(TargetGrid.GridIndex);
		TargetGrid.FirstButton = NewButton;
		NewButton->IndexInGrid = 0;
	}
	else FirstIndex = TargetGrid.FirstButton->ButtonIndex;
	
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

	UpdateCollectionLastIndex(TargetGrid.GridIndex, true);
}

void UUINavWidget::DecrementGrid(FGrid & TargetGrid, int IndexInGrid)
{
	if (IndexInGrid == 0)
	{
		TargetGrid.FirstButton =
			(TargetGrid.GetDimension() > 1 && TargetGrid.FirstButton->ButtonIndex + 1 < UINavButtons.Num())
			? UINavButtons[TargetGrid.FirstButton->ButtonIndex + 1] 
			: nullptr;
	}

	if (TargetGrid.GridType == EGridType::Horizontal) TargetGrid.DimensionX--;
	else if (TargetGrid.GridType == EGridType::Vertical) TargetGrid.DimensionY--;
	else
	{
		TargetGrid.NumGrid2DButtons--;
		if (TargetGrid.NumGrid2DButtons <= (TargetGrid.DimensionX * (TargetGrid.DimensionY - 1)))
		{
			TargetGrid.DimensionY--;
		}
	}

	UpdateCollectionLastIndex(TargetGrid.GridIndex, false);
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
				bool bAdded = false;
				for (int i = 0; i < UINavComponents.Num(); i++)
				{
					if (UINavComponents[i]->ComponentIndex > TargetIndex)
					{
						UINavComponents.Insert(NewComponent, i);
						IncrementUINavComponentIndices(NewComponent->ComponentIndex);
						bAdded = true;
						break;
					}
				}
				if (!bAdded)
				{
					UINavComponents.Add(NewComponent);
				}
			}
		}
		else
		{
			UINavComponents.Add(NewComponent);
		}
	}
}

void UUINavWidget::IncrementUINavButtonIndices(int StartingIndex, int GridIndex)
{
	for (int i = StartingIndex + 1; i < UINavButtons.Num(); i++)
	{
		if (UINavButtons[i]->ButtonIndex != i)
		{
			UINavButtons[i]->ButtonIndex = i;
		}
		if (UINavButtons[i]->GridIndex == GridIndex)
		{
			UINavButtons[i]->IndexInGrid++;
		}
	}
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

void UUINavWidget::DecrementUINavButtonIndices(int StartingIndex, int GridIndex)
{
	for (int i = StartingIndex; i < UINavButtons.Num()-1; i++)
	{
		UINavButtons[i] = UINavButtons[i + 1];
		UINavButtons[i]->ButtonIndex = i;
		if (UINavButtons[i]->GridIndex == GridIndex)
		{
			UINavButtons[i]->IndexInGrid--;
		}
	}
	UINavButtons.RemoveAt(UINavButtons.Num()-1, 1, true);
}

void UUINavWidget::DecrementUINavComponentIndices(int StartingIndex)
{
	int ValidIndex = GetLocalComponentIndex(StartingIndex);
	UUINavComponent* Component = (ValidIndex != -1 ? UINavComponents[ValidIndex] : nullptr);

	for (int i = (ValidIndex != -1 ? ValidIndex : 0); i < UINavComponents.Num() - 1; i++)
	{
		if (ValidIndex != -1) UINavComponents[i] = UINavComponents[i + 1];
		if (UINavComponents[i]->ComponentIndex > StartingIndex) UINavComponents[i]->ComponentIndex--;
	}

	if (Component != nullptr)
	{
		UINavComponents.RemoveAt(UINavComponents.Num()-1);
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
	if (IndexInGrid <= -1 || IndexInGrid >= TargetGrid.GetDimension())
		IndexInGrid = TargetGrid.GetDimension();

	if (UINavAnimations.Num() > 0) DISPLAYERROR("Runtime manipulation not supported with navigation using animations.");

	int OldGridIndex = Button->GridIndex;
	int OldIndexInGrid = Button->IndexInGrid;

	int From = Index;
	int To = GetGridStartingIndex(TargetGridIndex) + IndexInGrid;
	if (TargetGrid.GridIndex != Button->GridIndex)
	{
		DecrementGrid(NavigationGrids[Button->GridIndex], Button->IndexInGrid);
		IncrementGrid(Button, TargetGrid, IndexInGrid);
	}

	if (IndexInGrid == 0) TargetGrid.FirstButton = Button;
	Button->IndexInGrid = IndexInGrid;

	if (From == To) return;
	
	UpdateArrays(From, To, OldGridIndex, OldIndexInGrid);

	ReplaceButtonInNavigationGrid(Button, OldGridIndex, OldIndexInGrid);

	if (Button == CurrentButton) UpdateCurrentButton(Button);
}

void UUINavWidget::MoveUINavElementToGrid2(int FromGridIndex, int FromIndexInGrid, int TargetGridIndex, int TargetIndexInGrid)
{
	if (FromGridIndex < 0 || FromGridIndex >= NavigationGrids.Num()) return;

	FGrid TargetGrid = NavigationGrids[FromGridIndex];
	if (TargetGrid.FirstButton == nullptr) return;

	if (FromIndexInGrid <= -1 || FromIndexInGrid >= TargetGrid.GetDimension())
	{
		FromIndexInGrid = TargetGrid.GetDimension() - 1;
	}

	MoveUINavElementToGrid(TargetGrid.FirstButton->ButtonIndex + FromIndexInGrid, TargetGridIndex, TargetIndexInGrid);
}

void UUINavWidget::UpdateArrays(int From, int To, int OldGridIndex, int OldIndexInGrid)
{
	UpdateButtonArray(From, To, OldGridIndex, OldIndexInGrid);
	UpdateComponentArray(From, To);
}

void UUINavWidget::UpdateButtonArray(int From, int To, int OldGridIndex, int OldIndexInGrid)
{
	UUINavButton* TempButton = UINavButtons[From];
	if (To >= UINavButtons.Num()) To = UINavButtons.Num() - 1;

	if (From < To)
	{
		for (int i = From + 1; i <= To; i++)
		{
			UINavButtons[i]->ButtonIndex--;
			UINavButtons[i - 1] = UINavButtons[i];
			if (UINavButtons[i]->GridIndex == OldGridIndex)
			{
				UINavButtons[i]->IndexInGrid--;
			}
			else if (UINavButtons[i]->GridIndex == TempButton->GridIndex)
			{
				UINavButtons[i]->IndexInGrid++;
			}
			if (i == To)
			{
				UINavButtons[i] = TempButton;
				UINavButtons[i]->ButtonIndex = i;
			}
		}

		int TargetGridDimension = NavigationGrids[TempButton->GridIndex].GetDimension();
		if (OldGridIndex != TempButton->GridIndex &&
			TempButton->IndexInGrid + 1 < TargetGridDimension)
		{
			for (int j = TempButton->IndexInGrid + 1; j < TargetGridDimension; j++)
			{
				UINavButtons[TempButton->ButtonIndex + j]->IndexInGrid++;
			}
		}
	}
	else
	{
		for (int i = From - 1; i >= To; i--)
		{
			UINavButtons[i]->ButtonIndex++;
			UINavButtons[i+1] = UINavButtons[i];
			if (UINavButtons[i]->GridIndex == OldGridIndex)
			{
				UINavButtons[i]->IndexInGrid--;
			}
			else if (UINavButtons[i]->GridIndex == TempButton->GridIndex)
			{
				UINavButtons[i]->IndexInGrid++;
			}

			if (i == To)
			{
				UINavButtons[i] = TempButton;
				UINavButtons[i]->ButtonIndex = i;
			}
		}

		int TargetGridDimension = NavigationGrids[OldGridIndex].GetDimension();
		if (OldGridIndex != TempButton->GridIndex &&
			OldIndexInGrid < TargetGridDimension)
		{
			for (int j = OldIndexInGrid; j < TargetGridDimension; j++)
			{
				UINavButtons[From + j + 1]->IndexInGrid--;
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

	if (Start == End)
	{
		UINavComponents[Start]->ComponentIndex = UINavComponents[Start]->NavButton->ButtonIndex;
		return;
	}

	if (From < To)
	{
		if (TempComp != nullptr) Start = UINavComponents.Find(TempComp);

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

void UUINavWidget::UpdateCollectionLastIndex(int GridIndex, bool bAdded)
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

	for (UScrollBox* ScrollBox : ScrollBoxes)
	{
		if (NewCurrentButton->IsChildOf(ScrollBox))
		{
			ScrollBox->ScrollWidgetIntoView(NewCurrentButton, bAnimateScrollBoxes);
			break;
		}
	}
}

void UUINavWidget::ClearGrid(int GridIndex, bool bAutoNavigate)
{
	if (GridIndex < 0 || GridIndex >= NavigationGrids.Num()) return;

	FGrid& Grid = NavigationGrids[GridIndex];
	if (Grid.FirstButton == nullptr) return;

	int FirstIndex = Grid.FirstButton->ButtonIndex;
	int LastIndex = FirstIndex + Grid.GetDimension() - 1;
	int Difference = LastIndex - FirstIndex + 1;

	bool bShouldNavigate = bAutoNavigate && ButtonIndex >= FirstIndex && ButtonIndex <= LastIndex;
	if (bShouldNavigate)
	{
		bool bValid = false;

		UUINavButton* NextButton = UINavButtons[LastIndex];
		UUINavButton* FirstButton = NextButton;
		while (!bValid)
		{
			int NewIndex = NextButton->ButtonIndex + 1;
			if (NewIndex >= UINavButtons.Num()) NewIndex = 0;

			NextButton = UINavButtons[NewIndex];
			if (NextButton == FirstButton) break;

			if (NewIndex >= FirstIndex && NewIndex <= ButtonIndex) continue;

			UUINavComponent* UINavComp = GetUINavComponentAtIndex(NewIndex);
			if (UINavComp != nullptr && !UINavComp->IsValid()) continue;

			bValid = NextButton->IsValid();
		}

		if (bValid)
		{
			NavigateTo(NextButton->ButtonIndex);
		}
		else
		{
			CurrentButton = nullptr;
			ButtonIndex = 0;
		}
	}

	bool bDeletedFromEnd = true;
	int NumButtons = UINavButtons.Num();
	for (int i = FirstIndex; i < NumButtons; ++i)
	{
		if (i <= LastIndex)
		{
			UINavButtons.RemoveAt(FirstIndex);
			UUINavComponent* Component = GetUINavComponentAtIndex(i);
			if (Component != nullptr)
			{
				UINavComponents.Remove(Component);
			}
		}
		else
		{
			bDeletedFromEnd = false;
			UUINavButton* Button = UINavButtons[i - Difference];
			Button->ButtonIndex -= Difference;

			UUINavComponent* Component = GetUINavComponentAtIndex(i);
			if (Component != nullptr)
			{
				Component->ComponentIndex = Component->NavButton->ButtonIndex;
			}
		}
	}

	Grid.FirstButton = nullptr;
	if (Grid.GridType != EGridType::Grid2D)	Grid.DimensionX = 0;
	Grid.DimensionY = 0;
	Grid.NumGrid2DButtons = 0;

	if (bShouldNavigate)
	{
		ButtonIndex = CurrentButton->ButtonIndex;
	}

	DeleteGridEdgeNavigationRefs(GridIndex);
}

void UUINavWidget::DeleteButtonEdgeNavigationRefs(UUINavButton * Button)
{
	for (FGrid& Grid : NavigationGrids)
	{
		Grid.RemoveButtonFromEdgeNavigation(Button);
	}
}

void UUINavWidget::DeleteGridEdgeNavigationRefs(int GridIndex)
{
	for (FGrid& Grid : NavigationGrids)
	{
		if (Grid.GridIndex == GridIndex) continue;

		Grid.RemoveGridFromEdgeNavigation(GridIndex);
	}
}

void UUINavWidget::AppendNavigationGrid1D(EGridType GridType, int Dimension, FButtonNavigation EdgeNavigation, bool bWrap)
{
	if (NumberOfButtonsInGrids + Dimension > UINavButtons.Num())
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

	Add1DGrid(GridType,
			  UINavButtons.Num() > 0 ? (UINavButtons.Num() > NumberOfButtonsInGrids ? UINavButtons[NumberOfButtonsInGrids] : nullptr) : nullptr,
			  NavigationGrids.Num(),
			  Dimension,
			  EdgeNavigation,
			  bWrap);

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

	if (NumberOfButtonsInGrids + (ButtonsInGrid == -1 ? (DimensionX * DimensionY) : ButtonsInGrid) > UINavButtons.Num() && ButtonsInGrid != 0)
	{
		DISPLAYERROR("Not enough UINavButtons to append this navigation grid!");
		return;
	}

	FButtonNavigation NewNav;

	if ((NumberOfButtonsInGrids >= UINavButtons.Num() || UINavButtons.Num() == 0) && ButtonsInGrid != 0)
	{
		DISPLAYERROR("Not enough UINavButtons to add specified navigation dimensions!");
		return;
	}

	if (ButtonsInGrid < -1 || ButtonsInGrid > (DimensionX * DimensionY))
	{
		DISPLAYERROR("Invalid ButtonsInGrid value!");
		return;
	}
	else if (ButtonsInGrid >= 0)
	{
		int DesiredY = (ButtonsInGrid / DimensionX) + 1;
		if (DimensionY > DesiredY)
		{
			DimensionY = DesiredY;
		}
	}

	FGrid NewGrid = FGrid(EGridType::Grid2D, 
						  ButtonsInGrid != 0 ? UINavButtons[NumberOfButtonsInGrids] : nullptr,
						  NavigationGrids.Num(), 
						  DimensionX, 
						  DimensionY, 
						  EdgeNavigation, 
						  bWrap,
						  ButtonsInGrid);

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

	UUINavCollection* Collection = UINavCollections[CollectionIndex];
	Collection->FirstGridIndex = NavigationGrids.Num();
	Collection->SetupNavigation(EdgeNavigations);
	if (Collection->LastButtonIndex < Collection->FirstButtonIndex)
	{
		Collection->LastButtonIndex = Collection->FirstButtonIndex;
	}

	CollectionIndex++;
}

void UUINavWidget::SetEdgeNavigation(int GridIndex, FButtonNavigation NewEdgeNavigation)
{
	if (GridIndex < 0 || GridIndex >= NavigationGrids.Num())
	{
		return;
	}
	NavigationGrids[GridIndex].SetEdgeNavigation(NewEdgeNavigation);
}

void UUINavWidget::SetEdgeNavigationByButton(int GridIndex, FButtonNavigation NewEdgeNavigation)
{
	if (GridIndex < 0 || GridIndex >= NavigationGrids.Num())
	{
		return;
	}
	NavigationGrids[GridIndex].SetEdgeNavigationByButton(NewEdgeNavigation);
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
	if (TheSelector == nullptr || UINavButtons.Num() == 0) return;
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
	UUserWidget* TargetFromWidget = this;
	UUserWidget* TargetToWidget = this;

	/* Widget Animations functions must be called in the widget directly,
	so we need to check whether this animation belongs to a UINavCollection */
	for (UUINavCollection* Collection : UINavCollections)
	{
		if (Collection->FirstButtonIndex <= From && Collection->LastButtonIndex >= From)
		{
			TargetFromWidget = Collection->GetCollectionByIndex(From);
		}
		if (Collection->FirstButtonIndex <= To && Collection->LastButtonIndex >= To)
		{
			TargetToWidget = Collection->GetCollectionByIndex(To);
		}
	}

	if (From != -1 && From != To &&
		UINavAnimations.Num() > From && From < UINavAnimations.Num() &&
		UINavAnimations[From] != nullptr)
	{
		if (TargetFromWidget->IsAnimationPlaying(UINavAnimations[From]))
		{
			TargetFromWidget->ReverseAnimation(UINavAnimations[From]);
		}
		else
		{
			TargetFromWidget->PlayAnimation(UINavAnimations[From], 0.0f, 1, EUMGSequencePlayMode::Reverse, AnimationPlaybackSpeed);
		}
	}

	if (UINavAnimations.Num() <= To || UINavAnimations[To] == nullptr) return;

	if (TargetToWidget->IsAnimationPlaying(UINavAnimations[To]))
	{
		TargetToWidget->ReverseAnimation(UINavAnimations[To]);
	}
	else
	{
		TargetToWidget->PlayAnimation(UINavAnimations[To], 0.0f, 1, EUMGSequencePlayMode::Forward, AnimationPlaybackSpeed);
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
		if (NewText == nullptr) return;
	}
	else
	{
		NewText = Cast<UTextBlock>(UINavButtons[Index]->GetChildAt(0));
		if (NewText == nullptr) return;
	}
	NewText->SetColorAndOpacity(Color);
}

void UUINavWidget::UpdateHoveredButtonStates(int Index, bool bHovered)
{
	UUINavButton* ToButton = UINavButtons[Index];

	//Update new button state
	SwitchButtonStyle(EButtonStyle::Hovered, Index);

	if (ButtonIndex == Index) return;

	//Update previous button state
	SwitchButtonStyle(EButtonStyle::Normal,
					  ButtonIndex);
}

void UUINavWidget::SwitchButtonStyle(EButtonStyle NewStyle, int Index, bool bRevertStyle)
{
	UUINavButton* TheButton = UINavButtons[Index];
	bool bWasForcePressed = TheButton->ForcedStyle == EButtonStyle::Pressed;

	if (bRevertStyle)
	{
		RevertButtonStyle(Index);
	}

	TheButton->CurrentStyle = GetStyleFromButtonState(TheButton);
	SwapStyle(TheButton, NewStyle, TheButton->CurrentStyle);

	if (NewStyle == EButtonStyle::Pressed && TheButton->CurrentStyle != EButtonStyle::Pressed)
	{
		SwapPadding(TheButton);
	}
	else if (bWasForcePressed)
	{
		SwapPadding(TheButton);
	}

	if (NewStyle != TheButton->CurrentStyle) TheButton->ForcedStyle = NewStyle;
	
	if (NewStyle == EButtonStyle::Hovered && Index != ButtonIndex)
	{
		USoundBase* HoverSound = Cast<USoundBase>(TheButton->WidgetStyle.HoveredSlateSound.GetResourceObject());
		if (HoverSound != nullptr)
		{
			PlaySound(HoverSound);
		}
	}
}

void UUINavWidget::RevertButtonStyle(int Index)
{
	UUINavButton* TheButton = UINavButtons[Index];
	if (TheButton->ForcedStyle == EButtonStyle::None) return;

	SwapStyle(TheButton, TheButton->ForcedStyle, TheButton->CurrentStyle);

	TheButton->ForcedStyle = EButtonStyle::None;
}

void UUINavWidget::SwapStyle(UUINavButton* TargetButton, EButtonStyle Style1, EButtonStyle Style2)
{
	FButtonStyle Style = TargetButton->WidgetStyle;
	FSlateBrush TempState;

	switch (Style1)
	{
		case EButtonStyle::Normal:
			TempState = Style.Normal;
			switch (Style2)
			{
				case EButtonStyle::Hovered:
					Style.Normal = Style.Hovered;
					Style.Hovered = TempState;
					break;
				case EButtonStyle::Pressed:
					Style.Normal = Style.Pressed;
					Style.Pressed = TempState;
					break;
			}
			break;
		case EButtonStyle::Hovered:
			TempState = Style.Hovered;
			switch (Style2)
			{
				case EButtonStyle::Normal:
					Style.Hovered = Style.Normal;
					Style.Normal = TempState;
					break;
				case EButtonStyle::Pressed:
					Style.Hovered = Style.Pressed;
					Style.Pressed = TempState;
					break;
			}
			break;
		case EButtonStyle::Pressed:
			TempState = Style.Pressed;
			switch (Style2)
			{
				case EButtonStyle::Normal:
					Style.Pressed = Style.Normal;
					Style.Normal = TempState;
					break;
				case EButtonStyle::Hovered:
					Style.Pressed = Style.Hovered;
					Style.Hovered = TempState;
					break;
			}
			break;
	}

	TargetButton->SetStyle(Style);
}

void UUINavWidget::SwapPadding(UUINavButton* TargetButton)
{
	FButtonStyle Style = TargetButton->WidgetStyle;
	UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(TargetButton->Slot);
	FMargin PressedPadding = Style.PressedPadding - Style.NormalPadding;
	if (OverlaySlot != nullptr)
	{
		OverlaySlot->SetPadding(OverlaySlot->Padding == PressedPadding ? FMargin(0.0f) : PressedPadding);
	}
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

void UUINavWidget::OnNavigate_Implementation(int From, int To)
{

}

void UUINavWidget::OnNavigatedDirection_Implementation(ENavigationDirection Direction)
{

}

void UUINavWidget::OnSelect_Implementation(int Index)
{

}

void UUINavWidget::OnStartSelect_Implementation(int Index)
{

}

void UUINavWidget::OnStopSelect_Implementation(int Index)
{

}

void UUINavWidget::NavigateTo(int Index, bool bHoverEvent)
{
	if (Index >= UINavButtons.Num() || Index == ButtonIndex) return;

	DispatchNavigation(Index, bHoverEvent);
	OnNavigate(ButtonIndex, Index);
	CollectionNavigateTo(Index);

	if (!bHoverEvent)
	{
		bIgnoreMouseEvent = true;
		CurrentButton->OnUnhovered.Broadcast();
	}

	ButtonIndex = Index;
	CurrentButton = UINavButtons[ButtonIndex];

	if (!bHoverEvent)
	{
		bIgnoreMouseEvent = true;
		CurrentButton->OnHovered.Broadcast();
	}
}

void UUINavWidget::CollectionNavigateTo(int Index)
{
	bool bFoundFrom = false;
	bool bFoundTo = false;
	for (UUINavCollection* Collection : UINavCollections)
	{
		int CollectionFromIndex = Index != ButtonIndex ? GetCollectionButtonIndex(Collection, ButtonIndex) : -1;
		int CollectionToIndex = GetCollectionButtonIndex(Collection, Index);

		bool bValidFrom = CollectionFromIndex != -1;
		bool bValidTo = CollectionToIndex != -1;
		if (bValidFrom || bValidTo)
		{
			if (!bFoundFrom) bFoundFrom = bValidFrom;
			if (!bFoundTo) bFoundTo = bValidTo;

			Collection->NotifyOnNavigate(Index != ButtonIndex ? ButtonIndex : -1, Index, CollectionFromIndex, CollectionToIndex);
		}

		if (bFoundFrom && bFoundTo) break;
	}
}

void UUINavWidget::CallCustomInput(FName ActionName, uint8* Buffer)
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

void UUINavWidget::DispatchNavigation(int Index, bool bHoverEvent)
{
	//Update all the possible scroll boxes in the widget
	for (UScrollBox* ScrollBox : ScrollBoxes)
	{
		ScrollBox->ScrollWidgetIntoView(UINavButtons[Index], bAnimateScrollBoxes);
	}

	if (bUseButtonStates) UpdateHoveredButtonStates(Index, bHoverEvent);

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
}

void UUINavWidget::CollectionOnSelect(int Index)
{
	for (UUINavCollection* Collection : UINavCollections)
	{
		int CollectionButtonIndex = GetCollectionButtonIndex(Collection, Index);
		if (CollectionButtonIndex != -1)
		{
			Collection->NotifyOnSelect(Index, CollectionButtonIndex);
			break;
		}
	}
}

void UUINavWidget::CollectionOnStartSelect(int Index)
{
	for (UUINavCollection* Collection : UINavCollections)
	{
		int CollectionButtonIndex = GetCollectionButtonIndex(Collection, Index);
		if (CollectionButtonIndex != -1)
		{
			Collection->NotifyOnStartSelect(Index, CollectionButtonIndex);
			break;
		}
	}
}

void UUINavWidget::CollectionOnStopSelect(int Index)
{
	for (UUINavCollection* Collection : UINavCollections)
	{
		int CollectionButtonIndex = GetCollectionButtonIndex(Collection, Index);
		if (CollectionButtonIndex != -1)
		{
			Collection->NotifyOnStopSelect(Index, CollectionButtonIndex);
			break;
		}
	}
}

void UUINavWidget::OnPreSelect(int Index, bool bMouseClick)
{
	if (CurrentButton == nullptr ||
		SelectedButtonIndex == -1) return;

	bool bIsSelectedButton = SelectedButtonIndex == Index && (!bMouseClick || UINavButtons[Index]->IsHovered());

	if (SelectCount == 1)
	{
		if (!bMouseClick)
		{
			bIgnoreMouseEvent = true;
			CurrentButton->OnReleased.Broadcast();
			if (bIsSelectedButton) CurrentButton->OnClicked.Broadcast();
		}

		UUINavComponent* CurrentUINavComp = GetUINavComponentAtIndex(Index);
		if (CurrentUINavComp != nullptr)
		{
			if (bIsSelectedButton)
			{
				CurrentUINavComp->OnSelected();
			}
			CurrentUINavComp->OnStopSelected();
		}
	}

	if (UINavInputContainer != nullptr && Index >= UINavInputContainer->FirstButtonIndex && Index <= UINavInputContainer->LastButtonIndex)
	{
		InputBoxIndex = Index - UINavInputContainer->FirstButtonIndex;
		int KeysPerInput = UINavInputContainer->KeysPerInput;
		UINavInputBoxes[InputBoxIndex / KeysPerInput]->NotifySelected(InputBoxIndex % KeysPerInput);
		ReceiveInputType = UINavInputBoxes[InputBoxIndex / KeysPerInput]->IsAxis() ? EReceiveInputType::Axis : EReceiveInputType::Action;
		APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
		SetUserFocus(PC);
		SetKeyboardFocus();
		bWaitForInput = true;
	}
	else
	{
		SwitchButtonStyle(UINavButtons[Index]->IsPressed() || SelectCount > 1 ? EButtonStyle::Pressed : (Index == ButtonIndex ? EButtonStyle::Hovered : EButtonStyle::Normal),
						  ButtonIndex);

		SelectCount--;
		if (SelectCount == 0)
		{
			SelectedButtonIndex = -1;

			if (bIsSelectedButton)
			{
				OnSelect(Index);
				CollectionOnSelect(Index);
			}
			OnStopSelect(Index);
			CollectionOnStopSelect(Index);
		}
	}
}

void UUINavWidget::OnReturn_Implementation()
{
	ReturnToParent();
}

void UUINavWidget::OnNext_Implementation()
{

}

void UUINavWidget::OnPrevious_Implementation()
{

}

void UUINavWidget::OnInputChanged_Implementation(EInputType From, EInputType To)
{

}

void UUINavWidget::PreSetup_Implementation(bool bFirstSetup)
{

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

void UUINavWidget::OnHorizCompUpdated_Implementation(int Index)
{

}

UWidget* UUINavWidget::GoToWidget(TSubclassOf<UUINavWidget> NewWidgetClass, bool bRemoveParent, bool bDestroyParent, int ZOrder)
{
	if (NewWidgetClass == nullptr)
	{
		DISPLAYERROR("GoToWidget: No Widget Class found");
		return nullptr;
	}

	APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
	UUINavWidget* NewWidget = CreateWidget<UUINavWidget>(PC, NewWidgetClass);
	return GoToBuiltWidget(NewWidget, bRemoveParent, bDestroyParent, ZOrder);
}

UWidget * UUINavWidget::GoToBuiltWidget(UUINavWidget* NewWidget, bool bRemoveParent, bool bDestroyParent, int ZOrder)
{
	if (NewWidget == nullptr) return nullptr;
	APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
	NewWidget->ParentWidget = this;
	NewWidget->bParentRemoved = bRemoveParent;
	NewWidget->bShouldDestroyParent = bDestroyParent;
	NewWidget->WidgetComp = WidgetComp;
	if (WidgetComp != nullptr)
	{
		WidgetComp->SetWidget(NewWidget);
	}
	else
	{
		NewWidget->AddToViewport(ZOrder);
		NewWidget->SetUserFocus(PC);
		if (UINavPC->GetInputMode() == EInputMode::UI)
		{
			NewWidget->SetKeyboardFocus();
		}
	}
	CleanSetup();
	return NewWidget;
}

void UUINavWidget::ReturnToParent(bool bRemoveAllParents, int ZOrder)
{
	if (ParentWidget == nullptr)
	{
		if (bAllowRemoveIfRoot)
		{
			IUINavPCReceiver::Execute_OnRootWidgetRemoved(UINavPC->GetOwner());
			UINavPC->SetActiveWidget(nullptr);

			if (WidgetComp != nullptr) WidgetComp->SetWidget(nullptr);
			else RemoveFromParent();
		}
		return;
	}

	if (WidgetComp != nullptr)
	{
		if (bRemoveAllParents)
		{
			WidgetComp->SetWidget(nullptr);
		}
		else
		{
			if (bParentRemoved)
			{
				ParentWidget->ReturnedFromWidget = this;
			}
			else
			{
				UINavPC->SetActiveWidget(ParentWidget);
				ParentWidget->ReconfigureSetup();
			}
			WidgetComp->SetWidget(ParentWidget);

		}
	}
	else
	{
		if (bRemoveAllParents)
		{
			IUINavPCReceiver::Execute_OnRootWidgetRemoved(UINavPC->GetOwner());
			UINavPC->SetActiveWidget(nullptr);
			ParentWidget->RemoveAllParents();
			RemoveFromParent();
			Destruct();
		}
		else
		{
			//If parent was removed, add it to viewport
			if (bParentRemoved)
			{
				ParentWidget->ReturnedFromWidget = this;
				ParentWidget->AddToViewport(ZOrder);
			}
			else
			{
				UINavPC->SetActiveWidget(ParentWidget);
				ParentWidget->ReconfigureSetup();
			}
			RemoveFromParent();
		}
	}
}

void UUINavWidget::RemoveAllParents()
{
	if (ParentWidget != nullptr)
	{
		ParentWidget->RemoveAllParents();
	}
	RemoveFromParent();
	Destruct();
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

UUINavButton * UUINavWidget::GetButtonAtIndex(int InButtonIndex)
{
	if (InButtonIndex < 0 || InButtonIndex >= UINavButtons.Num())
	{
		return nullptr;
	}
	else return UINavButtons[InButtonIndex];
}

EButtonStyle UUINavWidget::GetStyleFromButtonState(UButton* Button)
{
	if (Button->IsPressed()) return EButtonStyle::Pressed;
	else if (Button->IsHovered()) return EButtonStyle::Hovered;
	else return EButtonStyle::Normal;
}

void UUINavWidget::GetGridAtIndex(int GridIndex, FGrid & Grid, bool & IsValid)
{
	if (GridIndex < 0 || GridIndex >= NavigationGrids.Num())
	{
		IsValid = false;
	}
	else
	{
		IsValid = true;
		Grid = NavigationGrids[GridIndex];
	}
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

void UUINavWidget::GetButtonGridFromIndex(int InButtonIndex, FGrid & ButtonGrid, bool & IsValid)
{
	if (InButtonIndex < 0 || InButtonIndex >= UINavButtons.Num())
	{
		IsValid = false;
	}
	else
	{
		GetButtonGrid(UINavButtons[InButtonIndex], ButtonGrid, IsValid);
	}
}

int UUINavWidget::GetIndexInGridFromButtonIndex(int InButtonIndex)
{
	if (InButtonIndex < 0 || InButtonIndex >= UINavButtons.Num())
	{
		return -1;
	}
	else
	{
		return UINavButtons[InButtonIndex]->IndexInGrid;
	}
}

int UUINavWidget::GetGridIndexFromButtonIndex(int InButtonIndex)
{
	if (InButtonIndex < 0 || InButtonIndex >= UINavButtons.Num())
	{
		return -1;
	}
	else
	{
		return UINavButtons[InButtonIndex]->GridIndex;
	}
}

int UUINavWidget::GetGridStartingIndex(int GridIndex)
{
	if (GridIndex < 0 || GridIndex >= NavigationGrids.Num()) return -1;

	if (NavigationGrids[GridIndex].FirstButton != nullptr)
	{
		if (NavigationGrids[GridIndex].FirstButton->ButtonIndex < 0)
		{
			if (GridIndex > 0) return (GetGridStartingIndex(GridIndex - 1) + 1);
			else return 0;
		}
		else
		{
			return NavigationGrids[GridIndex].FirstButton->ButtonIndex;
		}
	}
	else
	{
		for (int i = GridIndex - 1; i >= 0; i--)
		{
			if (NavigationGrids[i].FirstButton != nullptr)
			{
				return (NavigationGrids[i].GetLastButtonIndex() + 1);
			}
		}
	}
	return -1;
}

UUINavButton * UUINavWidget::GetLastButtonInGrid(const FGrid Grid)
{
	int LastIndex = Grid.GetLastButtonIndex();

	return UINavButtons.Num() > LastIndex ? UINavButtons[LastIndex] : nullptr;
}

UUINavButton * UUINavWidget::GetButtonAtGridIndex(const FGrid ButtonGrid, const int IndexInGrid)
{
	if (ButtonGrid.FirstButton == nullptr || IndexInGrid < 0) return nullptr;
	int NewIndex = ButtonGrid.FirstButton->ButtonIndex + IndexInGrid;

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

int UUINavWidget::GetCollectionButtonIndex(UUINavCollection * Collection, int Index)
{
	if (Collection == nullptr || Index == -1) return -1;

	if (Collection->FirstButtonIndex <= Index &&
		Collection->LastButtonIndex >= Index &&
		Collection->FirstButtonIndex < Collection->LastButtonIndex)
	{
		return Index - Collection->FirstButtonIndex;
	}

	return -1;
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
	if (bIgnoreMouseEvent)
	{
		bIgnoreMouseEvent = false;
		return;
	}

	if (bWaitForInput)
	{
		CancelRebind();
	}

	if (!UINavPC->AllowsDirectionalInput())
	{
		return;
	}

	if (UINavPC->GetCurrentInputType() != EInputType::Mouse || Index == ButtonIndex)
	{
		if (bUseButtonStates) RevertButtonStyle(Index);
		return;
	}

	UINavPC->ClearTimer();
	NavigateTo(Index, true);
}

void UUINavWidget::UnhoverEvent(int Index)
{
	if (bIgnoreMouseEvent)
	{
		bIgnoreMouseEvent = false;
		return;
	}

	if (bWaitForInput)
	{
		CancelRebind();
	}

	if (bUseButtonStates)
	{
		UUINavButton* ToButton = UINavButtons[Index];
		if (SelectedButtonIndex != ButtonIndex)
		{
			SwitchButtonStyle(ButtonIndex == Index ? EButtonStyle::Hovered : EButtonStyle::Normal, Index);
		}

		ButtonIndex = (ToButton->ForcedStyle != EButtonStyle::None) ? Index : ButtonIndex;
	}
}

void UUINavWidget::PressEvent(int Index)
{
	if (bWaitForInput)
	{
		if (ReceiveInputType == EReceiveInputType::Axis) CancelRebind();
		else ProcessKeybind(EKeys::LeftMouseButton);
	}
	else
	{
		if (bIgnoreMouseEvent)
		{
			bIgnoreMouseEvent = false;
			return;
		}
		else
		{
			UINavPC->NotifyMouseInputType();
		}

		if (!UINavPC->AllowsSelectInput()) return;

		FinishPress(true);
	}
}

void UUINavWidget::ReleaseEvent(int Index)
{
	if (bIgnoreMouseEvent)
	{
		bIgnoreMouseEvent = false;
		return;
	}
	else
	{
		UINavPC->NotifyMouseInputType();
	}

	if (bMovingSelector)
	{
		HaltedIndex = SELECT_INDEX;
		return;
	}

	if (!UINavButtons[Index]->IsHovered()) RevertButtonStyle(Index);

	if (!UINavPC->AllowsSelectInput()) return;

	OnPreSelect(Index, true);

	if (Index != ButtonIndex) NavigateTo(Index);
}

void UUINavWidget::FinishPress(bool bMouse)
{
	SelectedButtonIndex = ButtonIndex;
	SelectCount++;

  	if (!bMouse)
	{
		if (!CurrentButton->IsPressed())
		{
 			SwitchButtonStyle(EButtonStyle::Pressed,
							  ButtonIndex);
		}
	}
	else if (CurrentButton->ForcedStyle != EButtonStyle::Normal)
	{
		SwitchButtonStyle(EButtonStyle::Pressed,
						ButtonIndex);
	}

	UUINavComponent* CurrentUINavComp = GetUINavComponentAtIndex(ButtonIndex);
	if (CurrentUINavComp != nullptr)
	{
		CurrentUINavComp->OnStartSelected();
	}

	OnStartSelect(ButtonIndex);
	CollectionOnStartSelect(ButtonIndex);
}

void UUINavWidget::SetupUINavButtonDelegates(UUINavButton * NewButton)
{
	if (!NewButton->CustomHover.IsBound())
		NewButton->CustomHover.AddDynamic(this, &UUINavWidget::HoverEvent);
	if (!NewButton->CustomUnhover.IsBound())
		NewButton->CustomUnhover.AddDynamic(this, &UUINavWidget::UnhoverEvent);
	FScriptDelegate OnClickScriptDelegate;
	OnClickScriptDelegate.BindUFunction(NewButton, FName("OnClick"));
	if (NewButton->OnPressed.Contains(OnClickScriptDelegate))
	{
		NewButton->OnPressed.Remove(OnClickScriptDelegate);
		NewButton->OnPressed.AddDynamic(NewButton, &UUINavButton::OnPress);
	}
	if (!NewButton->CustomPress.IsBound())
		NewButton->CustomPress.AddDynamic(this, &UUINavWidget::PressEvent);
	if (!NewButton->CustomRelease.IsBound())
		NewButton->CustomRelease.AddDynamic(this, &UUINavWidget::ReleaseEvent);
}

void UUINavWidget::ProcessKeybind(FKey PressedKey)
{
	int KeysPerInput = UINavInputContainer->KeysPerInput;
	UINavInputBoxes[InputBoxIndex / KeysPerInput]->UpdateInputKey(PressedKey, InputBoxIndex % KeysPerInput);
	bWaitForInput = false;
	ReceiveInputType = EReceiveInputType::None;
}

void UUINavWidget::CancelRebind()
{
	bWaitForInput = false;
	int KeysPerInput = UINavInputContainer->KeysPerInput;
	UINavInputBoxes[InputBoxIndex / KeysPerInput]->RevertToKeyText(InputBoxIndex % KeysPerInput);
	ReceiveInputType = EReceiveInputType::None;
}

void UUINavWidget::NavigateInDirection(ENavigationDirection Direction)
{
	if (bWaitForInput)
	{
		CancelRebind();
		return;
	}

	if (Direction == ENavigationDirection::None || UINavButtons.Num() == 0) return;

	if (NumberOfButtonsInGrids == 0)
	{
		OnNavigatedDirection(Direction);
		return;
	}

	if (bMovingSelector)
	{
		UUINavButton* NextButton = FindNextButton(CurrentButton, Direction);
		HaltedIndex = NextButton != nullptr ? NextButton->ButtonIndex : -1;
		return;
	}

	OnNavigatedDirection(Direction);

	UUINavHorizontalComponent* HorizComp = GetUINavHorizontalCompAtIndex(ButtonIndex);
	if (HorizComp != nullptr && 
		(Direction == ENavigationDirection::Left || 
		 Direction == ENavigationDirection::Right))
	{
		if (Direction == ENavigationDirection::Left)
		{
			HorizComp->NavigateLeft();
		}
		else
		{
			HorizComp->NavigateRight();
		}
	}
	else MenuNavigate(Direction);
}

void UUINavWidget::MenuSelect()
{
	MenuSelectPress();
	MenuSelectRelease();
}

void UUINavWidget::MenuReturn()
{
	MenuReturnPress();
	MenuReturnRelease();
}

void UUINavWidget::MenuSelectPress()
{
	if (bWaitForInput)
	{
		CancelRebind();
		return;
	}

	if (CurrentButton != nullptr)
	{
		if (SelectCount == 0)
		{
			USoundBase* PressSound = Cast<USoundBase>(CurrentButton->WidgetStyle.PressedSlateSound.GetResourceObject());
			if (PressSound != nullptr) PlaySound(PressSound);
			bIgnoreMouseEvent = true;
			CurrentButton->OnPressed.Broadcast();
		}

		FinishPress(false);
	}
}

void UUINavWidget::MenuSelectRelease()
{
	if (bMovingSelector)
	{
		HaltedIndex = SELECT_INDEX;
		return;
	}
	OnPreSelect(ButtonIndex);
}

void UUINavWidget::MenuReturnPress()
{
	bReturning = true;
}

void UUINavWidget::MenuReturnRelease()
{
	if (!bReturning) return;

	if (bWaitForInput)
	{
		CancelRebind();
		return;
	}

	if (bMovingSelector)
	{
		HaltedIndex = RETURN_INDEX;
		return;
	}

	bReturning = false;
	OnReturn();
}
