// Copyright (C) 2018 Gon�alo Marques - All Rights Reserved

#include "UINavWidget.h"
#include "UINavButton.h"
#include "UINavComponentBox.h"
#include "UINavComponent.h"
#include "UINavInputBox.h"
#include "UINavInputContainer.h"
#include "UINavController.h"
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
	if (CurrentPC == nullptr)
	{
		CurrentPC = Cast<AUINavController>(GetOwningPlayer());
		if (CurrentPC == nullptr)
		{
			DISPLAYERROR("PlayerController isn't a UINavController");
			return;
		}
	}

	FetchButtonsInHierarchy();
	ReadyForSetup();

	if (UINavAnimations.Num() > 0 && UINavAnimations.Num() != UINavButtons.Num())
	{
		DISPLAYERROR("Number of animations doesn't match number of UINavButtons.");
		return;
	}

	if (bUseTextColor) ChangeTextColorToDefault();

	//If this widget doesn't need to create the selector, skip to setup
	if (!bUseSelector)
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
	if (!bUseSelector)
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

	if (FirstButtonIndex >= UINavButtons.Num())
	{
		DISPLAYERROR("Invalid FirstButton index, can't be greater than number of buttons.");
		return;
	}
	if (FirstButtonIndex < 0)
	{
		DISPLAYERROR("Invalid FirstButton index, can't be less than 0.");
		return;
	}

	ButtonIndex = FirstButtonIndex;
	CurrentButton = UINavButtons[FirstButtonIndex];

	while (CurrentButton->Visibility == ESlateVisibility::Collapsed ||
		   CurrentButton->Visibility == ESlateVisibility::Hidden ||
		   !CurrentButton->bIsEnabled)
	{
		ButtonIndex++;
		if (ButtonIndex >= UINavButtons.Num()) ButtonIndex = 0;
		
		CurrentButton = UINavButtons[ButtonIndex];
		if (ButtonIndex == FirstButtonIndex) break;
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

		UUINavInputContainer* InputContainer = Cast<UUINavInputContainer>(widget);
		if (InputContainer != nullptr)
		{
			if (UINavInputContainer != nullptr)
			{
				DISPLAYERROR("Found more than 1 UINavInputContainer!");
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
				if (UIComp->ComponentIndex != -1) NewNavButton->ButtonIndex = UIComp->ComponentIndex;

				UINavComponentsIndices.Add(UINavButtons.Num());
				UINavComponents.Add(UIComp);

				UUINavComponentBox* UICompBox = Cast<UUINavComponentBox>(widget);
				if (UICompBox != nullptr)
				{
					ComponentBoxIndices.Add(UINavButtons.Num());
					UINavComponentBoxes.Add(UICompBox);
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
	if (TheSelector == nullptr)
	{
		DISPLAYERROR("Couldn't find TheSelector");
		return;
	}

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

	if (CurrentPC != nullptr) CurrentPC->SetActiveWidget(this);

	if (bUseSelector)
	{
		if (TheSelector == nullptr) return;
		TheSelector->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	bCompletedSetup = true;

	NavigateTo(ButtonIndex);
	OnNavigate(-1, ButtonIndex);
	if (UINavAnimations.Num() > 0) ExecuteAnimations(-1, ButtonIndex);

	OnSetupCompleted();
}

void UUINavWidget::ReadyForSetup_Implementation()
{

}

void UUINavWidget::NativeTick(const FGeometry & MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	if (!bUseSelector || !bSetupStarted) return;

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

		if ((UINavInputContainer->bCanCancelKeybind && CurrentPC->IsReturnKey(PressedKey)))
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
		if (CurrentPC->GetCurrentInputType() != EInputType::Mouse)
		{
			CurrentPC->NotifyMouseInputType();
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
	FString ActionName = CurrentPC->FindActionByKey(PressedKey);
	if (ActionName.Equals(TEXT(""))) return FReply::Handled();

	return CurrentPC->OnActionPressed(ActionName);
}

FReply UUINavWidget::OnKeyReleased(FKey PressedKey)
{
	FString ActionName = CurrentPC->FindActionByKey(PressedKey);
	if (ActionName.Equals(TEXT(""))) return FReply::Handled();

	return CurrentPC->OnActionReleased(ActionName);
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
		CurrentPC->bAllowNavigation = true;
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

void UUINavWidget::AddUINavButton(UUINavButton * NewButton, FGrid& TargetGrid, int IndexInGrid)
{
	NewButton->ButtonIndex = TargetGrid.FirstButton->ButtonIndex + IndexInGrid;
	SetupUINavButtonDelegates(NewButton);
	UINavButtons.Insert(NewButton, NewButton->ButtonIndex);

	int i;
	for (i = NewButton->ButtonIndex + 1; i < UINavButtons.Num(); i++) UINavButtons[i]->ButtonIndex = i;

	for (i = 0; i < UINavComponentsIndices.Num(); i++)
	{
		if (UINavComponentsIndices[i] >= NewButton->ButtonIndex)
		{
			UINavComponents[i]->ComponentIndex++;
			UINavComponentsIndices[i]++;
		}
	}

	//Take into account UINavComponent, Containers, etc.
	for (i = 0; i < NavigationGrids.Num(); i++)
	{

	}

	if (UINavInputContainer != nullptr && NewButton->ButtonIndex <= UINavInputContainer->FirstButtonIndex)
	{
		for (i = 0; i < UINavInputBoxes.Num(); i++)
		{

		}
	}

	//Button can be first and last in a grid!!
	if (IndexInGrid >= TargetGrid.DimensionX || IndexInGrid <= -1) IndexInGrid = -1;

	//TODO: Check if dimension greater than 1
	switch (TargetGrid.GridType)
	{
		case EGridType::Horizontal:

			NewButton->ButtonNav.UpButton = TargetGrid.EdgeNavigation.UpButton;
			NewButton->ButtonNav.DownButton = TargetGrid.EdgeNavigation.DownButton;

			if (IndexInGrid == 0)
			{
				UUINavButton* RightButton = UINavButtons[NewButton->ButtonIndex + 1];
				NewButton->ButtonNav.LeftButton = TargetGrid.EdgeNavigation.LeftButton;
				RightButton->ButtonNav.LeftButton = NewButton;
			}
			else
			{
				UUINavButton* LeftButton = UINavButtons[NewButton->ButtonIndex - 1];
				NewButton->ButtonNav.LeftButton = LeftButton;
				LeftButton->ButtonNav.RightButton = NewButton;
			}

			if (IndexInGrid == -1)
			{
				UUINavButton* LeftButton = UINavButtons[NewButton->ButtonIndex - 1];
				NewButton->ButtonNav.RightButton = TargetGrid.EdgeNavigation.RightButton;
				LeftButton->ButtonNav.RightButton = NewButton;
			}
			else
			{
				UUINavButton* RightButton = UINavButtons[NewButton->ButtonIndex + 1];
				NewButton->ButtonNav.RightButton = RightButton;
				RightButton->ButtonNav.LeftButton = NewButton;
			}

			TargetGrid.DimensionX++;

			break;
		case EGridType::Vertical:

			NewButton->ButtonNav.LeftButton = TargetGrid.EdgeNavigation.LeftButton;
			NewButton->ButtonNav.RightButton = TargetGrid.EdgeNavigation.RightButton;

			if (IndexInGrid == 0)
			{
				UUINavButton* DownButton = UINavButtons[NewButton->ButtonIndex + 1];
				NewButton->ButtonNav.UpButton = TargetGrid.EdgeNavigation.UpButton;
				DownButton->ButtonNav.UpButton = NewButton;
			}
			else
			{
				UUINavButton* UpButton = UINavButtons[NewButton->ButtonIndex - 1];
				NewButton->ButtonNav.UpButton = UpButton;
				UpButton->ButtonNav.DownButton = NewButton;
			}

			if (IndexInGrid == -1)
			{
				UUINavButton* UpButton = UINavButtons[NewButton->ButtonIndex - 1];
				NewButton->ButtonNav.DownButton = TargetGrid.EdgeNavigation.RightButton;
				UpButton->ButtonNav.DownButton = NewButton;
			}
			else
			{
				UUINavButton* DownButton = UINavButtons[NewButton->ButtonIndex + 1];
				NewButton->ButtonNav.DownButton = DownButton;
				DownButton->ButtonNav.UpButton = NewButton;
			}

			TargetGrid.DimensionY++;
			break;
		case EGridType::Grid2D:

			//TODO: ???
			TargetGrid.DimensionX++;
			TargetGrid.DimensionY++;
			break;
	}

}

void UUINavWidget::AddUINavComponent(UUINavComponent * NewButton, FGrid& TargetGrid, int IndexInGrid)
{

}

void UUINavWidget::AppendNavigationGrid1D(EGridType GridType, int Dimension, FButtonNavigation EdgeNavigation, bool bWrap)
{
	if (Dimension < 0) Dimension = UINavButtons.Num();
	if (GridType == EGridType::Grid2D)
	{
		DISPLAYERROR("Append Navigation Grid 1D Type shouldn't be 2D");
		return;
	}

	FButtonNavigation NewNav;
	int StartingIndex = NavigationGrids.Num() > 0 ? NavigationGrids.Last().FirstButton->ButtonIndex + NavigationGrids.Last().GetDimension() : 0;
	int GridDimension = Dimension;
	FButtonNavigation GridEdge = EdgeNavigation;
	int ExtraButtons = 0;
	if (InputContainerIndex >= StartingIndex && InputContainerIndex <= StartingIndex + Dimension - 1)
	{
		ExtraButtons = (UINavInputBoxes.Num() * UINavInputContainer->KeysPerInput) - 1;
		GridDimension = InputContainerIndex - StartingIndex;
		GridEdge.DownButton = UINavButtons[UINavInputContainer->TopButtonIndex];
	}

	if (GridType == EGridType::Vertical && GridDimension > 0) NavigationGrids.Add(FGrid(EGridType::Vertical, UINavButtons[StartingIndex], 0, GridDimension, GridEdge));
	if (GridType == EGridType::Horizontal && GridDimension > 0) NavigationGrids.Add(FGrid(EGridType::Horizontal, UINavButtons[StartingIndex], GridDimension, 0, GridEdge));

	bool bReachedInputContainer = false;
	int ContainerUpIndex = -1;

	int GridIndex = NavigationGrids.Num() - 1;
	for (int i = 0; i < Dimension; i++)
	{
		if ((InputContainerIndex - StartingIndex) == i)
		{
			ETargetColumn TargetColumn = UINavInputContainer->GetTargetColumn();
			FButtonNavigation InputEdgeNav;
			bReachedInputContainer = true;

			if (GridType == EGridType::Vertical)
			{
				InputEdgeNav.LeftButton = EdgeNavigation.LeftButton;
				InputEdgeNav.RightButton = EdgeNavigation.RightButton;
				InputEdgeNav.UpButton = i == 0 ? (EdgeNavigation.UpButton != nullptr ? EdgeNavigation.UpButton : (bWrap ? UINavButtons[StartingIndex + Dimension + ExtraButtons - 1] : nullptr)) : UINavButtons[UINavInputContainer->FirstButtonIndex + i - 1];
				InputEdgeNav.DownButton = i == Dimension - 1 ? (EdgeNavigation.DownButton != nullptr ? EdgeNavigation.DownButton : (bWrap ? UINavButtons[StartingIndex] : nullptr)) : UINavButtons[UINavInputContainer->LastButtonIndex + i + 1];
			}
			else if (GridType == EGridType::Horizontal)
			{
				InputEdgeNav.UpButton = EdgeNavigation.UpButton;
				InputEdgeNav.DownButton = EdgeNavigation.DownButton;
				InputEdgeNav.LeftButton =  i == 0 ? (EdgeNavigation.UpButton != nullptr ? EdgeNavigation.UpButton : (bWrap ? UINavButtons[StartingIndex + Dimension + ExtraButtons - 1] : nullptr)) : UINavButtons[UINavInputContainer->FirstButtonIndex + i - 1];
				InputEdgeNav.RightButton = i == Dimension - 1 ? (EdgeNavigation.DownButton != nullptr ? EdgeNavigation.DownButton : (bWrap ? UINavButtons[StartingIndex] : nullptr)) : UINavButtons[UINavInputContainer->LastButtonIndex + i + 1];
			}

			AppendNavigationGrid2D(UINavInputContainer->KeysPerInput, UINavInputBoxes.Num(), InputEdgeNav, false);
			int Start = StartingIndex + i + UINavInputContainer->KeysPerInput * UINavInputBoxes.Num();
			GridEdge = EdgeNavigation;
			GridEdge.UpButton = UINavButtons[UINavInputContainer->BottomButtonIndex];
			GridDimension = Dimension - i - 1;
			if (GridType == EGridType::Vertical && GridDimension > 0) NavigationGrids.Add(FGrid(EGridType::Vertical, UINavButtons[Start], 0, Dimension - i - 1, GridEdge));
			else if (GridType == EGridType::Horizontal && GridDimension > 0) NavigationGrids.Add(FGrid(EGridType::Horizontal, UINavButtons[Start], Dimension - i - 1, 0, GridEdge));

			if (i != 0)
			{
				if (GridType == EGridType::Vertical) UINavButtons[UINavInputContainer->FirstButtonIndex + i - 1]->ButtonNav.DownButton = UINavButtons[UINavInputContainer->TopButtonIndex];
				else if (GridType == EGridType::Horizontal) UINavButtons[UINavInputContainer->FirstButtonIndex + i - 1]->ButtonNav.RightButton = UINavButtons[UINavInputContainer->TopButtonIndex];
			}
			ContainerUpIndex = UINavInputContainer->LastButtonIndex + i + 1;
			continue;
		}

		if (GridType == EGridType::Vertical)
		{
			if (ContainerUpIndex != -1)
			{
				NewNav.UpButton = UINavButtons[UINavInputContainer->BottomButtonIndex];
			}
			else
			{
				if (i == 0) NewNav.UpButton = EdgeNavigation.UpButton == nullptr ? (bWrap ? UINavButtons[StartingIndex + Dimension + ExtraButtons - 1] : NewNav.UpButton) : EdgeNavigation.UpButton;
				else NewNav.UpButton = UINavButtons[StartingIndex + i + (bReachedInputContainer ? ExtraButtons : 0) - 1];
			}

			if (i == Dimension - 1) NewNav.DownButton = EdgeNavigation.DownButton == nullptr ? (bWrap ? UINavButtons[StartingIndex] : NewNav.DownButton) : EdgeNavigation.DownButton;
			else NewNav.DownButton = UINavButtons[StartingIndex + i + (bReachedInputContainer ? ExtraButtons : 0) + 1];

			if (EdgeNavigation.LeftButton != nullptr)
			{
				NewNav.LeftButton = EdgeNavigation.LeftButton;
			}
			if (EdgeNavigation.RightButton != nullptr)
			{
				NewNav.RightButton = EdgeNavigation.RightButton;
			}
		}
		else if (GridType == EGridType::Horizontal)
		{
			if (ContainerUpIndex != -1)
			{
				NewNav.LeftButton = UINavButtons[UINavInputContainer->BottomButtonIndex];
			}
			else
			{
				if (i == 0) NewNav.LeftButton = EdgeNavigation.LeftButton == nullptr ? (bWrap ? UINavButtons[StartingIndex + Dimension + ExtraButtons - 1] : NewNav.LeftButton) : EdgeNavigation.LeftButton;
				else NewNav.LeftButton = UINavButtons[StartingIndex + i + (bReachedInputContainer ? ExtraButtons : 0) - 1];
			}

			if (i == Dimension - 1) NewNav.RightButton = EdgeNavigation.RightButton == nullptr ? (bWrap ? UINavButtons[StartingIndex] : NewNav.RightButton) : EdgeNavigation.RightButton;
			else NewNav.RightButton = UINavButtons[StartingIndex + i + (bReachedInputContainer ? ExtraButtons : 0) + 1];

			if (EdgeNavigation.UpButton != nullptr)
			{
				NewNav.UpButton = EdgeNavigation.UpButton;
			}
			if (EdgeNavigation.DownButton != nullptr)
			{
				NewNav.DownButton = EdgeNavigation.DownButton;
			}
		}

		UINavButtons[StartingIndex + i]->ButtonNav = NewNav;
		UINavButtons[StartingIndex + i]->GridIndex = GridIndex;
		UINavButtons[StartingIndex + i]->IndexInGrid = i;
	}
}

void UUINavWidget::AppendNavigationGrid2D(int DimensionX, int DimensionY, FButtonNavigation EdgeNavigation, bool bWrap)
{
	if (DimensionX <= 0 || DimensionY <= 0)
	{
		DISPLAYERROR("AppendNavigationGrid2D Dimension should be greater than 0");
		return;
	}

	FButtonNavigation NewNav;
	int StartingIndex = NavigationGrids.Num() > 0 ? NavigationGrids.Last().FirstButton->ButtonIndex + NavigationGrids.Last().GetDimension() : 0;

	NavigationGrids.Add(FGrid(EGridType::Grid2D, UINavButtons[StartingIndex], DimensionX, DimensionY, EdgeNavigation));

	int GridIndex = NavigationGrids.Num() - 1;
	int Iterations = DimensionX * DimensionY;
	for (int i = 0; i < Iterations; i++)
	{
		//Top edge
		if (i < DimensionX)
		{
			NewNav.UpButton = bWrap ? UINavButtons[i + DimensionX * (DimensionY - 1)] : EdgeNavigation.UpButton;
		}
		//Rest of the grid
		else
		{
			NewNav.UpButton = UINavButtons[StartingIndex + i - DimensionX];
		}

		//Bottom edge
		if ((i / DimensionX) == (DimensionY - 1))
		{
			NewNav.DownButton = bWrap ? UINavButtons[i % DimensionX] : EdgeNavigation.DownButton;
		}
		//Rest of the grid
		else
		{
			NewNav.DownButton = UINavButtons[StartingIndex + i + DimensionX];
		}

		//Left edge
		if (((i + DimensionX) % DimensionX) == 0)
		{
			NewNav.LeftButton = bWrap ? UINavButtons[i - 1 + DimensionX] : EdgeNavigation.LeftButton;
		}
		//Rest of the grid
		else
		{
			NewNav.LeftButton = UINavButtons[StartingIndex + i - 1];
		}

		//Right edge
		if (((i + 1 + DimensionX) % DimensionX) == 0)
		{
			NewNav.RightButton = bWrap ? UINavButtons[i + 1 - DimensionX] : EdgeNavigation.RightButton;
		}
		//Rest of the grid
		else
		{
			NewNav.RightButton = UINavButtons[StartingIndex + i + 1];
		}

		UINavButtons[StartingIndex + i]->ButtonNav = NewNav;
		UINavButtons[StartingIndex + i]->GridIndex = GridIndex;
		UINavButtons[StartingIndex + i]->IndexInGrid = i;
	}
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
	if (From != -1)
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
	int NewComponentIndex = UINavComponentsIndices.Find(Index);
	if (NewComponentIndex != INDEX_NONE)
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
	//Update new button state
	if (!(bHovered ^ bSwitchedStyle[Index]))
	{
		SwitchButtonStyle(Index);
		bSwitchedStyle[Index] = !bHovered;
	}

	if (ButtonIndex == Index) return;

	//Update previous button state
	if (!(bHovered ^ !bSwitchedStyle[Index]))
	{
		SwitchButtonStyle(ButtonIndex);
		bSwitchedStyle[ButtonIndex] = false;
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

void UUINavWidget::SetSelector(UUserWidget * NewSelector)
{
	if (NewSelector == nullptr) DISPLAYERROR("Received invalid Selector");
		
	TheSelector = NewSelector;
}

void UUINavWidget::SetSelectorScale(FVector2D NewScale)
{
	if (TheSelector == nullptr) return;
	TheSelector->SetRenderScale(NewScale);
}

void UUINavWidget::SetSelectorVisibility(bool bVisible)
{
	ESlateVisibility Vis = bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
	TheSelector->SetVisibility(Vis);
}

void UUINavWidget::NavigateTo(int Index, bool bHoverEvent)
{
	if (Index >= UINavButtons.Num()) return;

	bool bShouldNotify = Index != ButtonIndex;
	
	if (bUseButtonStates) UpdateButtonsStates(Index, bHoverEvent);

	if (bUseSelector)
	{
		if (MoveCurve != nullptr) BeginSelectorMovement(Index);
		else UpdateSelectorLocation(Index);
	}

	if (bUseTextColor) UpdateTextColor(Index);

	if (bShouldNotify)
	{
		OnNavigate(ButtonIndex, Index);

		UUINavComponent* FromComponent = GetUINavComponentAtIndex(ButtonIndex);
		UUINavComponent* ToComponent = GetUINavComponentAtIndex(Index);
		if (FromComponent != nullptr) FromComponent->OnNavigatedFrom();
		if (ToComponent != nullptr) ToComponent->OnNavigatedTo();

		if (UINavAnimations.Num() > 0) ExecuteAnimations(ButtonIndex, Index);
	}

	//Update all the possible scroll boxes in the widget
	for (int i = 0; i < ScrollBoxes.Num(); ++i)
	{
		ScrollBoxes[i]->ScrollWidgetIntoView(UINavButtons[Index]);
	}

	ButtonIndex = Index;
	CurrentButton = UINavButtons[ButtonIndex];
}

void UUINavWidget::BeginSelectorMovement(int Index)
{
	if (MoveCurve == nullptr) return;

	SelectorOrigin = GetButtonLocation(ButtonIndex);
	SelectorDestination = GetButtonLocation(Index);
	Distance = SelectorDestination - SelectorOrigin;

	float MinTime, MaxTime;
	MoveCurve->GetTimeRange(MinTime, MaxTime);
	MovementTime = MaxTime - MinTime;

	bMovingSelector = true;
	CurrentPC->bAllowNavigation = false;
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
		SetUserFocus(CurrentPC);
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

UWidget* UUINavWidget::GoToWidget(TSubclassOf<UUINavWidget> NewWidgetClass, bool bRemoveParent, int ZOrder)
{
	if (NewWidgetClass == nullptr)
	{
		DISPLAYERROR("GoToWidget: No Widget Class found");
		return nullptr;
	}

	UUINavWidget* NewWidget = CreateWidget<UUINavWidget>(CurrentPC, NewWidgetClass);
	NewWidget->ParentWidget = this;
	NewWidget->bParentRemoved = bRemoveParent;
	if (HasUserFocus(CurrentPC)) NewWidget->SetUserFocus(CurrentPC);
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
			CurrentPC->OnRootWidgetRemoved();
			RemoveFromParent();
		}
		return;
	}

	if (HasUserFocus(CurrentPC)) ParentWidget->SetUserFocus(CurrentPC);

	//If parent was removed, add it to viewport
	if (bParentRemoved)
	{
		ParentWidget->ReturnedFromWidget = this;
		ParentWidget->AddToViewport();
	}
	else
	{
		CurrentPC->SetActiveWidget(ParentWidget);
		ParentWidget->ReconfigureSetup();
	}
	RemoveFromParent();
}

void UUINavWidget::MenuNavigate(ENavigationDirection Direction)
{
	UUINavButton* NewButton = FindNextButton(Direction);
	if (NewButton == nullptr) return;
	NavigateTo(NewButton->ButtonIndex);
}

UUINavButton* UUINavWidget::FindNextButton(ENavigationDirection Direction)
{
	UUINavButton* NewButton = FetchButtonByDirection(Direction, UINavButtons[ButtonIndex]);
	if (NewButton == nullptr) return nullptr;

	//Check if the button is visible, if not, skip to next button
	while (NewButton->Visibility == ESlateVisibility::Collapsed ||
		   NewButton->Visibility == ESlateVisibility::Hidden ||
		   !NewButton->bIsEnabled)
	{
		NewButton = FetchButtonByDirection(Direction, NewButton);
		if (NewButton == nullptr || NewButton == UINavButtons[ButtonIndex]) return nullptr;
	}
	return NewButton;
}

UUINavButton* UUINavWidget::FetchButtonByDirection(ENavigationDirection Direction, UUINavButton* CurrentButton)
{
	UUINavButton* NextButton = nullptr;

	switch (Direction)
	{
		case ENavigationDirection::Up:
			NextButton = CurrentButton->ButtonNav.UpButton;
			break;
		case ENavigationDirection::Down:
			NextButton = CurrentButton->ButtonNav.DownButton;
			break;
		case ENavigationDirection::Left:
			NextButton = CurrentButton->ButtonNav.LeftButton;
			break;
		case ENavigationDirection::Right:
			NextButton = CurrentButton->ButtonNav.RightButton;
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

UUINavComponent * UUINavWidget::GetUINavComponentAtIndex(int Index)
{
	int ValidIndex = UINavComponentsIndices.Find(Index);
	if (ValidIndex == INDEX_NONE) return nullptr;
	
	return UINavComponents[ValidIndex];
}

UUINavComponentBox * UUINavWidget::GetUINavComponentBoxAtIndex(int Index)
{
	int ValidIndex = ComponentBoxIndices.Find(Index);
	if (ValidIndex == INDEX_NONE) return nullptr;
	
	return UINavComponentBoxes[ValidIndex];
}

void UUINavWidget::HoverEvent(int Index)
{
	if (bWaitForInput)
	{
		bWaitForInput = false;
		UINavInputBoxes[InputBoxIndex / UINavInputContainer->KeysPerInput]->RevertToActionText(InputBoxIndex % UINavInputContainer->KeysPerInput);
	}
	if (CurrentPC->GetCurrentInputType() != EInputType::Mouse)
	{
		if (bUseButtonStates) SwitchButtonStyle(Index);
		return;
	}

	if (!CurrentPC->bAllowNavigation)
	{
		HaltedIndex = Index;
		return;
	}

	CurrentPC->ClearTimer();
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
		if (!bSwitchedStyle[Index] || (bSwitchedStyle[Index] && ButtonIndex == Index))
		{
			SwitchButtonStyle(Index);
			bSwitchedStyle[Index] = ButtonIndex == Index;
		}

		ButtonIndex = bSwitchedStyle[Index] ? Index : ButtonIndex;
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
		CurrentPC->NotifyMouseInputType();

		if (!CurrentPC->bAllowNavigation) return;

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
	bSwitchedStyle.Add(false);
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

	if (!CurrentPC->bAllowNavigation)
	{
		UUINavButton* NextButton = FindNextButton(Direction);
		HaltedIndex = NextButton != nullptr ? NextButton->ButtonIndex : -1;
		return;
	}

	OnNavigatedDirection(Direction);

	UUINavComponentBox* ComponentBox = GetUINavComponentBoxAtIndex(ButtonIndex);
	if (ComponentBox != nullptr && (Direction == ENavigationDirection::Left || Direction == ENavigationDirection::Right))
	{
		if (Direction == ENavigationDirection::Left) ComponentBox->NavigateLeft();
		else if (Direction == ENavigationDirection::Right) ComponentBox->NavigateRight();
	}
	else MenuNavigate(Direction);
}

void UUINavWidget::MenuSelect()
{
	if (!CurrentPC->bAllowNavigation)
	{
		HaltedIndex = SELECT_INDEX;
		return;
	}
	CurrentPC->ClearTimer();
	OnPreSelect(ButtonIndex);
}

void UUINavWidget::MenuReturn()
{
	if (!CurrentPC->bAllowNavigation)
	{
		HaltedIndex = RETURN_INDEX;
		return;
	}
	CurrentPC->ClearTimer();
	OnReturn();
}
