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

	bIsFocusable = false;

	CurrentPC = Cast<AUINavController>(GetOwningPlayer());

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

	//check(NavButtons.Num() == ButtonNavigations.Num() && "Dimension of UINavButtons and ButtonNavigations array is not the same.");
	if (NavButtons.Num() != ButtonNavigations.Num())
	{
		DISPLAYERROR("Dimension of UINavButtons and ButtonNavigations array is not the same.");
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

	if (TheSelector == nullptr) CreateSelector();
}

void UUINavWidget::FetchButtonsInHierarchy()
{
	TraverseHierarquy();

	//check(FirstButtonIndex < NavButtons.Num() && "Invalid FirstButton index, can't be greater than number of buttons.");
	//check(FirstButtonIndex > -1 && "Invalid FirstButton index, can't be less than 0.");

	if (FirstButtonIndex >= NavButtons.Num())
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

	while (NavButtons[ButtonIndex]->Visibility == ESlateVisibility::Collapsed ||
		NavButtons[ButtonIndex]->Visibility == ESlateVisibility::Hidden ||
		!NavButtons[ButtonIndex]->bIsEnabled)
	{
		ButtonIndex++;
	}
}

void UUINavWidget::TraverseHierarquy()
{
	//Find UINavButtons in the widget hierarchy
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

				NavComponentsIndices.Add(NavButtons.Num());
				NavComponents.Add(UIComp);

				if (Cast<UUINavOptionBox>(Widgets[i]))
				{
					SliderIndices.Add(NavButtons.Num());
					Sliders.Add(Cast<UUINavOptionBox>(Widgets[i]));
				}
			}
		}

		if (NewNavButton == nullptr) continue;

		NewNavButton->ButtonIndex = NavButtons.Num();
		NewNavButton->CustomHover.AddDynamic(this, &UUINavWidget::HoverEvent);
		NewNavButton->CustomUnhover.AddDynamic(this, &UUINavWidget::UnhoverEvent);
		NewNavButton->CustomClick.AddDynamic(this, &UUINavWidget::ClickEvent);
		NewNavButton->CustomRelease.AddDynamic(this, &UUINavWidget::ReleaseEvent);
		bSwitchedStyle.Add(false);

		//Add button to array of UINavButtons
		NavButtons.Add(NewNavButton);
	}
	/*NavButtons.Sort([](const UUINavButton& Wid1, const UUINavButton& Wid2)
	{
	return Wid1.ButtonIndex < Wid2.ButtonIndex;
	});*/
}

void UUINavWidget::ChangeTextColorToDefault()
{
	UTextBlock* TextBlock = nullptr;
	for (int j = 0; j < NavButtons.Num(); j++)
	{
		int NavCompIndex = NavComponentsIndices.Find(j);
		if (NavCompIndex != INDEX_NONE)
		{
			TextBlock = NavComponents[NavCompIndex]->NavText;
			//check(TextBlock != nullptr && "When bUseTextColor is true, UINavComponent should have a valid TextBlock called NavText.");
			if (TextBlock == nullptr)
			{
				DISPLAYERROR("When bUseTextColor is true, UINavComponent should have a valid TextBlock called NavText.");
				return;
			}
		}
		else
		{
			TextBlock = Cast<UTextBlock>(NavButtons[j]->GetChildAt(0));
			//check(TextBlock != nullptr && "When bUseTextColor is true, UINavButton should have a TextBlock as its child.");
			if (TextBlock == nullptr)
			{
				DISPLAYERROR("When bUseTextColor is true, UINavButton should have a TextBlock as its child.");
				return;
			}
		}
		TextBlock->SetColorAndOpacity(TextDefaultColor);
	}
}

void UUINavWidget::CreateSelector()
{
	UPanelWidget* RootPanel = Cast<UPanelWidget>(GetRootWidget());
	//Create the selector
	TheSelector = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Selector"));

	//check(TheSelector != nullptr && "Couldn't construct TheSelector");
	//check(SelectorImage != nullptr && "bUseSelector is true but SelectorImage is null!");

	if (TheSelector == nullptr)
	{
		DISPLAYERROR("Couldn't construct TheSelector");
		return;
	}
	if (SelectorImage == nullptr)
	{
		DISPLAYERROR("bUseSelector is true but SelectorImage is null!");
		return; 
	}

	RootPanel->AddChild(TheSelector);
	TheSelector->SetBrushFromTexture(SelectorImage);
	TheSelector->SetVisibility(ESlateVisibility::Hidden);

	UCanvasPanelSlot* SelectorSlot = Cast<UCanvasPanelSlot>(TheSelector->Slot);

	//Make sure the selector is centered
	SelectorSlot->SetAlignment(FVector2D(0.5f, 0.5f));

	InitialOffset = GEngine->GameViewport->Viewport->GetSizeXY() / 2;
	/*Set an initial position close to the middle of the screen,
	otherwise the selector's image might get cropped*/
	SelectorSlot->SetPosition(InitialOffset);

	FVector2D SelectorSize = FVector2D(SelectorImage->GetSizeX() * SelectorScale.X, SelectorImage->GetSizeY() * SelectorScale.Y);
	SelectorSlot->SetSize(SelectorSize);
	SelectorSlot->SetZOrder(SelectorZOrder);
	TheSelector->SetRenderScale(SelectorScale);
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

void UUINavWidget::HandleSelectorMovement(float DeltaTime)
{
	MovementCounter += DeltaTime;

	if (MovementCounter >= MovementTime)
	{
		MovementCounter = 0.f;
		bMovingSelector = false;
		TheSelector->SetRenderTranslation(SelectorDestination);
		return;
	}

	TheSelector->SetRenderTranslation(SelectorOrigin + Distance*MoveCurve->GetFloatValue(MovementCounter));
}

void UUINavWidget::AppendVerticalNavigation(int Dimension, FButtonNavigation EdgeNavigation, bool bWrap)
{
	if (Dimension == -1) Dimension = NavButtons.Num();
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
	if (Dimension == -1) Dimension = NavButtons.Num();
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
		TheSelector->SetVisibility(ESlateVisibility::Visible);
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
	FGeometry Geom = NavButtons[Index]->GetCachedGeometry();
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
	UTextBlock* PreviousText = nullptr;
	UTextBlock* NewText = nullptr;

	//Change text color on the button that was navigated from
	int PreviousComponentIndex = NavComponentsIndices.Find(ButtonIndex);
	if (PreviousComponentIndex != INDEX_NONE)
	{
		PreviousText = NavComponents[PreviousComponentIndex]->NavText;
		//check(PreviousText != nullptr && "When bUseTextColor is true, UINavComponent should have a valid TextBlock called NavText.");
		if (PreviousText == nullptr)
		{
			DISPLAYERROR("When bUseTextColor is true, UINavComponent should have a valid TextBlock called NavText.");
			return;
		}
	}
	else
	{
		PreviousText = Cast<UTextBlock>(NavButtons[ButtonIndex]->GetChildAt(0));
		//check(PreviousText != nullptr && "When bUseTextColor is true, UINavButton should have a TextBlock as its child.");
		if (PreviousText == nullptr)
		{
			DISPLAYERROR("When bUseTextColor is true, UINavButton should have a TextBlock as its child.");
			return;
		}
	}
	PreviousText->SetColorAndOpacity(TextDefaultColor);

	//Change text color on the button that was navigated to
	int NewComponentIndex = NavComponentsIndices.Find(Index);
	if (NewComponentIndex != INDEX_NONE)
	{
		NewText = NavComponents[NewComponentIndex]->NavText;
		//check(NewText != nullptr && "When bUseTextColor is true, UINavComponent should have a valid TextBlock called NavText.");
		if (NewText == nullptr)
		{
			DISPLAYERROR("When bUseTextColor is true, UINavComponent should have a valid TextBlock called NavText.");
			return;
		}
	}
	else
	{
		NewText = Cast<UTextBlock>(NavButtons[Index]->GetChildAt(0));
		//check(NewText != nullptr && "When bUseTextColor is true, UINavButton should have a TextBlock as its child.");
		if (NewText == nullptr)
		{
			DISPLAYERROR("When bUseTextColor is true, UINavButton should have a TextBlock as its child.");
			return;
		}
	}
	NewText->SetColorAndOpacity(TextNavigatedColor);
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
	UUINavButton* TheButton = NavButtons[Index];
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

void UUINavWidget::SetSelector(UImage * NewSelector)
{
	//check(TheSelector != nullptr && "No Selector found");
	if (NewSelector == nullptr)
	{
		DISPLAYERROR("Received invalid Selector");
	}
	TheSelector = NewSelector;
}

void UUINavWidget::SetSelectorBrush(UTexture2D * NewBrush)
{
	if (NewBrush == nullptr) return;
	TheSelector->SetBrushFromTexture(NewBrush);
}

void UUINavWidget::SetSelectorVisibility(bool bVisible)
{
	ESlateVisibility Vis = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
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
		ScrollBoxes[i]->ScrollWidgetIntoView(NavButtons[ButtonIndex]);
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
	OnSelect(Index);
}

void UUINavWidget::ReleaseEvent(int Index)
{
	//Handle button style switching when mouse is released
	if (!NavButtons[Index]->IsHovered())
	{
		SwitchButtonStyle(Index);
		for (int i = 0; i < NavButtons.Num(); i++)
		{
			if (i == Index) continue;
			if (NavButtons[i]->IsHovered())
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
	NewWidget->AddToViewport();
	return NewWidget;
}

void UUINavWidget::MenuNavigate(ENavigationDirection Direction)
{
	int LocalIndex = FetchDirection(Direction);
	if (LocalIndex == -1) return;

	//Check if the button is visible, if not, skip to next button
	while (NavButtons[LocalIndex]->Visibility == ESlateVisibility::Collapsed ||
		NavButtons[LocalIndex]->Visibility == ESlateVisibility::Hidden ||
		!NavButtons[ButtonIndex]->bIsEnabled)
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

void UUINavWidget::MenuUp()
{
	if (bMovingSelector) return;
	MenuNavigate(ENavigationDirection::Nav_UP);
}

void UUINavWidget::MenuDown()
{
	if (bMovingSelector) return;
	MenuNavigate(ENavigationDirection::Nav_DOWN);
}

void UUINavWidget::MenuLeft()
{
	if (bMovingSelector) return;

	int SliderIndex = SliderIndices.Find(ButtonIndex);
	if (SliderIndex == INDEX_NONE)
	{
		MenuNavigate(ENavigationDirection::Nav_LEFT);
	}
	else
	{
		Sliders[SliderIndex]->NavigateLeft();
	}
}

void UUINavWidget::MenuRight()
{
	if (bMovingSelector) return;

	int SliderIndex = SliderIndices.Find(ButtonIndex);
	if (SliderIndex == INDEX_NONE)
	{
		MenuNavigate(ENavigationDirection::Nav_RIGHT);
	}
	else
	{
		Sliders[SliderIndex]->NavigateRight();
	}
}

void UUINavWidget::MenuSelect()
{
	if (bMovingSelector) return;

	OnSelect(ButtonIndex);
}

void UUINavWidget::MenuReturn()
{
	if (bMovingSelector) return;

	ReturnToParent();
}


