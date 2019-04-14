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
	if (TheSelector == nullptr)
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
	if (TheSelector == nullptr || TheSelector->Visibility == ESlateVisibility::Collapsed)
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
			DISPLAYERROR("The plugin doesn't support nested UINavWidgets");
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

	if (TheSelector != nullptr) TheSelector->SetVisibility(ESlateVisibility::HitTestInvisible);

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

	if (TheSelector == nullptr || !bSetupStarted) return;

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

void UUINavWidget::AddUINavButton(UUINavButton * NewButton, FGrid& TargetGrid, int IndexInGrid, FName AnimationTarget)
{
	if (TargetGrid.GridIndex == -1)
	{
		DISPLAYERROR("Can't add a button to a grid with an invalid index. Make sure this function is being called at least during OnSetupCompleted event.");
		return;
	}
	if (IndexInGrid >= TargetGrid.DimensionX || IndexInGrid <= -1) IndexInGrid = TargetGrid.GetDimension();

	if (IndexInGrid == 0) TargetGrid.FirstButton = NewButton;

	if (TargetGrid.GridType == EGridType::Horizontal) TargetGrid.DimensionX++;
	else if (TargetGrid.GridType == EGridType::Vertical) TargetGrid.DimensionY++;
	else {
		TargetGrid.NumGrid2DButtons++;
		if (TargetGrid.GetDimension() == 0)
		{
			TargetGrid.DimensionX = 1;
			TargetGrid.DimensionY = 1;
		}
		else if (TargetGrid.NumGrid2DButtons > TargetGrid.GetDimension())
		{
			TargetGrid.DimensionY++;
		}
	}

	NewButton->ButtonIndex = TargetGrid.FirstButton->ButtonIndex + IndexInGrid;
	SetupUINavButtonDelegates(NewButton);
	NewButton->GridIndex = TargetGrid.GridIndex;
	NewButton->IndexInGrid = IndexInGrid;
	UINavButtons.Insert(NewButton, NewButton->ButtonIndex);

	if (!AnimationTarget.IsEqual(FName("None")))
	{
		UWidgetAnimation* anim = UINavAnimations[0];
		UWidgetAnimation* newAnim = DuplicateObject<UWidgetAnimation>(anim, anim->GetOuter(), AnimationTarget);
		UINavAnimations.Add(newAnim);
	}

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
}

void UUINavWidget::AddUINavComponent(UUINavComponent * NewButton, FGrid& TargetGrid, int IndexInGrid)
{

}

void UUINavWidget::AppendNavigationGrid1D(EGridType GridType, int Dimension, FButtonNavigation EdgeNavigation, bool bWrap)
{
	if (UINavButtons.Num() == 0)
	{
		DISPLAYERROR("Make sure to call the Append Navigation functions only during the ReadyForSetup event!");
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

	FButtonNavigation NewNav;

	if (NumberOfButtonsInGrids >= UINavButtons.Num() || UINavButtons.Num() == 0)
	{
		DISPLAYERROR("Not enough UINavButtons to add specified navigation dimensions!");
		return;
	}

	NavigationGrids.Add(FGrid(EGridType::Grid2D, UINavButtons[NumberOfButtonsInGrids], NavigationGrids.Num(), DimensionX, DimensionY, EdgeNavigation, bWrap, ButtonsInGrid));

	int GridIndex = NavigationGrids.Num() - 1;
	int Iterations = DimensionX * DimensionY;
	for (int i = 0; i < Iterations; i++)
	{
		UINavButtons[NumberOfButtonsInGrids + i]->GridIndex = GridIndex;
		UINavButtons[NumberOfButtonsInGrids + i]->IndexInGrid = i;
	}

	NumberOfButtonsInGrids = NumberOfButtonsInGrids + Iterations;
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

	if (TheSelector != nullptr)
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

void UUINavWidget::OnComponentBoxNavigateLeft_Implementation(int Index)
{
}

void UUINavWidget::OnComponentBoxNavigateRight_Implementation(int Index)
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
	UUINavButton* NewButton = FindNextButton(CurrentButton, Direction);
	if (NewButton == nullptr) return;
	NavigateTo(NewButton->ButtonIndex);
}

UUINavButton* UUINavWidget::FindNextButton(UUINavButton* Button, ENavigationDirection Direction)
{
	if (Button == nullptr) return nullptr;

	UUINavButton* NewButton = FetchButtonByDirection(Direction, Button);
	if (NewButton == nullptr) return nullptr;

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
						else if (ButtonGrid.bWrap) NextButton = UINavButtons[ButtonGrid.DimensionX - 1];
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
						else if (ButtonGrid.bWrap) NextButton = UINavButtons[ButtonGrid.DimensionY - 1];
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
						else if (ButtonGrid.bWrap) NextButton = UINavButtons[ButtonGrid.DimensionX * (ButtonGrid.DimensionY - 1) + Button->IndexInGrid];
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
						else if (ButtonGrid.bWrap) NextButton = UINavButtons[Button->ButtonIndex - 1 + ButtonGrid.DimensionX];
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

UUINavButton * UUINavWidget::GetLastButtonInGrid(const FGrid Grid)
{
	int LastIndex = Grid.GetLastButtonIndex();

	return UINavButtons.Num() > LastIndex ? UINavButtons[LastIndex] : nullptr;
}

UUINavButton * UUINavWidget::GetButtonAtGridIndex(const FGrid ButtonGrid, const int GridIndex)
{
	if (ButtonGrid.FirstButton == nullptr) return nullptr;
	int NewIndex = ButtonGrid.FirstButton->ButtonIndex + GridIndex;

	if (NewIndex >= UINavButtons.Num()) return nullptr;

	return UINavButtons[NewIndex];
}

bool UUINavWidget::IsButtonInGrid(const FGrid ButtonGrid, UUINavButton * Button)
{
	return IsButtonIndexInGrid(ButtonGrid, Button->ButtonIndex);
}

bool UUINavWidget::IsButtonIndexInGrid(const FGrid ButtonGrid, const int Index)
{
	return (GetIndexInGridFromButtonIndex(ButtonGrid, Index) != -1);
}

int UUINavWidget::GetIndexInGridFromButton(const FGrid ButtonGrid, UUINavButton * Button)
{
	return GetIndexInGridFromButtonIndex(ButtonGrid, Button->ButtonIndex);
}

int UUINavWidget::GetIndexInGridFromButtonIndex(const FGrid ButtonGrid, const int Index)
{
	if (ButtonGrid.FirstButton == nullptr) return -1;
	if (Index < ButtonGrid.FirstButton->ButtonIndex) return -1;
	else if (Index > ButtonGrid.GetLastButtonIndex()) return -1;
	return Index - ButtonGrid.FirstButton->ButtonIndex;
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

	if (!CurrentPC->bAllowNavigation)
	{
		UUINavButton* NextButton = FindNextButton(CurrentButton, Direction);
		HaltedIndex = NextButton != nullptr ? NextButton->ButtonIndex : -1;
		return;
	}

	OnNavigatedDirection(Direction);

	UUINavComponentBox* ComponentBox = GetUINavComponentBoxAtIndex(ButtonIndex);
	if (ComponentBox != nullptr && (Direction == ENavigationDirection::Left || Direction == ENavigationDirection::Right))
	{
		if (Direction == ENavigationDirection::Left)
		{
			ComponentBox->NavigateLeft();
			OnComponentBoxNavigateLeft(ButtonIndex);
		}
		else if (Direction == ENavigationDirection::Right)
		{
			ComponentBox->NavigateRight();
			OnComponentBoxNavigateRight(ButtonIndex);
		}
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
