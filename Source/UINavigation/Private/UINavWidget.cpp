// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavWidget.h"
#include "UINavButton.h"
#include "UINavOptionBox.h"
#include "UINavComponent.h"
#include "UINavController.h"
#include "Blueprint/UserWidget.h"
#include "WidgetTree.h"
#include "Image.h"
#include "Border.h"
#include "TextBlock.h"
#include "ScrollBox.h"
#include "CanvasPanelSlot.h"
#include "Blueprint/SlateBlueprintLibrary.h"

void UUINavWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;

	CurrentPC = Cast<AUINavController>(GetOwningPlayer());

	if (InputMode == EInputMode::UIOnly)
	{
		SetUserFocus(CurrentPC);
	}

	//check(CurrentPC != nullptr && "PlayerController isn't a UINavController");
	if (CurrentPC == nullptr)
	{
		DISPLAYERROR("PlayerController isn't a UINavController");
		return;
	}
	WidgetClass = GetClass();

	if (bUseSelector && bUseMovementCurve)
	{
		//check(MoveCurve != nullptr && "UseMovementCurve is true but MoveCurve is null");
		if (MoveCurve == nullptr)
		{
			DISPLAYERROR("UseMovementCurve is true but MoveCurve is null");
			return;
		}
	}

	FetchButtonsInHierarchy();
	ReadyForSetup();

	//check(UINavButtons.Num() == ButtonNavigations.Num() && "Dimension of UIUINavButtons and ButtonNavigations array is not the same.");
	if (UINavButtons.Num() != ButtonNavigations.Num())
	{
		DISPLAYERROR("Dimension of UIUINavButtons and ButtonNavigations array is not the same.");
		return;
	}

	if (bUseTextColor) ChangeTextColorToDefault();

	//If this widget doesn't need to create the selector, notify ReadyForSetup
	if (!bUseSelector)
	{
		UINavSetup();
		bShouldTick = false;
		return;
	}

	if (bUseSelector) SetupSelector();
}

void UUINavWidget::FetchButtonsInHierarchy()
{
	TraverseHierarquy();

	//check(FirstButtonIndex < UINavButtons.Num() && "Invalid FirstButton index, can't be greater than number of buttons.");
	//check(FirstButtonIndex > -1 && "Invalid FirstButton index, can't be less than 0.");

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

	while (UINavButtons[ButtonIndex]->Visibility == ESlateVisibility::Collapsed ||
		UINavButtons[ButtonIndex]->Visibility == ESlateVisibility::Hidden ||
		!UINavButtons[ButtonIndex]->bIsEnabled)
	{
		ButtonIndex++;
	}
}

void UUINavWidget::TraverseHierarquy()
{
	//Find UIUINavButtons in the widget hierarchy
	TArray<UWidget*> Widgets;
	WidgetTree->GetAllWidgets(Widgets);
	for (int i = 0; i < Widgets.Num(); ++i)
	{
		UScrollBox* Scroll = Cast<UScrollBox>(Widgets[i]);
		if (Scroll != nullptr)
		{
			ScrollBoxes.Add(Scroll);
		}

		UUINavButton* NewNavButton = Cast<UUINavButton>(Widgets[i]);
		if (NewNavButton == nullptr)
		{
			UUINavComponent* UIComp = Cast<UUINavComponent>(Widgets[i]);
			if (UIComp != nullptr)
			{
				NewNavButton = Cast<UUINavButton>(UIComp->NavButton);

				UINavComponentsIndices.Add(UINavButtons.Num());
				UINavComponents.Add(UIComp);

				if (Cast<UUINavOptionBox>(Widgets[i]))
				{
					OptionBoxIndices.Add(UINavButtons.Num());
					UINavOptionBoxes.Add(Cast<UUINavOptionBox>(Widgets[i]));
				}
			}
		}

		if (NewNavButton == nullptr) continue;

		if (!bOverrideButtonIndices)
		{
			NewNavButton->ButtonIndex = UINavButtons.Num();
		}
		NewNavButton->CustomHover.AddDynamic(this, &UUINavWidget::HoverEvent);
		NewNavButton->CustomUnhover.AddDynamic(this, &UUINavWidget::UnhoverEvent);
		NewNavButton->CustomClick.AddDynamic(this, &UUINavWidget::ClickEvent);
		NewNavButton->CustomRelease.AddDynamic(this, &UUINavWidget::ReleaseEvent);
		bSwitchedStyle.Add(false);

		//Add button to array of UIUINavButtons
		UINavButtons.Add(NewNavButton);
	}
	/*UINavButtons.Sort([](const UUINavButton& Wid1, const UUINavButton& Wid2)
	{
	return Wid1.ButtonIndex < Wid2.ButtonIndex;
	});*/
}

void UUINavWidget::ChangeTextColorToDefault()
{
	UTextBlock* TextBlock = nullptr;
	for (int j = 0; j < UINavButtons.Num(); j++)
	{
		SwitchTextColorTo(j, TextDefaultColor);
	}
}

void UUINavWidget::SetupSelector()
{
	//check(TheSelector != nullptr && "Couldn't find TheSelector");

	if (TheSelector == nullptr)
	{
		DISPLAYERROR("Couldn't find TheSelector");
		return;
	}

	TheSelector->SetVisibility(ESlateVisibility::Hidden);

	UCanvasPanelSlot* SelectorSlot = Cast<UCanvasPanelSlot>(TheSelector->Slot);

	//TODO: TRY TO DEACTIVATE
	//Make sure the selector is centered
	SelectorSlot->SetAlignment(FVector2D(0.5f, 0.5f));

	InitialOffset = GEngine->GameViewport->Viewport->GetSizeXY() / 2;
	/*Set an initial position close to the middle of the screen,
	otherwise the selector's image might get cropped*/
	SelectorSlot->SetPosition(InitialOffset);

	TheSelector->SetRenderTranslation(-InitialOffset - FVector2D(0.f, -1000.f));
}

void UUINavWidget::NativeTick(const FGeometry & MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

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
		}

		WaitForTick++;
	}
}

FReply UUINavWidget::NativeOnMouseMove(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent)
{
	if (CurrentPC->GetCurrentInputType() != EInputType::Mouse && InMouseEvent.GetCursorDelta().Size() > 0.f)
	{
		CurrentPC->NotifyMouseInputType();
	}
	return FReply::Handled();
}

FReply UUINavWidget::NativeOnKeyDown(const FGeometry & InGeometry, const FKeyEvent & InKeyEvent)
{
	Super::NativeOnKeyDown(InGeometry, InKeyEvent);
	if (PressedKeys.Find(InKeyEvent.GetKey()) == INDEX_NONE)
	{
		PressedKeys.Add(InKeyEvent.GetKey());
		FindActionByKey(InKeyEvent.GetKey(), true);
	}
	return FReply::Handled();
}

FReply UUINavWidget::NativeOnKeyUp(const FGeometry & InGeometry, const FKeyEvent & InKeyEvent)
{
	Super::NativeOnKeyUp(InGeometry, InKeyEvent);
	PressedKeys.Remove(InKeyEvent.GetKey());
	FindActionByKey(InKeyEvent.GetKey(), false);
	return FReply::Handled();
}

void UUINavWidget::FindActionByKey(FKey PressedKey, bool bPressed)
{
	TArray<FString> ActionNames;
	TMap<FString, TArray<FKey>> KeyMap = CurrentPC->GetKeyMap();
	KeyMap.GetKeys(ActionNames);

	for (FString Action : ActionNames)
	{
		TArray<FKey> Keys = KeyMap[Action];
		for (FKey Key : Keys)
		{
			if (Key == PressedKey)
			{
				GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, TEXT("Notify PC"));
				NotifyPCByAction(Action, bPressed);
				return;
			}
		}
	}
}

void UUINavWidget::NotifyPCByAction(FString Action, bool bPressed)
{
	if (Action.Equals("MenuUp"))
	{
		if (bPressed)
		{
			CurrentPC->StartMenuUp();
		}
		else
		{
			CurrentPC->MenuUpRelease();
		}
	}
	else if (Action.Equals("MenuDown"))
	{
		if (bPressed)
		{
			CurrentPC->StartMenuDown();
		}
		else
		{
			CurrentPC->MenuDownRelease();
		}
	}
	else if (Action.Equals("MenuLeft"))
	{
		if (bPressed)
		{
			CurrentPC->StartMenuLeft();
		}
		else
		{
			CurrentPC->MenuLeftRelease();
		}
	}
	else if (Action.Equals("MenuRight"))
	{
		if (bPressed)
		{
			CurrentPC->StartMenuRight();
		}
		else
		{
			CurrentPC->MenuRightRelease();
		}
	}
	else if (Action.Equals("MenuSelect"))
	{
		if (bPressed)
		{
			CurrentPC->MenuSelect();
		}
	}
	else if (Action.Equals("MenuReturn"))
	{
		if (bPressed)
		{
			CurrentPC->MenuReturn();
		}
	}
}

void UUINavWidget::HandleSelectorMovement(float DeltaTime)
{
	MovementCounter += DeltaTime;

	if (MovementCounter >= MovementTime)
	{
		MovementCounter = 0.f;
		bMovingSelector = false;
		bAllowNavigation = true;
		TheSelector->SetRenderTranslation(SelectorDestination);
		return;
	}

	TheSelector->SetRenderTranslation(SelectorOrigin + Distance*MoveCurve->GetFloatValue(MovementCounter));
}

void UUINavWidget::AppendVerticalNavigation(int Dimension, FButtonNavigation EdgeNavigation, bool bWrap)
{
	if (Dimension == -1) Dimension = UINavButtons.Num();
	//check(Dimension > 0 && "Append Navigation Dimension should be greater than 0");
	if (Dimension <= 0)
	{
		DISPLAYERROR("Append Navigation Dimension should be greater than 0");
		return;
	}

	TArray<FButtonNavigation> ButtonsNav = TArray<FButtonNavigation>();
	FButtonNavigation NewNav;

	int StartingIndex = ButtonNavigations.Num();

	for (int i = 0; i < Dimension; i++)
	{
		if (i == 0)
		{
			NewNav.UpButton = EdgeNavigation.UpButton == -1 ? (bWrap ? StartingIndex + Dimension - 1 : NewNav.UpButton) : EdgeNavigation.UpButton;
		}
		else
		{
			NewNav.UpButton = StartingIndex + i - 1;
		}

		if (i == Dimension - 1)
		{
			NewNav.DownButton = EdgeNavigation.DownButton == -1 ? (bWrap ? StartingIndex : NewNav.DownButton) : EdgeNavigation.DownButton;
		}
		else
		{
			NewNav.DownButton = StartingIndex + i + 1;
		}

		if (EdgeNavigation.LeftButton != -1)
		{
			NewNav.LeftButton = EdgeNavigation.LeftButton;
		}
		if (EdgeNavigation.RightButton != -1)
		{
			NewNav.RightButton = EdgeNavigation.RightButton;
		}

		ButtonsNav.Add(NewNav);
	}

	ButtonNavigations.Append(ButtonsNav);
}

void UUINavWidget::AppendHorizontalNavigation(int Dimension, FButtonNavigation EdgeNavigation, bool bWrap)
{
	if (Dimension == -1) Dimension = UINavButtons.Num();
	//check(Dimension > 0 && "Append Navigation Dimension should be greater than 0");
	if (Dimension <= 0)
	{
		DISPLAYERROR("Append Navigation Dimension should be greater than 0");
		return;
	}

	TArray<FButtonNavigation> ButtonsNav = TArray<FButtonNavigation>();
	FButtonNavigation NewNav;

	int StartingIndex = ButtonNavigations.Num();

	for (int i = 0; i < Dimension; i++)
	{
		if (i == 0)
		{
			NewNav.LeftButton = EdgeNavigation.LeftButton == -1 ? (bWrap ? StartingIndex + Dimension - 1 : NewNav.LeftButton) : EdgeNavigation.LeftButton;
		}
		else
		{
			NewNav.LeftButton = StartingIndex + i - 1;
		}

		if (i == Dimension - 1)
		{
			NewNav.RightButton = EdgeNavigation.RightButton == -1 ? (bWrap ? StartingIndex : NewNav.RightButton) : EdgeNavigation.RightButton;
		}
		else
		{
			NewNav.RightButton = StartingIndex + i + 1;
		}

		if (EdgeNavigation.UpButton != -1)
		{
			NewNav.UpButton = EdgeNavigation.UpButton;
		}
		if (EdgeNavigation.DownButton != -1)
		{
			NewNav.DownButton = EdgeNavigation.DownButton;
		}

		ButtonsNav.Add(NewNav);
	}

	ButtonNavigations.Append(ButtonsNav);
}

void UUINavWidget::AppendGridNavigation(int DimensionX, int DimensionY, FButtonNavigation EdgeNavigation, bool bWrap)
{
	//check(DimensionX > 0 && "Append Navigation Dimension should be greater than 0");
	//check(DimensionY > 0 && "Append Navigation Dimension should be greater than 0");
	if (DimensionX <= 0)
	{
		DISPLAYERROR("Append Navigation Dimension should be greater than 0");
		return;
	}
	if (DimensionY <= 0)
	{
		DISPLAYERROR("Append Navigation Dimension should be greater than 0");
		return;
	}

	TArray<FButtonNavigation> ButtonsNav = TArray<FButtonNavigation>();
	FButtonNavigation NewNav;

	int StartingIndex = ButtonNavigations.Num();

	int Iterations = DimensionX * DimensionY;
	int i;
	for (i = 0; i < Iterations; i++)
	{

		//Top edge
		if (i < DimensionX)
		{
			NewNav.UpButton = bWrap ? i + DimensionX * (DimensionY - 1) : EdgeNavigation.UpButton;
		}
		//Rest of the grid
		else
		{
			NewNav.UpButton = StartingIndex + i - DimensionX;
		}

		//Bottom edge
		if ((i / DimensionX) == (DimensionY - 1))
		{
			NewNav.DownButton = bWrap ? i % DimensionX : EdgeNavigation.DownButton;
		}
		//Rest of the grid
		else
		{
			NewNav.DownButton = StartingIndex + i + DimensionX;
		}

		//Left edge
		if (((i + DimensionX) % DimensionX) == 0)
		{
			NewNav.LeftButton = bWrap ? i - 1 + DimensionX : EdgeNavigation.LeftButton;
		}
		//Rest of the grid
		else
		{
			NewNav.LeftButton = StartingIndex + i - 1;
		}

		//Right edge
		if (((i + 1 + DimensionX) % DimensionX) == 0)
		{
			NewNav.RightButton = bWrap ? i + 1 - DimensionX : EdgeNavigation.RightButton;
		}
		//Rest of the grid
		else
		{
			NewNav.RightButton = StartingIndex + i + 1;
		}

		ButtonsNav.Add(NewNav);
	}

	ButtonNavigations.Append(ButtonsNav);
}

void UUINavWidget::UINavSetup()
{
	/*
	If this widget was added through a parent widget and should remove it from the viewport,
	remove that widget from viewport
	*/
	if (ParentWidget != nullptr && ParentWidget->IsInViewport())
	{
		if (bParentRemoved)
		{
			CurrentPC = ParentWidget->CurrentPC;
			ParentWidget->RemoveFromParent();
			ParentWidget->Destruct();
		}
	}
	//If this widget was added through a child widget, remove it from viewport
	else if (ReturnedFromWidget != nullptr)
	{
		CurrentPC = ReturnedFromWidget->CurrentPC;
		if (ReturnedFromWidget->IsInViewport())
		{
			ReturnedFromWidget->RemoveFromParent();
			ReturnedFromWidget->Destruct();
		}
		ReturnedFromWidget = nullptr;
	}

	CurrentPC->SetActiveWidget(this);
	CurrentPC->EnableInput(CurrentPC);

	if (bUseSelector)
	{
		if (TheSelector == nullptr) return;
		TheSelector->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	OnNavigate(-1, ButtonIndex);
	NavigateTo(ButtonIndex);
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
	}
	
	FVector2D PixelPos, ViewportPos;
	USlateBlueprintLibrary::LocalToViewport(GetWorld(), Geom, LocalPosition, PixelPos, ViewportPos);
	//The selector's position is initially set to (500, 500) so subtract that from the new position
	ViewportPos -= InitialOffset;
	ViewportPos += SelectorOffset;
	return ViewportPos;
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
		//check(NewText != nullptr && "When bUseTextColor is true, UINavComponent should have a valid TextBlock called NavText.");
		if (NewText == nullptr)
		{
			DISPLAYERROR("When bUseTextColor is true, UINavComponent should have a valid TextBlock called NavText.");
			return;
		}
	}
	else
	{
		NewText = Cast<UTextBlock>(UINavButtons[Index]->GetChildAt(0));
		//check(NewText != nullptr && "When bUseTextColor is true, UINavButton should have a TextBlock as its child.");
		if (NewText == nullptr)
		{
			DISPLAYERROR("When bUseTextColor is true, UINavButton should have a TextBlock as its child.");
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

void UUINavWidget::SetAllowNavigation(bool bAllow)
{
	bAllowNavigation = bAllow;
}

void UUINavWidget::SetSelectorScale(FVector2D NewScale)
{
	if (TheSelector == nullptr) return;
	TheSelector->SetRenderScale(NewScale);
}

void UUINavWidget::SetSelector(UUserWidget * NewSelector)
{
	//check(TheSelector != nullptr && "Received invalid Selector");
	if (NewSelector == nullptr)
	{
		DISPLAYERROR("Received invalid Selector");
	}
	TheSelector = NewSelector;
}

void UUINavWidget::SetSelectorVisibility(bool bVisible)
{
	ESlateVisibility Vis = bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
	TheSelector->SetVisibility(Vis);
}

void UUINavWidget::NavigateTo(int Index, bool bHoverEvent)
{
	bool bShouldNotify = false;
	if (Index != ButtonIndex) bShouldNotify = true;
		
	if (bUseButtonStates)
	{
		UpdateButtonsStates(Index, bHoverEvent);
	}
	if (bUseSelector)
	{
		if (bUseMovementCurve)
		{
			BeginSelectorMovement(Index);
		}
		else
		{
			UpdateSelectorLocation(Index);
		}
	}
	if (bUseTextColor)
	{
		UpdateTextColor(Index);
	}

	if (bShouldNotify) OnNavigate(ButtonIndex, Index);
	ButtonIndex = Index;

	//Update all the possible scroll boxes in the widget
	for (int i = 0; i < ScrollBoxes.Num(); ++i)
	{
		ScrollBoxes[i]->ScrollWidgetIntoView(UINavButtons[ButtonIndex]);
	}
}

void UUINavWidget::BeginSelectorMovement(int Index)
{
	SelectorOrigin = GetButtonLocation(ButtonIndex);
	SelectorDestination = GetButtonLocation(Index);
	Distance = SelectorDestination - SelectorOrigin;
	float MinTime, MaxTime;
	MoveCurve->GetTimeRange(MinTime, MaxTime);
	MovementTime = MaxTime - MinTime;
	bMovingSelector = true;
	bAllowNavigation = false;
}

void UUINavWidget::OnNavigate_Implementation(int From, int To)
{

}

void UUINavWidget::OnReturnToParent_Implementation()
{

}

void UUINavWidget::HoverEvent(int Index)
{
	CurrentPC->NotifyMouseInputType();

	if (!bAllowNavigation) return;

	NavigateTo(Index, true);
}

void UUINavWidget::UnhoverEvent(int Index)
{
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
	CurrentPC->NotifyMouseInputType();

	if (!bAllowNavigation) return;

	OnSelect(Index);
}

void UUINavWidget::ReleaseEvent(int Index)
{
	//Handle button style switching when mouse is released
	if (!UINavButtons[Index]->IsHovered())
	{
		SwitchButtonStyle(Index);
		for (int i = 0; i < UINavButtons.Num(); i++)
		{
			if (i == Index) continue;
			if (UINavButtons[i]->IsHovered())
			{
				SwitchButtonStyle(i);
				return;
			}
		}
	}
}

void UUINavWidget::OnSelect_Implementation(int Index)
{

}

UWidget* UUINavWidget::GoToWidget(TSubclassOf<UUINavWidget> NewWidgetClass, bool bRemoveParent)
{
	//check(NewWidgetClass != nullptr && "GoToWidget: No Widget Class found");
	if (NewWidgetClass == nullptr)
	{
		DISPLAYERROR("GoToWidget: No Widget Class found");
		return nullptr;
	}

	CurrentPC->DisableInput(CurrentPC);
	UUINavWidget* NewWidget = CreateWidget<UUINavWidget>(CurrentPC, NewWidgetClass);
	NewWidget->ParentWidget = this;
	NewWidget->ParentWidgetClass = WidgetClass;
	NewWidget->bParentRemoved = bRemoveParent;
	NewWidget->InputMode = InputMode;
	NewWidget->AddToViewport();
	return NewWidget;
}

void UUINavWidget::MenuNavigate(ENavigationDirection Direction)
{
	int LocalIndex = FetchDirection(Direction);
	if (LocalIndex == -1) return;

	//Check if the button is visible, if not, skip to next button
	while (UINavButtons[LocalIndex]->Visibility == ESlateVisibility::Collapsed ||
		UINavButtons[LocalIndex]->Visibility == ESlateVisibility::Hidden ||
		!UINavButtons[ButtonIndex]->bIsEnabled)
	{
		LocalIndex = FetchDirection(Direction);
	}

	NavigateTo(LocalIndex);
}

int UUINavWidget::FetchDirection(ENavigationDirection Direction)
{
	int LocalIndex = -1;

	switch (Direction)
	{
		case ENavigationDirection::Nav_UP:
			LocalIndex = ButtonNavigations[ButtonIndex].UpButton;
			break;
		case ENavigationDirection::Nav_DOWN:
			LocalIndex = ButtonNavigations[ButtonIndex].DownButton;
			break;
		case ENavigationDirection::Nav_LEFT:
			LocalIndex = ButtonNavigations[ButtonIndex].LeftButton;
			break;
		case ENavigationDirection::Nav_RIGHT:
			LocalIndex = ButtonNavigations[ButtonIndex].RightButton;
			break;
		default:
			break;
	}

	return LocalIndex;
}

void UUINavWidget::ReturnToParent()
{
	if (ParentWidget == nullptr)
	{
		if (bAllowRemoveIfRoot)
		{
			OnReturnToParent();
			RemoveFromParent();
			Destruct();
		}
		return;
	}

	OnReturnToParent();
	//If parent was removed, add it to viewport
	if (bParentRemoved)
	{
		CurrentPC->DisableInput(CurrentPC);

		ParentWidget = CreateWidget<UUINavWidget>(CurrentPC, ParentWidgetClass);
		ParentWidget->ReturnedFromWidget = this;
		ParentWidget->AddToViewport();
	}
	else
	{
		RemoveFromParent();
		Destruct();

		CurrentPC->SetActiveWidget(ParentWidget);
		CurrentPC->EnableInput(CurrentPC);
	}
}

void UUINavWidget::ChangeInputMode(EInputMode NewInputMode)
{
	InputMode = NewInputMode;
	switch (InputMode)
	{
		case EInputMode::GameAndUI:
			SetUserFocus(nullptr);
			break;
		case EInputMode::UIOnly:
			SetUserFocus(CurrentPC);
			break;
	}
}

UUINavButton * UUINavWidget::GetUINavButtonAtIndex(int Index)
{
	if (UINavButtons.Num() <= Index)
	{
		DISPLAYERROR("GetUINavButtonAtIndex was given an invalid index!");
		return nullptr;
	}
	return UINavButtons[Index];
}

UUINavComponent * UUINavWidget::GetUINavComponentAtIndex(int Index)
{
	int ValidIndex = UINavComponentsIndices.Find(Index);
	if (ValidIndex == INDEX_NONE) 
	{
		DISPLAYERROR("GetUINavComponentAtIndex: Element at given index isn't a UINavComponent");
		return nullptr;
	}
	return UINavComponents[ValidIndex];
}

UUINavOptionBox * UUINavWidget::GetUINavOptionBoxAtIndex(int Index)
{
	int ValidIndex = OptionBoxIndices.Find(Index);
	if (ValidIndex == INDEX_NONE)
	{
		DISPLAYERROR("GetUINavOptionBoxAtIndex: Element at given index isn't a UINavOptionBox");
		return nullptr;
	}
	return UINavOptionBoxes[ValidIndex];
}

void UUINavWidget::MenuUp()
{
	if (!bAllowNavigation) return;
	MenuNavigate(ENavigationDirection::Nav_UP);
}

void UUINavWidget::MenuDown()
{
	if (!bAllowNavigation) return;
	MenuNavigate(ENavigationDirection::Nav_DOWN);
}

void UUINavWidget::MenuLeft()
{
	if (!bAllowNavigation) return;

	int SliderIndex = OptionBoxIndices.Find(ButtonIndex);
	if (SliderIndex == INDEX_NONE)
	{
		MenuNavigate(ENavigationDirection::Nav_LEFT);
	}
	else
	{
		UINavOptionBoxes[SliderIndex]->NavigateLeft();
	}
}

void UUINavWidget::MenuRight()
{
	if (!bAllowNavigation) return;

	int SliderIndex = OptionBoxIndices.Find(ButtonIndex);
	if (SliderIndex == INDEX_NONE)
	{
		MenuNavigate(ENavigationDirection::Nav_RIGHT);
	}
	else
	{
		UINavOptionBoxes[SliderIndex]->NavigateRight();
	}
}

void UUINavWidget::MenuSelect()
{
	if (!bAllowNavigation) return;
	OnSelect(ButtonIndex);
}

void UUINavWidget::MenuReturn()
{
	if (!bAllowNavigation) return;
	ReturnToParent();
}


