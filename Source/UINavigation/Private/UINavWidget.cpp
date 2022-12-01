// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

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
#include "UINavPromptWidget.h"
#include "UINavSettings.h"
#include "UINavWidgetComponent.h"
#include "UINavBlueprintFunctionLibrary.h"
#include "UINavMacros.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/UniformGridSlot.h"
#include "Components/ActorComponent.h"
#include "Components/ListView.h"
#if IS_VR_PLATFORM
#include "HeadMountedDisplayFunctionLibrary.h"
#endif

UUINavWidget::UUINavWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bIsFocusable = true;
}

void UUINavWidget::NativeConstruct()
{
	const UWorld* const World = GetWorld();
	OuterUINavWidget = Cast<UUINavWidget>(GetOuter() != nullptr ? GetOuter()->GetOuter() : nullptr);
	if (OuterUINavWidget != nullptr)
	{
		ParentWidget = OuterUINavWidget;
	}
	if (World != nullptr)
	{
		if (const UGameViewportClient* ViewportClient = World->GetGameViewport())
		{
			bUsingSplitScreen = ViewportClient->GetCurrentSplitscreenConfiguration() != ESplitScreenType::None;
		}
	}
	/*
	If this widget was added through a parent widget and should remove it from the viewport,
	remove that widget from viewport
	*/
	if (ParentWidget != nullptr && ParentWidget->IsInViewport() && bParentRemoved)
	{
		UUINavWidget* OuterParentWidget = ParentWidget->GetMostOuterUINavWidget();
		OuterParentWidget->bReturningToParent = true;
		OuterParentWidget->RemoveFromParent();

		if (bShouldDestroyParent)
		{
			OuterParentWidget->Destruct();
			OuterParentWidget = nullptr;
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

void UUINavWidget::InitialSetup(const bool bRebuilding)
{
	if (!bRebuilding)
	{
		WidgetClass = GetClass();
		if (UINavPC == nullptr)
		{
			ConfigureUINavPC();
		}

		//If widget was already setup, apply only certain steps
		if (bCompletedSetup)
		{
			ReconfigureSetup();
			return;
		}

		bSetupStarted = true;
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
	}
	else
	{
		SetupSelector();
	}

	for (UUINavWidget* ChildUINavWidget : ChildUINavWidgets)
	{
		ChildUINavWidget->ReconfigureSetup();
	}
}

void UUINavWidget::CleanSetup()
{
	for (UUINavWidget* ChildUINavWidget : ChildUINavWidgets)
	{
		ChildUINavWidget->CleanSetup();
	}
	
	//Disable all buttons (bug fix)
	SetEnableUINavButtons(false, false);
	
	bSetupStarted = false;
	SelectedButtonIndex = -1;
}

void UUINavWidget::FetchButtonsInHierarchy()
{
	TraverseHierarchy(this, this);

	const int ButtonsNum = UINavButtons.Num();
	if (FirstButtonIndex >= ButtonsNum && ButtonsNum > 0)
	{
		DISPLAYERROR("Invalid FirstButton index, can't be greater than number of buttons.");
		return;
	}

	if (FirstButtonIndex < 0) FirstButtonIndex = 0;

	ButtonIndex = FirstButtonIndex;
	if (ButtonsNum > 0) CurrentButton = UINavButtons[FirstButtonIndex];
	else return;

	const bool bIgnoreDisabledUINavButton = GetDefault<UUINavSettings>()->bIgnoreDisabledUINavButton;
	bool bValid = CurrentButton->IsValid(bIgnoreDisabledUINavButton);

	if (bValid)
	{
		UUINavComponent* UINavComp = GetUINavComponentAtIndex(ButtonIndex);
		if (UINavComp != nullptr && !UINavComp->IsValid(bIgnoreDisabledUINavButton)) bValid = false;
	}

	while (!bValid)
	{
		ButtonIndex++;
		if (ButtonIndex >= UINavButtons.Num()) ButtonIndex = 0;
		
		CurrentButton = UINavButtons[ButtonIndex];
		if (ButtonIndex == FirstButtonIndex) break;

		UUINavComponent* UINavComp = GetUINavComponentAtIndex(ButtonIndex);
		if (UINavComp != nullptr && !UINavComp->IsValid(bIgnoreDisabledUINavButton)) continue;

		bValid = CurrentButton->IsValid(bIgnoreDisabledUINavButton);
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

void UUINavWidget::TraverseHierarchy(UUINavWidget* UINavWidget, UUserWidget* WidgetToTraverse)
{
	//Find UINavButtons in the widget hierarchy
	TArray<UWidget*> Widgets;
	WidgetToTraverse->WidgetTree->GetAllWidgets(Widgets);
	UUINavCollection* TraversingCollection = Cast<UUINavCollection>(WidgetToTraverse);
	if (TraversingCollection != nullptr)
	{
		TraversingCollection->FirstGridIndex = UINavWidget->NavigationGrids.Num();
	}
	int GridDepth = -1;
	for (UWidget* Widget : Widgets)
	{
		if (GridDepth != -1)
		{
			FGrid& LastGrid = UINavWidget->NavigationGrids.Last();
			if (LastGrid.GridType == EGridType::Grid2D)
			{
				const UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(Widget->Slot);
				if (GridSlot != nullptr)
				{
					if (LastGrid.DimensionX < GridSlot->GetColumn() + 1)
					{
						LastGrid.DimensionX = GridSlot->GetColumn() + 1;
					}
					if (LastGrid.DimensionY < GridSlot->GetRow() + 1)
					{
						LastGrid.DimensionY = GridSlot->GetRow() + 1;
					}
				}
			}
			if (UINavWidget->GetWidgetHierarchyDepth(Widget) <= GridDepth) GridDepth = -1;
		}

		UPanelWidget* Panel = Cast<UPanelWidget>(Widget);
		if (Panel != nullptr)
		{
			UScrollBox* ScrollBox = Cast<UScrollBox>(Widget);
			if (ScrollBox != nullptr)
			{
				UINavWidget->ScrollBoxes.Add(ScrollBox);
				if (ScrollBox->GetFName().ToString().Left(4).Equals(TEXT("UIN_")))
				{
					if (!UINavWidget->bAutoAppended) UINavWidget->bAutoAppended = true;
					const bool bIsHorizontal = ScrollBox->Orientation == EOrientation::Orient_Horizontal;
					UINavWidget->GridIndexMap.Add(ScrollBox, UINavWidget->NavigationGrids.Num());
					GridDepth = UINavWidget->GetWidgetHierarchyDepth(ScrollBox);
					UINavWidget->Add1DGrid(bIsHorizontal ? EGridType::Horizontal : EGridType::Vertical, nullptr, UINavWidget->NavigationGrids.Num(), 0, FButtonNavigation(), true);
					if (TraversingCollection != nullptr)
					{
						TraversingCollection->IncrementGridCount();
					}
				}
				else
				{
					continue;
				}
			}
			
			UHorizontalBox * HorizontalBox = Cast<UHorizontalBox>(Panel);
			if (HorizontalBox != nullptr)
			{
				if (HorizontalBox->GetFName().ToString().Left(4).Equals(TEXT("UIN_")))
				{
					if (!UINavWidget->bAutoAppended) UINavWidget->bAutoAppended = true;
					UINavWidget->GridIndexMap.Add(HorizontalBox, UINavWidget->NavigationGrids.Num());
					GridDepth = UINavWidget->GetWidgetHierarchyDepth(HorizontalBox);
					UINavWidget->Add1DGrid(EGridType::Horizontal, nullptr, UINavWidget->NavigationGrids.Num(), 0, FButtonNavigation(), true);
					if (TraversingCollection != nullptr)
					{
						TraversingCollection->IncrementGridCount();
					}
				}
			}
			else
			{
				UVerticalBox* VerticalBox = Cast<UVerticalBox>(Panel);
				if (VerticalBox != nullptr)
				{
					if (VerticalBox->GetFName().ToString().Left(4).Equals(TEXT("UIN_")))
					{
						if (!UINavWidget->bAutoAppended) UINavWidget->bAutoAppended = true;
						UINavWidget->GridIndexMap.Add(VerticalBox, UINavWidget->NavigationGrids.Num());
						GridDepth = UINavWidget->GetWidgetHierarchyDepth(VerticalBox);
						UINavWidget->Add1DGrid(EGridType::Vertical, nullptr, UINavWidget->NavigationGrids.Num(), 0, FButtonNavigation(), true);
						if (TraversingCollection != nullptr)
						{
							TraversingCollection->IncrementGridCount();
						}
					}
				}
				else
				{
					UUniformGridPanel* GridPanel = Cast<UUniformGridPanel>(Panel);
					if (GridPanel != nullptr)
					{
						if (GridPanel->GetFName().ToString().Left(4).Equals(TEXT("UIN_")))
						{
							if (!UINavWidget->bAutoAppended) UINavWidget->bAutoAppended = true;
							UINavWidget->GridIndexMap.Add(GridPanel, UINavWidget->NavigationGrids.Num());
							GridDepth = UINavWidget->GetWidgetHierarchyDepth(GridPanel);
							UINavWidget->NavigationGrids.Add(FGrid(EGridType::Grid2D,
														nullptr,
														UINavWidget->NavigationGrids.Num(),
														0,
														0,
														FButtonNavigation(),
														true,
														0));
							if (TraversingCollection != nullptr)
							{
								TraversingCollection->IncrementGridCount();
							}
						}
					}
				}
			}
		}

		UUINavWidget* ChildUINavWidget = Cast<UUINavWidget>(Widget);
		if (ChildUINavWidget != nullptr)
		{
			ChildUINavWidget->AddParentToPath(UINavWidget->ChildUINavWidgets.Num());
			UINavWidget->ChildUINavWidgets.Add(ChildUINavWidget);
		}

		UUINavCollection* Collection = Cast<UUINavCollection>(Widget);
		if (Collection != nullptr)
		{
			Collection->ParentWidget = UINavWidget;
			Collection->ParentCollection = Cast<UUINavCollection>(WidgetToTraverse);
			Collection->Init(UINavWidget->UINavButtons.Num());
			UINavWidget->UINavCollections.Add(Collection);
			continue;
		}

		UUINavInputContainer* InputContainer = Cast<UUINavInputContainer>(Widget);
		if (InputContainer != nullptr)
		{
			if (UINavWidget->UINavInputContainer != nullptr)
			{
				DISPLAYERROR_STATIC(WidgetToTraverse, "You should only have 1 UINavInputContainer");
				return;
			}

			UINavWidget->InputContainerIndex = UINavWidget->UINavButtons.Num();
			UINavWidget->UINavInputContainer = InputContainer;

			InputContainer->Init(UINavWidget, UINavWidget->bAutoAppended ? UINavWidget->NavigationGrids.Num() : -1);

			if (UINavWidget->bAutoAppended && InputContainer->NumberOfInputs > 0)
			{
				UINavWidget->GridIndexMap.Add(InputContainer, UINavWidget->NavigationGrids.Num());
				const int NumInputContainerButtons = InputContainer->KeysPerInput * InputContainer->NumberOfInputs;
				UINavWidget->NavigationGrids.Add(FGrid(EGridType::Grid2D,
												UINavWidget->UINavButtons[InputContainer->FirstButtonIndex],
												UINavWidget->NavigationGrids.Num(),
												InputContainer->KeysPerInput,
												InputContainer->NumberOfInputs,
												FButtonNavigation(),
												true,
												-1));
				UINavWidget->NumberOfButtonsInGrids += NumInputContainerButtons;
			}
			continue;
		}

		UListView* ListView = Cast<UListView>(Widget);
		if (ListView != nullptr)
		{
			TArray<UObject*> ListItems = ListView->GetListItems();
			for (UObject* ListItem : ListItems)
			{
				SearchForUINavElements(UINavWidget, WidgetToTraverse, Cast<UWidget>(ListItem), TraversingCollection, GridDepth);
			}

			continue;
		}

		SearchForUINavElements(UINavWidget, WidgetToTraverse, Widget, TraversingCollection, GridDepth);
	}

	if (WidgetToTraverse->IsA<UUINavWidget>())
	{
		UINavWidget->UINavButtons.HeapSort([](const UUINavButton& Wid1, const UUINavButton& Wid2)
			{
				return Wid1.ButtonIndex < Wid2.ButtonIndex;
			});
	}

	if (UINavWidget->bAutoAppended && TraversingCollection != nullptr)
	{
		const TArray<FButtonNavigation> EdgeNavigations;
		TraversingCollection->SetupNavigation(EdgeNavigations);
	}
}

void UUINavWidget::SearchForUINavElements(UUINavWidget* UINavWidget, UUserWidget* WidgetToTraverse, UWidget* Widget, UUINavCollection* TraversingCollection, const int GridDepth)
{
	if (Widget == nullptr) return;

	UUINavButton* NewNavButton = Cast<UUINavButton>(Widget);
	if (NewNavButton == nullptr)
	{
		UUINavComponent* UIComp = Cast<UUINavComponent>(Widget);
		if (UIComp == nullptr)
		{
			UUINavComponentWrapper* UICompWrapper = Cast<UUINavComponentWrapper>(Widget);
			if (UICompWrapper != nullptr)
			{
				UIComp = UICompWrapper->GetUINavComponent();
			}
		}

		if (UIComp != nullptr)
		{
			NewNavButton = Cast<UUINavButton>(UIComp->NavButton);

			UUINavHorizontalComponent* HorizComp = Cast<UUINavHorizontalComponent>(UIComp);
			if (HorizComp != nullptr)
			{
				HorizComp->ParentWidget = UINavWidget;
			}
		}
	}

	if (NewNavButton == nullptr) return;

	if (NewNavButton->ButtonIndex == -1) NewNavButton->ButtonIndex = UINavWidget->UINavButtons.Num();

	UINavWidget->SetupUINavButtonDelegates(NewNavButton);

	NewNavButton->bAutoCollapse = NewNavButton->GetIsEnabled();
	UINavWidget->UINavButtons.Add(NewNavButton);
	UINavWidget->RevertButtonStyle(UINavWidget->UINavButtons.Num() - 1);

	if (UINavWidget->bAutoAppended && TraversingCollection != nullptr)
	{
		if (TraversingCollection->FirstButtonIndex == -1) TraversingCollection->FirstButtonIndex = NewNavButton->ButtonIndex;
		TraversingCollection->SetLastButtonIndex(NewNavButton->ButtonIndex);
	}

	if (GridDepth != -1)
	{
		FGrid& LastGrid = UINavWidget->NavigationGrids.Last();
		UINavWidget->NumberOfButtonsInGrids++;
		NewNavButton->GridIndex = UINavWidget->NavigationGrids.Num() - 1;
		if (LastGrid.FirstButton == nullptr) LastGrid.FirstButton = NewNavButton;

		switch (LastGrid.GridType)
		{
		case EGridType::Horizontal:
			NewNavButton->IndexInGrid = LastGrid.DimensionX++;
			break;
		case EGridType::Vertical:
			NewNavButton->IndexInGrid = LastGrid.DimensionY++;
			break;
		case EGridType::Grid2D:
			NewNavButton->IndexInGrid = LastGrid.NumGrid2DButtons++;
			break;
		}
	}
}

void UUINavWidget::ChangeTextColorToDefault()
{
	for (int j = 0; j < UINavButtons.Num(); j++) SwitchTextColorTo(j, TextDefaultColor);
}

void UUINavWidget::SetEnableUINavButtons(const bool bEnable, const bool bRecursive)
{
	for (UUINavButton* Button : UINavButtons)
	{
		if (Button->bAutoCollapse)
		{
			Button->SetIsEnabled(bEnable);
		}
	}

	if (!bRecursive) return;
	
	for (UUINavWidget* ChildUINavWidget : ChildUINavWidgets)
	{
		ChildUINavWidget->SetEnableUINavButtons(bEnable, bRecursive);
	}
}

void UUINavWidget::RebuildNavigation(const int NewButtonIndex)
{
	bCompletedSetup = false;
	bMovingSelector = false;
	bIgnoreMouseEvent = false;
	bReturning = false;
	ReceiveInputType = EReceiveInputType::None;
	HaltedIndex = -1;
	SelectedButtonIndex = -1;
	SelectCount = 0;
	InputBoxIndex = -1;
	NumberOfButtonsInGrids = 0;
	CollectionIndex = 0;
	FirstButtonIndex = NewButtonIndex > -1 ? NewButtonIndex : FirstButtonIndex;
	UINavInputContainer = nullptr;
	CurrentButton = nullptr;
	PromptWidgetClass = nullptr;

	for (UUINavButton* UINavButton : UINavButtons)
	{
		UINavButton->ButtonIndex = -1;
	}
	NavigationGrids.Reset();
	GridIndexMap.Reset();
	DynamicEdgeNavigations.Reset();
	UINavAnimations.Reset();
	ScrollBoxes.Reset();
	UINavButtons.Reset();
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
	
	UINavSetupWaitForTick=0;
	bShouldTickUINavSetup = true;
}

void UUINavWidget::UINavSetup()
{
	if (UINavPC == nullptr) return;

	if (OuterUINavWidget == nullptr)
	{
		UINavPC->SetActiveWidget(this);

		if (UINavPC->GetInputMode() == EInputMode::UI)
		{
			SetUserFocus(UINavPC->GetPC());
			SetKeyboardFocus();
		}
	}

	//Re-enable all buttons (bug fix)
	if (OuterUINavWidget == nullptr)
	{
		SetEnableUINavButtons(true, true);
	}

	bCompletedSetup = true;

	if (OuterUINavWidget == nullptr)
	{
		GainNavigation(nullptr);
	}

	if (PreviousNestedWidget != nullptr)
	{
		UINavPC->SetActiveNestedWidget(PreviousNestedWidget);
		PreviousNestedWidget = nullptr;
	}

	OnSetupCompleted();

	if (PromptWidgetClass != nullptr)
	{
		OnPromptDecided(PromptWidgetClass, PromptSelectedIndex);
	}
}

void UUINavWidget::AddParentToPath(const int IndexInParent)
{
	UINavWidgetPath.EmplaceAt(0, IndexInParent);

	for (UUINavWidget* ChildUINavWidget : ChildUINavWidgets)
	{
		ChildUINavWidget->AddParentToPath(IndexInParent);
	}
}

void UUINavWidget::PropagateGainNavigation(UUINavWidget* PreviousActiveWidget, UUINavWidget* NewActiveWidget, const UUINavWidget* const CommonParent)
{
	if (this == PreviousActiveWidget) return;

	if (OuterUINavWidget != nullptr && this != CommonParent)
	{
		OuterUINavWidget->PropagateGainNavigation(PreviousActiveWidget, NewActiveWidget, CommonParent);
	}

	if (this != CommonParent || this == NewActiveWidget)
	{
		GainNavigation(PreviousActiveWidget);
	}
}

void UUINavWidget::GainNavigation(UUINavWidget* PreviousActiveWidget)
{
	if (bHasNavigation) return;

	if (IsValid(UINavPC))
	{
		bForcingNavigation = bShouldForceNavigation || UINavPC->CurrentInputType != EInputType::Mouse;
	}

	const bool bShouldUpdateNavigation = bForcingNavigation || ButtonIndex == HoveredButtonIndex;
	
	if (UINavButtons.Num() > 0)
	{
		bHasNavigation = true;
		DispatchNavigation(ButtonIndex);

		if (bShouldUpdateNavigation)
		{
			OnNavigate(-1, ButtonIndex);
			CollectionNavigateTo(ButtonIndex);
		}
		
		if (IsSelectorValid())
		{
			TheSelector->SetVisibility(ESlateVisibility::HitTestInvisible);
		}

		if (bShouldUpdateNavigation)
		{
			bIgnoreMouseEvent = true;
			if (CurrentButton != nullptr)
			{
				CurrentButton->OnHovered.Broadcast();
			}
		}
		
		for (FDynamicEdgeNavigation& DynamicEdgeNavigation : DynamicEdgeNavigations)
		{
			ProcessDynamicEdgeNavigation(DynamicEdgeNavigation);
		}
	}

	const bool bPreviousWidgetIsChild = PreviousActiveWidget != nullptr ?
                                    UUINavBlueprintFunctionLibrary::ContainsArray<int>(PreviousActiveWidget->GetUINavWidgetPath(), UINavWidgetPath) :
                                    false;
	OnGainedNavigation(PreviousActiveWidget, bPreviousWidgetIsChild);
}

void UUINavWidget::OnGainedNavigation_Implementation(UUINavWidget* PreviousActiveWidget, const bool bFromChild)
{
}

void UUINavWidget::PropagateLoseNavigation(UUINavWidget* NewActiveWidget, UUINavWidget* PreviousActiveWidget, const UUINavWidget* const CommonParent)
{
	if (this == NewActiveWidget) return;

	if (this != CommonParent || this == PreviousActiveWidget)
	{
		LoseNavigation(NewActiveWidget);
	}

	if (OuterUINavWidget != nullptr && this != CommonParent)
	{
		OuterUINavWidget->PropagateLoseNavigation(NewActiveWidget, PreviousActiveWidget, CommonParent);
	}
}

void UUINavWidget::LoseNavigation(UUINavWidget* NewActiveWidget)
{
	if (!bHasNavigation) return;
	
	const bool bNewWidgetIsChild = NewActiveWidget != nullptr ?
									UUINavBlueprintFunctionLibrary::ContainsArray<int>(NewActiveWidget->GetUINavWidgetPath(), UINavWidgetPath) :
									false;

	if ((bNewWidgetIsChild && !bMaintainNavigationForChild) ||
		(!bNewWidgetIsChild && !bMaintainNavigationForParent))
	{
		DispatchNavigation(-1);
		OnNavigate(ButtonIndex, -1);
		CollectionNavigateTo(-1);

		if (IsSelectorValid())
		{
			TheSelector->SetVisibility(ESlateVisibility::Hidden);
		}

		bIgnoreMouseEvent = true;
		if (CurrentButton != nullptr)
		{
			CurrentButton->OnUnhovered.Broadcast();
		}
	}

	bHasNavigation = false;
	SelectedButtonIndex = -1;
	
	OnLostNavigation(NewActiveWidget, bNewWidgetIsChild);
}

void UUINavWidget::OnLostNavigation_Implementation(UUINavWidget* NewActiveWidget, const bool bToChild)
{
}

void UUINavWidget::ReadyForSetup_Implementation()
{

}

void UUINavWidget::NativeTick(const FGeometry & MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	if (IsSelectorValid())
	{
		if (bShouldTickUINavSetup)
		{
			if (UINavSetupWaitForTick >= 1)
			{
				UINavSetup();
				bShouldTickUINavSetup = false;
			}
			else
			{
				UINavSetupWaitForTick++;
			}
		}
		else
		{
			if (bShouldTickUpdateSelector)
			{
				if (UpdateSelectorWaitForTick >= 1)
				{
					if (MoveCurve != nullptr) BeginSelectorMovement(UpdateSelectorPrevButtonIndex, UpdateSelectorNextButtonIndex);
					else UpdateSelectorLocation(UpdateSelectorNextButtonIndex);
					bShouldTickUpdateSelector = false;
				}
				else
				{
					UpdateSelectorWaitForTick++;
				}
			}

			if (bMovingSelector)
			{
				HandleSelectorMovement(DeltaTime);
			}
		}
	}
}

void UUINavWidget::RemoveFromParent()
{
	if (OuterUINavWidget == nullptr && !bReturningToParent && !bDestroying && !GetFName().IsNone() && IsValid(this) &&
	    (ParentWidget != nullptr || (bAllowRemoveIfRoot && UINavPC != nullptr)))
	{
		ReturnToParent();
		return;
	}
	bReturningToParent = false;

	Super::RemoveFromParent();
}

FReply UUINavWidget::NativeOnKeyDown(const FGeometry & InGeometry, const FKeyEvent & InKeyEvent)
{
	if (UINavPC->GetInputMode() != EInputMode::UI)
	{
		if (ReceiveInputType == EReceiveInputType::None)
		{
			//Allow fullscreen by pressing F11 or Alt+Enter
			if (GEngine->GameViewport->TryToggleFullscreenOnInputKey(InKeyEvent.GetKey(), IE_Pressed))
			{
				return FReply::Handled();
			}

			if (UINavPC->OnKeyPressed(InKeyEvent.GetKey()).IsEventHandled())
			{
				return FReply::Handled();
			}
		}
		else
		{
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UUINavWidget::NativeOnKeyUp(const FGeometry & InGeometry, const FKeyEvent & InKeyEvent)
{
	if (UINavPC->GetInputMode() != EInputMode::UI)
	{
		if (IsRebindingInput())
		{
			const FKey Key = InKeyEvent.GetKey();
			ProcessKeybind(Key);
			return FReply::Handled();
		}
		else
		{
			if (UINavPC->OnKeyReleased(InKeyEvent.GetKey()).IsEventHandled())
			{
				return FReply::Handled();
			}
		}
	}

	return Super::NativeOnKeyUp(InGeometry, InKeyEvent);
}

FReply UUINavWidget::NativeOnMouseWheel(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent)
{
	if (IsRebindingInput())
	{
		const FKey PressedMouseKey = InMouseEvent.GetWheelDelta() > 0.f ? EKeys::MouseScrollUp : EKeys::MouseScrollDown;
		ProcessKeybind(PressedMouseKey);
		return FReply::Handled();
	}

	return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
}

FReply UUINavWidget::NativeOnMouseButtonDown(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent)
{
	if (IsRebindingInput())
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
		if (UINavPC->OnKeyPressed(InMouseEvent.GetEffectingButton()).IsEventHandled())
		{
			return FReply::Handled();
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UUINavWidget::NativeOnMouseButtonUp(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent)
{
	if (!IsRebindingInput() && InMouseEvent.GetEffectingButton().IsMouseButton())
	{
		UINavPC->OnKeyReleased(InMouseEvent.GetEffectingButton());
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UUINavWidget::HandleSelectorMovement(const float DeltaTime)
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
			else if (HaltedIndex == RETURN_INDEX)
			{
				CollectionOnReturn();
				OnReturn();
			}
			else NavigateTo(HaltedIndex);

			HaltedIndex = -1;
		}
		return;
	}

	TheSelector->SetRenderTranslation(SelectorOrigin + Distance*MoveCurve->GetFloatValue(MovementCounter));
}

void UUINavWidget::AddUINavButton(UUINavButton* NewButton, const int TargetGridIndex, int IndexInGrid)
{
	if (!IsValid(NewButton) || !NavigationGrids.IsValidIndex(TargetGridIndex)) return;

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

	if (UINavButtons.Num() == 1)
	{
		ButtonIndex = 0;
		CurrentButton = UINavButtons[0];
		DispatchNavigation(ButtonIndex);
		OnNavigate(-1, ButtonIndex);
	}
}

void UUINavWidget::AddUINavButtons(TArray<UUINavButton*> NewButtons, const int TargetGridIndex, int IndexInGrid)
{
	if (!NavigationGrids.IsValidIndex(TargetGridIndex)) return;
	if (UINavAnimations.Num() > 0)
	{
		DISPLAYERROR("Runtime manipulation not supported with navigation using animations.");
	}
	
	if (IndexInGrid >= NavigationGrids[TargetGridIndex].GetDimension()) IndexInGrid = -1;
	const bool bIncrementIndexInGrid = IndexInGrid > -1;

	for (UUINavButton* NewButton : NewButtons)
	{
		AddUINavButton(NewButton, TargetGridIndex, IndexInGrid);
		if (bIncrementIndexInGrid) IndexInGrid++;
	}
}

void UUINavWidget::AddUINavComponent(UUINavComponent * NewComponent, const int TargetGridIndex, int IndexInGrid)
{
	AddUINavButton(NewComponent->NavButton, TargetGridIndex, IndexInGrid);
}

void UUINavWidget::AddUINavComponents(TArray<UUINavComponent*> NewComponents, const int TargetGridIndex, int IndexInGrid)
{
	if (!NavigationGrids.IsValidIndex(TargetGridIndex)) return;
	if (UINavAnimations.Num() > 0)
	{
		DISPLAYERROR("Runtime manipulation not supported with navigation using animations.");
	}
	
	if (IndexInGrid >= NavigationGrids[TargetGridIndex].GetDimension()) IndexInGrid = -1;
	const bool bIncrementIndexInGrid = IndexInGrid > -1;

	for (const UUINavComponent* NewComponent : NewComponents)
	{
		AddUINavButton(NewComponent->NavButton, TargetGridIndex, IndexInGrid);
		if (bIncrementIndexInGrid) IndexInGrid++;
	}
}

void UUINavWidget::DeleteUINavElement(const int Index, const bool bAutoNavigate)
{
	if (!UINavButtons.IsValidIndex(Index)) return;

	if (Index == ButtonIndex)
	{
		bool bValid = false;

		UUINavButton* Temp = CurrentButton;
		while (!bValid && IsValid(Temp))
		{
			int NewIndex = Temp->ButtonIndex + 1;
			if (NewIndex >= UINavButtons.Num()) NewIndex = 0;

			Temp = UINavButtons[NewIndex];
			if (NewIndex == FirstButtonIndex) break;

			const bool bIgnoreDisabledUINavButton = GetDefault<UUINavSettings>()->bIgnoreDisabledUINavButton;
			UUINavComponent* UINavComp = GetUINavComponentAtIndex(NewIndex);
			if (UINavComp != nullptr && !UINavComp->IsValid(bIgnoreDisabledUINavButton)) continue;

			bValid = Temp->IsValid(bIgnoreDisabledUINavButton);
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

	DeleteButtonEdgeNavigationRefs(Button);
}

void UUINavWidget::DeleteUINavElementFromGrid(const int GridIndex, int IndexInGrid, const bool bAutoNavigate)
{
	if (!NavigationGrids.IsValidIndex(GridIndex))
	{
		DISPLAYERROR("Invalid GridIndex");
		return;
	}
	const FGrid& TargetGrid = NavigationGrids[GridIndex];
	IndexInGrid = IndexInGrid >= 0 && IndexInGrid < TargetGrid.GetDimension() ? IndexInGrid : TargetGrid.GetDimension() - 1;

	DeleteUINavElement(TargetGrid.FirstButton->ButtonIndex + IndexInGrid, bAutoNavigate);
}

void UUINavWidget::IncrementGrid(UUINavButton* NewButton, FGrid & TargetGrid, int& IndexInGrid)
{
	if (IndexInGrid == 0)
	{
		TargetGrid.FirstButton = NewButton;
		NewButton->IndexInGrid = 0;
		NewButton->ButtonIndex = GetGridStartingIndex(TargetGrid.GridIndex);
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

	UpdateCollectionLastIndex(TargetGrid.GridIndex, true);
}

void UUINavWidget::DecrementGrid(FGrid & TargetGrid, const int IndexInGrid)
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

void UUINavWidget::IncrementUINavButtonIndices(const int StartingIndex, const int GridIndex)
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

void UUINavWidget::DecrementUINavButtonIndices(const int StartingIndex, const int GridIndex)
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

void UUINavWidget::MoveUINavElementToGrid(const int Index, const int TargetGridIndex, int IndexInGrid)
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

	const int OldGridIndex = Button->GridIndex;
	const int OldIndexInGrid = Button->IndexInGrid;

	const int From = Index;
	const int To = GetGridStartingIndex(TargetGridIndex) + IndexInGrid;
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

void UUINavWidget::MoveUINavElementToGrid2(const int FromGridIndex, int FromIndexInGrid, const int TargetGridIndex, const int TargetIndexInGrid)
{
	if (!NavigationGrids.IsValidIndex(FromGridIndex)) return;

	const FGrid& TargetGrid = NavigationGrids[FromGridIndex];
	if (TargetGrid.FirstButton == nullptr) return;

	if (FromIndexInGrid <= -1 || FromIndexInGrid >= TargetGrid.GetDimension())
	{
		FromIndexInGrid = TargetGrid.GetDimension() - 1;
	}

	MoveUINavElementToGrid(TargetGrid.FirstButton->ButtonIndex + FromIndexInGrid, TargetGridIndex, TargetIndexInGrid);
}

void UUINavWidget::UpdateArrays(const int From, const int To, const int OldGridIndex, const int OldIndexInGrid)
{
	UpdateButtonArray(From, To, OldGridIndex, OldIndexInGrid);
}

void UUINavWidget::UpdateButtonArray(const int From, int To, const int OldGridIndex, const int OldIndexInGrid)
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

		const int TargetGridDimension = NavigationGrids[TempButton->GridIndex].GetDimension();
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

		const int TargetGridDimension = NavigationGrids[OldGridIndex].GetDimension();
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

void UUINavWidget::UpdateCollectionLastIndex(const int GridIndex, const bool bAdded)
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

void UUINavWidget::ReplaceButtonInNavigationGrid(UUINavButton * ButtonToReplace, const int GridIndex, const int IndexInGrid)
{
	UUINavButton* NewButton = NavigationGrids[GridIndex].GetDimension() > IndexInGrid ? GetButtonAtGridIndex(GridIndex, IndexInGrid) : nullptr;
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
	if (IsSelectorValid())
	{
		UpdateSelectorWaitForTick = 0;
		UpdateSelectorPrevButtonIndex = ButtonIndex;
		UpdateSelectorNextButtonIndex = NewCurrentButton->ButtonIndex;
		bShouldTickUpdateSelector = true;
	}

	ButtonIndex = NewCurrentButton->ButtonIndex;
	
	for (UScrollBox* ScrollBox : ScrollBoxes)
	{
			ScrollBox->ScrollWidgetIntoView(NewCurrentButton, bAnimateScrollBoxes);
	}
}

void UUINavWidget::ClearGrid(const int GridIndex, const bool bAutoNavigate)
{
	if (!NavigationGrids.IsValidIndex(GridIndex)) return;

	FGrid& Grid = NavigationGrids[GridIndex];
	if (Grid.FirstButton == nullptr) return;

	const int FirstIndex = Grid.FirstButton->ButtonIndex;
	const int LastIndex = FirstIndex + Grid.GetDimension() - 1;
	const int Difference = LastIndex - FirstIndex + 1;

	const bool bShouldNavigate = bAutoNavigate && ButtonIndex >= FirstIndex && ButtonIndex <= LastIndex;
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

			const bool bIgnoreDisabledUINavButton = GetDefault<UUINavSettings>()->bIgnoreDisabledUINavButton;
			UUINavComponent* UINavComp = GetUINavComponentAtIndex(NewIndex);
			if (UINavComp != nullptr && !UINavComp->IsValid(bIgnoreDisabledUINavButton)) continue;

			bValid = NextButton->IsValid(bIgnoreDisabledUINavButton);
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

	const int NumButtons = UINavButtons.Num();
	for (int i = FirstIndex; i < NumButtons; ++i)
	{
		if (i <= LastIndex)
		{
			UINavButtons[FirstIndex]->ButtonIndex = -1;
			UINavButtons[FirstIndex]->GridIndex = -1;
			UINavButtons[FirstIndex]->IndexInGrid = -1;
			UINavButtons.RemoveAt(FirstIndex);
		}
		else
		{
			UUINavButton* Button = UINavButtons[i - Difference];
			Button->ButtonIndex -= Difference;
		}
	}

	Grid.FirstButton = nullptr;
	if (Grid.GridType != EGridType::Grid2D)	Grid.DimensionX = 0;
	Grid.DimensionY = 0;
	Grid.NumGrid2DButtons = 0;

	if (bShouldNavigate && CurrentButton != nullptr)
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

void UUINavWidget::DeleteGridEdgeNavigationRefs(const int GridIndex)
{
	for (FGrid& Grid : NavigationGrids)
	{
		if (Grid.GridIndex == GridIndex) continue;

		Grid.RemoveGridFromEdgeNavigation(GridIndex);
	}
}

void UUINavWidget::AppendNavigationGrid1D(const EGridType GridType, int Dimension, const FButtonNavigation EdgeNavigation, const bool bWrap)
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

	if (Dimension > 0)
	{
		const bool bFoundContainer = InputContainerIndex >= NumberOfButtonsInGrids && InputContainerIndex <= NumberOfButtonsInGrids + Dimension - 1;
		if (bFoundContainer)
		{
			DISPLAYERROR("In order to append InputContainer navigation, use Append Navigation Grid 2D");
			return;
		}
	}

	Add1DGrid(GridType,
			  UINavButtons.Num() > 0 && Dimension > 0 ? (UINavButtons.Num() > NumberOfButtonsInGrids ? UINavButtons[NumberOfButtonsInGrids] : nullptr) : nullptr,
			  NavigationGrids.Num(),
			  Dimension,
			  EdgeNavigation,
			  bWrap);

	const int GridIndex = NavigationGrids.Num() - 1;
	for (int i = 0; i < Dimension; i++)
	{
		if (NumberOfButtonsInGrids + i >= UINavButtons.Num()) break;

		UINavButtons[NumberOfButtonsInGrids + i]->GridIndex = GridIndex;
		UINavButtons[NumberOfButtonsInGrids + i]->IndexInGrid = i;
	}

	NumberOfButtonsInGrids += Dimension;
}

void UUINavWidget::AppendNavigationGrid2D(const int DimensionX, int DimensionY, const FButtonNavigation EdgeNavigation, const bool bWrap, const int ButtonsInGrid)
{
	if (DimensionX <= 0)
	{
		DISPLAYERROR("AppendNavigationGrid2D Dimension X should be greater than 0");
		return;
	}
	if (DimensionY < 0)
	{
		DISPLAYERROR("AppendNavigationGrid2D Dimension Y should be at least 0");
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
	
	if (ButtonsInGrid >= 0)
	{
		const int DesiredY = (ButtonsInGrid / DimensionX) + 1;
		if (DimensionY > DesiredY)
		{
			DimensionY = DesiredY;
		}
	}

	const FGrid NewGrid = FGrid(EGridType::Grid2D, 
						  ButtonsInGrid != 0 ? UINavButtons[NumberOfButtonsInGrids] : nullptr,
						  NavigationGrids.Num(), 
						  DimensionX, 
						  DimensionY, 
						  EdgeNavigation, 
						  bWrap,
						  ButtonsInGrid);

	NavigationGrids.Add(NewGrid);

	const int GridIndex = NavigationGrids.Num() - 1;
	const int Iterations = NewGrid.NumGrid2DButtons;
	for (int i = 0; i < Iterations; i++)
	{
		if (NumberOfButtonsInGrids + i >= UINavButtons.Num()) break;

		UINavButtons[NumberOfButtonsInGrids + i]->GridIndex = GridIndex;
		UINavButtons[NumberOfButtonsInGrids + i]->IndexInGrid = i;
	}

	NumberOfButtonsInGrids = NumberOfButtonsInGrids + Iterations;
}

void UUINavWidget::AddEdgeNavigation(const int GridIndex1, const int TargetIndexInGrid1, const int GridIndex2, const int TargetIndexInGrid2, const ENavigationDirection Direction, const bool bTwoWayConnection)
{
	if (!NavigationGrids.IsValidIndex(GridIndex1)) return;
	if (!NavigationGrids.IsValidIndex(GridIndex2)) return;
	if (GridIndex1 == GridIndex2) return;

	FGrid& Grid1 = NavigationGrids[GridIndex1];
	FGrid& Grid2 = NavigationGrids[GridIndex2];

	UUINavButton* Grid1Button = GetButtonAtGridIndex(GridIndex1, TargetIndexInGrid1);
	if (Grid1Button == nullptr) return;

	UUINavButton* Grid2Button = GetButtonAtGridIndex(GridIndex2, TargetIndexInGrid2);
	if (Grid2Button == nullptr) return;

	if (Direction == ENavigationDirection::Left)
	{
		Grid1.EdgeNavigation.LeftButton = Grid2Button;
		if (bTwoWayConnection) Grid2.EdgeNavigation.RightButton = Grid1Button;
	}
	else if (Direction == ENavigationDirection::Right)
	{
		Grid1.EdgeNavigation.RightButton = Grid2Button;
		if (bTwoWayConnection) Grid2.EdgeNavigation.LeftButton = Grid1Button;
	}
	else if (Direction == ENavigationDirection::Up)
	{
		Grid1.EdgeNavigation.UpButton = Grid2Button;
		if (bTwoWayConnection) Grid2.EdgeNavigation.DownButton = Grid1Button;
	}
	else if (Direction == ENavigationDirection::Down)
	{
		Grid1.EdgeNavigation.DownButton = Grid2Button;
		if (bTwoWayConnection) Grid2.EdgeNavigation.UpButton = Grid1Button;
	}
}

void UUINavWidget::AddSingleGridDynamicEdgeNavigation(const int GridIndex, const int TargetGridIndex, TArray<int> TargetButtonIndices, const ENavigationEvent Event, const ENavigationDirection Direction, const bool bTwoWayConnection)
{
	if (!NavigationGrids.IsValidIndex(GridIndex) || !NavigationGrids.IsValidIndex(TargetGridIndex))
	{
		DISPLAYERROR("Invalid GridIndex or TargetGridIndex in AddSingleGridDynamicEdgeNavigation function!");
		return;
	}

	if (Direction == ENavigationDirection::None)
	{
		DISPLAYERROR("Invalid Direction in AddMultiGridDynamicEdgeNavigation function!");
		return;
	}

	const FGrid& CurrentGrid = NavigationGrids[GridIndex];
	const FGrid& TargetGrid = NavigationGrids[TargetGridIndex];

	const bool bHorizontal = Direction == ENavigationDirection::Left || Direction == ENavigationDirection::Right;

	if (TargetGrid.GridType != EGridType::Grid2D)
	{
		if ((bHorizontal && (CurrentGrid.GridType == EGridType::Horizontal || TargetGrid.GridType == EGridType::Horizontal)) ||
			(!bHorizontal && CurrentGrid.GridType == EGridType::Vertical))
		{
			DISPLAYERROR("Unnecessary use of AddSingleGridDynamicEdgeNavigation function.");
			return;
		}
	}
	else if (!bHorizontal && CurrentGrid.DimensionX == 1)
	{
		DISPLAYERROR("Unnecessary use of AddSingleGridDynamicEdgeNavigation function.");
		return;
	}

	if (TargetButtonIndices.Num() == 0)
	{
		if (TargetGrid.GridType != EGridType::Grid2D)
		{
			for (int i = 0; i < TargetGrid.GetDimension(); ++i)
			{
				TargetButtonIndices.Add(i);
			}
		}
		else
		{
			if (bHorizontal)
			{
				const int Grid2ButtonsNum = ((TargetGrid.NumGrid2DButtons - 1) / TargetGrid.DimensionX) + 1;

				for (int i = 0; i < Grid2ButtonsNum; ++i)
				{
					if (Direction == ENavigationDirection::Right) TargetButtonIndices.Add(i * TargetGrid.DimensionX);
					else TargetButtonIndices.Add((i + 1) * TargetGrid.DimensionX - 1);

					if (TargetButtonIndices.Last() >= TargetGrid.NumGrid2DButtons)
					{
						if (Direction == ENavigationDirection::Right) TargetButtonIndices.Last() = TargetGrid.DimensionX * (TargetGrid.DimensionY - 1);
						else TargetButtonIndices.Last() = TargetGrid.NumGrid2DButtons - 1;
					}
				}
			}
			else
			{
				const int Grid2ButtonsNum = TargetGrid.DimensionX;

				for (int i = 0; i < Grid2ButtonsNum; ++i)
				{
					if (Direction == ENavigationDirection::Down) TargetButtonIndices.Add(i);
					else
					{
						if ((TargetGrid.DimensionX * (TargetGrid.DimensionY - 1) + i) < TargetGrid.NumGrid2DButtons)
						{
							TargetButtonIndices.Add(TargetGrid.DimensionX * (TargetGrid.DimensionY - 1) + i);
						}
						else
						{
							TargetButtonIndices.Add(TargetGrid.DimensionX * (TargetGrid.DimensionY - 2) + i);
						}
					}

					if (TargetButtonIndices.Last() >= TargetGrid.NumGrid2DButtons)
					{
						if (Direction == ENavigationDirection::Right) TargetButtonIndices.Last() = TargetGrid.DimensionX * (TargetGrid.DimensionY - 1);
						else TargetButtonIndices.Last() = TargetGrid.NumGrid2DButtons - 1;
					}
				}
			}
		}
	}

	DynamicEdgeNavigations.Add(FDynamicEdgeNavigation(GridIndex, TargetGridIndex, TargetButtonIndices, Event, Direction, bTwoWayConnection));
}

void UUINavWidget::AddMultiGridDynamicEdgeNavigation(const int GridIndex, TArray<FGridButton> TargetButtons, const ENavigationEvent Event, const ENavigationDirection Direction, const bool bTwoWayConnection)
{
	if (!NavigationGrids.IsValidIndex(GridIndex))
	{
		DISPLAYERROR("Invalid GridIndex in AddMultiGridDynamicEdgeNavigation function!");
		return;
	}

	if (TargetButtons.Num() < 2)
	{
		DISPLAYERROR("Not enough TargetButtons in AddMultiGridDynamicEdgeNavigation function!");
		return;
	}

	if (Direction == ENavigationDirection::None)
	{
		DISPLAYERROR("Invalid Direction in AddMultiGridDynamicEdgeNavigation function!");
		return;
	}

	const FGrid& CurrentGrid = NavigationGrids[GridIndex];
	const bool bIsHorizontal = Direction == ENavigationDirection::Left || Direction == ENavigationDirection::Right;
	if (bIsHorizontal && CurrentGrid.GridType == EGridType::Horizontal)
	{
		DISPLAYERROR("Unnecessary use of AddMultiGridDynamicEdgeNavigation function.");
		return;
	}
	else if (!bIsHorizontal &&
		(CurrentGrid.GridType == EGridType::Vertical || (CurrentGrid.GridType == EGridType::Grid2D && CurrentGrid.DimensionX == 1)))
	{
		DISPLAYERROR("Unnecessary use of AddMultiGridDynamicEdgeNavigation function.");
		return;
	}

	for (int i = 0; i < TargetButtons.Num(); ++i)
	{
		const FGridButton& GridButton = TargetButtons[i];
		if (!NavigationGrids.IsValidIndex(GridButton.GridIndex))
		{
			DISPLAYERROR("Invalid TargetButton GridIndex in AddMultiGridDynamicEdgeNavigation function!");
			return;
		}

		int IndexInGrid = CurrentGrid.GridType != EGridType::Grid2D ? i : i * CurrentGrid.DimensionX;
		if (IndexInGrid >= CurrentGrid.GetDimension())
		{
			IndexInGrid = CurrentGrid.GridType != EGridType::Grid2D ? CurrentGrid.GetDimension() - 1 : CurrentGrid.NumGrid2DButtons / CurrentGrid.DimensionX;
		}

		UUINavButton* TargetButton = GetButtonAtGridIndex(GridIndex, IndexInGrid);
		UpdateEdgeNavigation(GridButton.GridIndex, TargetButton, Direction, true);

		if (GetButtonAtGridIndex(GridButton.GridIndex, GridButton.IndexInGrid) == nullptr)
		{
			DISPLAYERROR("Invalid TargetButton IndexInGrid in AddMultiGridDynamicEdgeNavigation function!");
			return;
		}
	}

	DynamicEdgeNavigations.Add(FDynamicEdgeNavigation(GridIndex, TargetButtons, Event, Direction, bTwoWayConnection));
}

void UUINavWidget::UpdateDynamicEdgeNavigations(const int UpdatedGridIndex)
{
	for (FDynamicEdgeNavigation& DynamicEdgeNavigation : DynamicEdgeNavigations)
	{
		if (DynamicEdgeNavigation.TargetGridIndex == UpdatedGridIndex)
		{
			const FGrid& CurrentGrid = NavigationGrids[DynamicEdgeNavigation.GridIndex];
			FGrid& TargetGrid = NavigationGrids[DynamicEdgeNavigation.TargetGridIndex];
			const bool bHorizontal = DynamicEdgeNavigation.Direction == ENavigationDirection::Left || DynamicEdgeNavigation.Direction == ENavigationDirection::Right;

			if (TargetGrid.GridType != EGridType::Grid2D)
			{
				if ((bHorizontal && (CurrentGrid.GridType == EGridType::Horizontal || TargetGrid.GridType == EGridType::Horizontal)) ||
					(!bHorizontal && CurrentGrid.GridType == EGridType::Vertical))
				{
					continue;
				}

				DynamicEdgeNavigation.TargetButtonIndices.Add(TargetGrid.GetDimension() - 1);
			}
			else
			{
				if (bHorizontal)
				{
					const int Grid2ButtonsNum = ((TargetGrid.NumGrid2DButtons - 1) / TargetGrid.DimensionX) + 1;

					if (DynamicEdgeNavigation.Direction == ENavigationDirection::Right) DynamicEdgeNavigation.TargetButtonIndices.Add((Grid2ButtonsNum - 1) * TargetGrid.DimensionX);
					else DynamicEdgeNavigation.TargetButtonIndices.Add(((Grid2ButtonsNum - 1) + 1) * TargetGrid.DimensionX - 1);

					if (DynamicEdgeNavigation.TargetButtonIndices.Last() >= TargetGrid.NumGrid2DButtons)
					{
						if (DynamicEdgeNavigation.Direction == ENavigationDirection::Right) DynamicEdgeNavigation.TargetButtonIndices.Last() = TargetGrid.DimensionX * (TargetGrid.DimensionY - 1);
						else DynamicEdgeNavigation.TargetButtonIndices.Last() = TargetGrid.NumGrid2DButtons - 1;
					}
				}
				else
				{
					if (CurrentGrid.DimensionX == 1)
					{
						continue;
					}

					const int Grid2ButtonsNum = TargetGrid.DimensionX;

					if (DynamicEdgeNavigation.Direction == ENavigationDirection::Down) DynamicEdgeNavigation.TargetButtonIndices.Add((Grid2ButtonsNum - 1));
					else
					{
						if ((TargetGrid.DimensionX * (TargetGrid.DimensionY - 1) + (Grid2ButtonsNum - 1)) < TargetGrid.NumGrid2DButtons)
						{
							DynamicEdgeNavigation.TargetButtonIndices.Add(TargetGrid.DimensionX * (TargetGrid.DimensionY - 1) + (Grid2ButtonsNum - 1));
						}
						else
						{
							DynamicEdgeNavigation.TargetButtonIndices.Add(TargetGrid.DimensionX * (TargetGrid.DimensionY - 2) + (Grid2ButtonsNum - 1));
						}
					}

					if (DynamicEdgeNavigation.TargetButtonIndices.Last() >= TargetGrid.NumGrid2DButtons)
					{
						if (DynamicEdgeNavigation.Direction == ENavigationDirection::Right) DynamicEdgeNavigation.TargetButtonIndices.Last() = TargetGrid.DimensionX * (TargetGrid.DimensionY - 1);
						else DynamicEdgeNavigation.TargetButtonIndices.Last() = TargetGrid.NumGrid2DButtons - 1;
					}
				}
			}
		}
	}
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

void UUINavWidget::SetEdgeNavigation(const int GridIndex, const FButtonNavigation NewEdgeNavigation)
{
	if (!NavigationGrids.IsValidIndex(GridIndex))
	{
		return;
	}
	NavigationGrids[GridIndex].SetEdgeNavigation(NewEdgeNavigation);
}

void UUINavWidget::SetBulkEdgeNavigation(const TArray<int>& GridIndices, const FButtonNavigation NewEdgeNavigation)
{
	for (const int& GridIndex : GridIndices)
	{
		SetEdgeNavigation(GridIndex, NewEdgeNavigation);
	}
}

void UUINavWidget::SetEdgeNavigationByButton(const int GridIndex, const FButtonNavigation NewEdgeNavigation)
{
	if (!NavigationGrids.IsValidIndex(GridIndex))
	{
		return;
	}
	NavigationGrids[GridIndex].SetEdgeNavigationByButton(NewEdgeNavigation);
}

void UUINavWidget::SetBulkEdgeNavigationByButton(const TArray<int>& GridIndices, const FButtonNavigation NewEdgeNavigation)
{
	for (const int& GridIndex : GridIndices)
	{
		SetEdgeNavigationByButton(GridIndex, NewEdgeNavigation);
	}
}

void UUINavWidget::SetWrap(const int GridIndex, const bool bWrap)
{
	if (!NavigationGrids.IsValidIndex(GridIndex))
	{
		return;
	}
	NavigationGrids[GridIndex].bWrap = bWrap;
}

void UUINavWidget::Add1DGrid(const EGridType GridType, UUINavButton * FirstButton, const int StartingIndex, const int Dimension, const FButtonNavigation EdgeNavigation, const bool bWrap)
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

void UUINavWidget::UpdateSelectorLocation(const int Index)
{
	if (TheSelector == nullptr || UINavButtons.Num() == 0) return;
	TheSelector->SetRenderTranslation(GetButtonLocation(Index));
}

FVector2D UUINavWidget::GetButtonLocation(const int Index)
{
	const FGeometry Geom = UINavButtons[Index]->GetCachedGeometry();
	const FVector2D LocalSize = Geom.GetLocalSize();
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

void UUINavWidget::ExecuteAnimations(const int From, const int To)
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

	if (UINavAnimations.IsValidIndex(To))
	{
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
}

void UUINavWidget::UpdateTextColor(const int Index)
{
	SwitchTextColorTo(ButtonIndex, TextDefaultColor);
	SwitchTextColorTo(Index, TextNavigatedColor);
}

void UUINavWidget::SwitchTextColorTo(const int Index, FLinearColor Color)
{
    if (!UINavButtons.IsValidIndex(Index)) return;
    
	UTextBlock* NewText = nullptr;
	UUINavComponent* UINavComponent = UINavButtons[Index]->NavComp;
	if (UINavComponent != nullptr)
	{
		NewText = UINavComponent->NavText;
	}
	else if (Index != -1)
	{
		NewText = Cast<UTextBlock>(UINavButtons[Index]->GetChildAt(0));
	}
	
	if (NewText == nullptr) return;
	NewText->SetColorAndOpacity(Color);
}

void UUINavWidget::UpdateHoveredButtonStates(const int Index)
{
	//Update new button state
	SwitchButtonStyle(EButtonStyle::Hovered, Index);

	if (ButtonIndex == Index) return;

	//Update previous button state
	SwitchButtonStyle(EButtonStyle::Normal, ButtonIndex);
}

void UUINavWidget::SwitchButtonStyle(const EButtonStyle NewStyle, const int Index, const bool bRevertStyle)
{
	if (!UINavButtons.IsValidIndex(Index)) return;
	
	UUINavButton* TheButton = UINavButtons[Index];
	const bool bWasForcePressed = TheButton->ForcedStyle == EButtonStyle::Pressed;

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

void UUINavWidget::RevertButtonStyle(const int Index)
{
    if (!UINavButtons.IsValidIndex(Index)) return;
	
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
	const FButtonStyle Style = TargetButton->WidgetStyle;
	UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(TargetButton->Slot);
	const FMargin PressedPadding = Style.PressedPadding - Style.NormalPadding;
	if (OverlaySlot != nullptr)
	{
		OverlaySlot->SetPadding(OverlaySlot->GetPadding() == PressedPadding ? FMargin(0.0f) : PressedPadding);
	}
}

void UUINavWidget::SetSelectorScale(FVector2D NewScale)
{
	if (TheSelector == nullptr) return;
	TheSelector->SetRenderScale(NewScale);
}

void UUINavWidget::SetSelectorVisibility(const bool bVisible)
{
	if (TheSelector == nullptr) return;
	const ESlateVisibility Vis = bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
	TheSelector->SetVisibility(Vis);
}

bool UUINavWidget::IsSelectorVisible()
{
	if (TheSelector == nullptr) return false;
	return TheSelector->GetVisibility() == ESlateVisibility::HitTestInvisible;
}

void UUINavWidget::OnNavigate_Implementation(const int From, const int To)
{

}

void UUINavWidget::OnNavigatedDirection_Implementation(const ENavigationDirection Direction)
{

}

void UUINavWidget::OnSelect_Implementation(const int Index)
{

}

void UUINavWidget::OnStartSelect_Implementation(const int Index)
{

}

void UUINavWidget::OnStopSelect_Implementation(const int Index)
{

}

void UUINavWidget::NavigateTo(const int Index, const bool bHoverEvent, const bool bBypassChecks)
{
	if (!bBypassChecks && (Index >= UINavButtons.Num() || (Index == ButtonIndex && bForcingNavigation))) return;

	DispatchNavigation(Index);
	const int OldButtonIndex = ButtonIndex;
	const UUINavButton* OldButton = CurrentButton;
	ButtonIndex = Index;
	CurrentButton = UINavButtons[ButtonIndex];
	OnNavigate(OldButtonIndex, ButtonIndex);
	CollectionNavigateTo(ButtonIndex);

	if (!bHoverEvent && IsValid(OldButton))
	{
		bIgnoreMouseEvent = true;
		OldButton->OnUnhovered.Broadcast();
	}
	if (!bHoverEvent)
	{
		bIgnoreMouseEvent = true;
		CurrentButton->OnHovered.Broadcast();
	}

	for (FDynamicEdgeNavigation& DynamicEdgeNavigation : DynamicEdgeNavigations)
	{
		if (DynamicEdgeNavigation.Event == ENavigationEvent::OnNavigate)
		{
			ProcessDynamicEdgeNavigation(DynamicEdgeNavigation);
		}
	}
}

void UUINavWidget::NavigateToGrid(const int GridIndex, const int IndexInGrid)
{
	UUINavButton* TargetButton = GetButtonAtGridIndex(GridIndex, IndexInGrid);

	if (TargetButton == nullptr) return;

	NavigateTo(TargetButton->ButtonIndex);
}

void UUINavWidget::CollectionNavigateTo(const int Index)
{
	bool bFoundFrom = false;
	bool bFoundTo = false;
	for (UUINavCollection* Collection : UINavCollections)
	{
		const int CollectionFromIndex = Index != ButtonIndex ? GetCollectionFirstButtonIndex(Collection, ButtonIndex) : -1;
		const int CollectionToIndex = GetCollectionFirstButtonIndex(Collection, Index);

		const bool bValidFrom = CollectionFromIndex != -1;
		const bool bValidTo = CollectionToIndex != -1;
		if (bValidFrom || bValidTo)
		{
			if (!bFoundFrom) bFoundFrom = bValidFrom;
			if (!bFoundTo) bFoundTo = bValidTo;

			Collection->OnNavigate(Index != ButtonIndex ? ButtonIndex : -1, Index, CollectionFromIndex, CollectionToIndex);
		}
	}
}

void UUINavWidget::CallCustomInput(const FName ActionName, uint8* Buffer)
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

	if (CurrentButton != nullptr && CurrentButton->NavComp != nullptr)
	{
		CurrentButton->NavComp->CallCustomInput(ActionName, Buffer);
	}

	for (UUINavCollection* Collection : UINavCollections)
	{
		Collection->CallCustomInput(ActionName, Buffer);
	}
}

void UUINavWidget::OnPromptDecided(const TSubclassOf<UUINavPromptWidget> PromptClass, const int Index)
{
	PromptWidgetClass = nullptr;

	FString ClassString = PromptClass->GetFName().ToString();
	ClassString.RemoveAt(ClassString.Len() - 2, 2);
	const FName EventName = FName(*(ClassString.Append(TEXT("_Decided"))));
	UFunction* CustomFunction = FindFunction(EventName);
	if (CustomFunction != nullptr)
	{
		if (CustomFunction->ParmsSize == sizeof(int))
		{
			uint8* Buffer = static_cast<uint8*>(FMemory_Alloca(sizeof(int)));
			FMemory::Memcpy(Buffer, &Index, sizeof(int));
			ProcessEvent(CustomFunction, Buffer);
		}
		else
		{
			DISPLAYERROR(FString::Printf(TEXT("%s Prompt Event could not be found!"), *EventName.ToString()));
		}
	}
}

void UUINavWidget::ProcessDynamicEdgeNavigation(FDynamicEdgeNavigation& DynamicEdgeNavigation)
{
	const int CurrentGridIndex = GetButtonGridIndex(ButtonIndex);
	const int CurrentIndexInGrid = GetButtonIndexInGrid(ButtonIndex);
	int AdaptedCurrentIndexInGrid = CurrentIndexInGrid;
	const ENavigationDirection Dir = DynamicEdgeNavigation.Direction;
	const bool bHorizontal = Dir == ENavigationDirection::Left || Dir == ENavigationDirection::Right;

	const FGrid& CurrentGrid = NavigationGrids[CurrentGridIndex];
	if (CurrentGrid.GridType == EGridType::Grid2D)
	{
		int XCoord, YCoord;
		GetButtonCoordinatesInGrid2D(ButtonIndex, XCoord, YCoord);
		if (XCoord != 0 &&
			XCoord != CurrentGrid.DimensionX - 1 &&
			YCoord != 0 &&
			YCoord != CurrentGrid.DimensionY - 1 &&
			CurrentIndexInGrid + CurrentGrid.DimensionX < CurrentGrid.NumGrid2DButtons)
		{
			return;
		}
		
		AdaptedCurrentIndexInGrid = bHorizontal ? YCoord : XCoord;
	}

	// If single-grid
	if (DynamicEdgeNavigation.TargetButtons.Num() == 0)
	{
		if (CurrentGridIndex == DynamicEdgeNavigation.GridIndex)
		{
			const int TargetIndicesNum = DynamicEdgeNavigation.TargetButtonIndices.Num();
			const int TargetIndexInGrid = DynamicEdgeNavigation.TargetButtonIndices[AdaptedCurrentIndexInGrid < TargetIndicesNum ? AdaptedCurrentIndexInGrid : TargetIndicesNum - 1];
			UUINavButton* TargetButton = GetButtonAtGridIndex(DynamicEdgeNavigation.TargetGridIndex, TargetIndexInGrid);

			UpdateEdgeNavigation(CurrentGridIndex, TargetButton, Dir, false);
		}
		else if (DynamicEdgeNavigation.bTwoWayConnection && CurrentGridIndex == DynamicEdgeNavigation.TargetGridIndex)
		{
			int IndexInGrid = -1;
			for (int i = 0; i < DynamicEdgeNavigation.TargetButtonIndices.Num(); ++i)
			{
				if (CurrentIndexInGrid == DynamicEdgeNavigation.TargetButtonIndices[i])
				{
					const FGrid& TargetGrid = NavigationGrids[DynamicEdgeNavigation.GridIndex];
					if (TargetGrid.GridType == EGridType::Grid2D)
					{
						if (Dir == ENavigationDirection::Left) IndexInGrid = i * TargetGrid.DimensionX;
						else if (Dir == ENavigationDirection::Right) IndexInGrid = ((i + 1) * TargetGrid.DimensionX) - 1;
						else if (Dir == ENavigationDirection::Up) IndexInGrid = i < TargetGrid.DimensionX ? i : TargetGrid.DimensionX - 1;
						else if (Dir == ENavigationDirection::Down) IndexInGrid = TargetGrid.DimensionX * (TargetGrid.DimensionY - 1) + i;

						if (IndexInGrid >= TargetGrid.NumGrid2DButtons)
						{
							if (Dir == ENavigationDirection::Left) IndexInGrid = (i - 1) * TargetGrid.DimensionX;
							else if (Dir == ENavigationDirection::Right) IndexInGrid = TargetGrid.NumGrid2DButtons - 1;
							else if (Dir == ENavigationDirection::Down) IndexInGrid -= TargetGrid.DimensionX;
						}
					}
					else IndexInGrid = i;
					break;
				}
			}

			if (IndexInGrid == -1) return;

			UUINavButton* TargetButton = GetButtonAtGridIndex(DynamicEdgeNavigation.GridIndex, IndexInGrid);
			UpdateEdgeNavigation(DynamicEdgeNavigation.TargetGridIndex, TargetButton, Dir, true);
		}
	}
	// If multi-grid
	else
	{
		if (CurrentGridIndex == DynamicEdgeNavigation.GridIndex)
		{
			int IndexInGrid = CurrentIndexInGrid;
			if (CurrentGrid.GridType == EGridType::Grid2D)
			{
				int XCoord, YCoord;
				GetButtonCoordinatesInGrid2D(ButtonIndex, XCoord, YCoord);
				if ((Dir == ENavigationDirection::Left && XCoord != 0) ||
					(Dir == ENavigationDirection::Right && XCoord != CurrentGrid.DimensionX - 1) ||
					(Dir == ENavigationDirection::Up && YCoord != 0) ||
					(Dir == ENavigationDirection::Down && YCoord != CurrentGrid.DimensionY - 1))
				{
					return;
				}
				IndexInGrid = bHorizontal ? YCoord : XCoord;
			}
			else if ((bHorizontal && CurrentGrid.GridType == EGridType::Horizontal) ||
					(!bHorizontal && CurrentGrid.GridType == EGridType::Vertical))
			{
				return;
			}

			const int NumTargetButtons = DynamicEdgeNavigation.TargetButtons.Num();
			const FGridButton& GridButton = DynamicEdgeNavigation.TargetButtons[NumTargetButtons > IndexInGrid ? IndexInGrid : NumTargetButtons - 1];
			UUINavButton* TargetButton = GetButtonAtGridIndex(GridButton.GridIndex, GridButton.IndexInGrid);
			UpdateEdgeNavigation(DynamicEdgeNavigation.GridIndex, TargetButton, Dir, false);

			return;
		}
		else if (DynamicEdgeNavigation.bTwoWayConnection)
		{
			bool bFoundGrid = false;
			for (int i = 0; i < DynamicEdgeNavigation.TargetButtons.Num(); ++i)
			{
				const FGridButton& GridButton = DynamicEdgeNavigation.TargetButtons[i];
				if (!NavigationGrids.IsValidIndex(GridButton.GridIndex) ||
					GridButton.IndexInGrid >= NavigationGrids[GridButton.GridIndex].GetDimension())
				{
					return;
				}

				if (CurrentGridIndex == GridButton.GridIndex &&
					GetButtonIndexInGrid(ButtonIndex) == GridButton.IndexInGrid)
				{
					bFoundGrid = true;
					break;
				}
			}

			if (!bFoundGrid) return;

			int IndexInGrid = -1;
			const int GridIndex = -1;
			for (int i = 0; i < DynamicEdgeNavigation.TargetButtons.Num(); ++i)
			{
				const FGridButton& GridButton = DynamicEdgeNavigation.TargetButtons[i];
				if (GridButton.GridIndex == CurrentGridIndex &&
					GridButton.IndexInGrid == CurrentIndexInGrid)
				{
					const FGrid& TargetGrid = NavigationGrids[DynamicEdgeNavigation.GridIndex];
					if (TargetGrid.GridType == EGridType::Grid2D)
					{
						if (Dir == ENavigationDirection::Left) IndexInGrid = i * TargetGrid.DimensionX;
						else if (Dir == ENavigationDirection::Right) IndexInGrid = ((i + 1) * TargetGrid.DimensionX) - 1;
						else if (Dir == ENavigationDirection::Up) IndexInGrid = i < TargetGrid.DimensionX ? i : TargetGrid.DimensionX - 1;
						else if (Dir == ENavigationDirection::Down) IndexInGrid = TargetGrid.DimensionX * (TargetGrid.DimensionY - 1) + i;

						if (IndexInGrid >= TargetGrid.NumGrid2DButtons)
						{
							if (Dir == ENavigationDirection::Left) IndexInGrid = (i - 1) * TargetGrid.DimensionX;
							else if (Dir == ENavigationDirection::Right) IndexInGrid = TargetGrid.NumGrid2DButtons - 1;
							else if (Dir == ENavigationDirection::Up) IndexInGrid = i;
							else if (Dir == ENavigationDirection::Down) IndexInGrid -= TargetGrid.DimensionX;
						}
					}
					else IndexInGrid = i;
					break;
				}
			}

			if (GridIndex == -1 || IndexInGrid == -1) return;

			UUINavButton* TargetButton = GetButtonAtGridIndex(DynamicEdgeNavigation.GridIndex, IndexInGrid);
			UpdateEdgeNavigation(GridIndex, TargetButton, Dir, true);
		}
	}
}

void UUINavWidget::UpdateEdgeNavigation(const int GridIndex, UUINavButton* TargetButton, const ENavigationDirection Direction, const bool bInverted)
{
	if (!bInverted)
	{
		if (Direction == ENavigationDirection::Left) NavigationGrids[GridIndex].EdgeNavigation.LeftButton = TargetButton;
		else if (Direction == ENavigationDirection::Right) NavigationGrids[GridIndex].EdgeNavigation.RightButton = TargetButton;
		else if (Direction == ENavigationDirection::Up) NavigationGrids[GridIndex].EdgeNavigation.UpButton = TargetButton;
		else if (Direction == ENavigationDirection::Down) NavigationGrids[GridIndex].EdgeNavigation.DownButton = TargetButton;
	}
	else
	{
		if (Direction == ENavigationDirection::Left) NavigationGrids[GridIndex].EdgeNavigation.RightButton = TargetButton;
		else if (Direction == ENavigationDirection::Right) NavigationGrids[GridIndex].EdgeNavigation.LeftButton = TargetButton;
		else if (Direction == ENavigationDirection::Down) NavigationGrids[GridIndex].EdgeNavigation.UpButton = TargetButton;
		else if (Direction == ENavigationDirection::Up) NavigationGrids[GridIndex].EdgeNavigation.DownButton = TargetButton;
	}
}

void UUINavWidget::DispatchNavigation(const int Index, const bool bBypassForcedNavigation)
{
	if (bForcingNavigation || Index == HoveredButtonIndex || bBypassForcedNavigation)
	{
		//Update all the possible scroll boxes in the widget
		if (UINavButtons.IsValidIndex(Index))
		{
			for (UScrollBox* ScrollBox : ScrollBoxes)
			{
				ScrollBox->ScrollWidgetIntoView(UINavButtons[Index], bAnimateScrollBoxes);
			}
		}

		if (bUseButtonStates) UpdateHoveredButtonStates(Index);
	}

	if (UINavButtons.IsValidIndex(Index) && IsSelectorValid())
	{
		UpdateSelectorWaitForTick = 0;
		UpdateSelectorPrevButtonIndex = ButtonIndex;
		UpdateSelectorNextButtonIndex = Index;
		bShouldTickUpdateSelector = true;
	}

	if (bForcingNavigation || Index == HoveredButtonIndex || bBypassForcedNavigation)
	{
		if (bUseTextColor) UpdateTextColor(Index);

		UUINavComponent* FromComponent = GetUINavComponentAtIndex(ButtonIndex);
		UUINavComponent* ToComponent = GetUINavComponentAtIndex(Index);
		
		if (FromComponent != nullptr) FromComponent->OnNavigatedFrom();
		if (ToComponent != nullptr) ToComponent->OnNavigatedTo();
		if (UINavAnimations.Num() > 0) ExecuteAnimations(ButtonIndex, Index);
	}
}

void UUINavWidget::BeginSelectorMovement(const int PrevButtonIndex, const int NextButtonIndex)
{
	if (MoveCurve == nullptr) return;

	SelectorOrigin = bMovingSelector ? TheSelector->GetRenderTransform().Translation : GetButtonLocation(PrevButtonIndex);
	SelectorDestination = GetButtonLocation(NextButtonIndex);
	Distance = SelectorDestination - SelectorOrigin;

	float MinTime, MaxTime;
	MoveCurve->GetTimeRange(MinTime, MaxTime);
	MovementTime = MaxTime - MinTime;
	MovementCounter = 0.0f;

	bMovingSelector = true;
}

void UUINavWidget::CollectionOnSelect(const int Index)
{
	for (UUINavCollection* Collection : UINavCollections)
	{
		const int CollectionButtonIndex = GetCollectionFirstButtonIndex(Collection, Index);
		if (CollectionButtonIndex != -1)
		{
			Collection->OnSelect(Index, CollectionButtonIndex);
			break;
		}
	}
}

void UUINavWidget::CollectionOnStartSelect(const int Index)
{
	for (UUINavCollection* Collection : UINavCollections)
	{
		const int CollectionButtonIndex = GetCollectionFirstButtonIndex(Collection, Index);
		if (CollectionButtonIndex != -1)
		{
			Collection->OnStartSelect(Index, CollectionButtonIndex);
			break;
		}
	}
}

void UUINavWidget::CollectionOnStopSelect(const int Index)
{
	for (UUINavCollection* Collection : UINavCollections)
	{
		const int CollectionButtonIndex = GetCollectionFirstButtonIndex(Collection, Index);
		if (CollectionButtonIndex != -1)
		{
			Collection->OnStopSelect(Index, CollectionButtonIndex);
			break;
		}
	}
}

void UUINavWidget::OnPreSelect(const int Index, const bool bMouseClick)
{
	if (CurrentButton == nullptr ||
		SelectedButtonIndex == -1 ||
		!UINavButtons.IsValidIndex(Index)) return;

	const bool bIsSelectedButton = SelectedButtonIndex == Index && (!bMouseClick || UINavButtons[Index]->IsHovered());

	if (UINavInputContainer != nullptr && Index >= UINavInputContainer->FirstButtonIndex && Index <= UINavInputContainer->LastButtonIndex)
	{
		InputBoxIndex = Index - UINavInputContainer->FirstButtonIndex;
		const int KeysPerInput = UINavInputContainer->KeysPerInput;
		UINavInputBoxes[InputBoxIndex / KeysPerInput]->NotifySelected(InputBoxIndex % KeysPerInput);
		ReceiveInputType = UINavInputBoxes[InputBoxIndex / KeysPerInput]->IsAxis() ? EReceiveInputType::Axis : EReceiveInputType::Action;
		APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
		SetUserFocus(PC);
		SetKeyboardFocus();

		SwitchButtonStyle(UINavButtons[Index]->IsPressed() || SelectCount > 1 ? EButtonStyle::Pressed : (Index == ButtonIndex ? EButtonStyle::Hovered : EButtonStyle::Normal), ButtonIndex);

		if (SelectCount > 0) SelectCount--;
		if (SelectCount == 0) SelectedButtonIndex = -1;
	}
	else if (UINavButtons.IsValidIndex(Index))
	{
		SwitchButtonStyle(UINavButtons[Index]->IsPressed() || SelectCount > 1 ? EButtonStyle::Pressed : (Index == ButtonIndex ? EButtonStyle::Hovered : EButtonStyle::Normal), ButtonIndex);

		if (SelectCount > 0) SelectCount--;
		if (SelectCount == 0)
		{
			SelectedButtonIndex = -1;

			if (bIsSelectedButton)
			{
				for (FDynamicEdgeNavigation& DynamicEdgeNavigation : DynamicEdgeNavigations)
				{
					if (DynamicEdgeNavigation.Event == ENavigationEvent::OnSelect)
					{
						ProcessDynamicEdgeNavigation(DynamicEdgeNavigation);
					}
				}
				OnSelect(Index);
				CollectionOnSelect(Index);
			}
			OnStopSelect(Index);
			CollectionOnStopSelect(Index);

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
	}
}

void UUINavWidget::AttemptUnforceNavigation(const EInputType NewInputType)
{
	if (!bShouldForceNavigation && NewInputType == EInputType::Mouse)
	{
		bForcingNavigation = false;
		if (HoveredButtonIndex != -1)
		{
			if (ButtonIndex != HoveredButtonIndex)
			{
				NavigateTo(HoveredButtonIndex);
			}
		}
		else
		{
			DispatchNavigation(-1, true);
			OnNavigate(ButtonIndex, -1);
			CollectionNavigateTo(-1);
		}
	}
}

void UUINavWidget::OnReturn_Implementation()
{
	if(GetDefault<UUINavSettings>()->bRemoveWidgetOnReturn) ReturnToParent();
}

void UUINavWidget::CollectionOnReturn()
{
	for (UUINavCollection* Collection : UINavCollections)
	{
		Collection->NotifyOnReturn();
	}
}

void UUINavWidget::OnNext_Implementation()
{

}

void UUINavWidget::OnPrevious_Implementation()
{

}

void UUINavWidget::OnInputChanged_Implementation(const EInputType From, const EInputType To)
{

}

void UUINavWidget::PreSetup_Implementation(const bool bFirstSetup)
{

}

void UUINavWidget::OnSetupCompleted_Implementation()
{

}

void UUINavWidget::OnHorizCompNavigateLeft_Implementation(const int Index)
{

}

void UUINavWidget::OnHorizCompNavigateRight_Implementation(const int Index)
{

}

void UUINavWidget::OnHorizCompUpdated_Implementation(const int Index)
{

}

UUINavWidget* UUINavWidget::GoToWidget(TSubclassOf<UUINavWidget> NewWidgetClass, const bool bRemoveParent, const bool bDestroyParent, const int ZOrder)
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

UUINavWidget * UUINavWidget::GoToBuiltWidget(UUINavWidget* NewWidget, const bool bRemoveParent, const bool bDestroyParent, const int ZOrder)
{
	if (NewWidget == nullptr) return nullptr;

	UUINavWidget* OldOuterUINavWidget = GetMostOuterUINavWidget();
	UUINavWidget* NewOuterUINavWidget = NewWidget->GetMostOuterUINavWidget();
	
	if (OuterUINavWidget != nullptr || NewOuterUINavWidget == this)
	{
		if (NewOuterUINavWidget == OldOuterUINavWidget)
		{
			UINavPC->SetActiveNestedWidget(NewWidget);
			return NewWidget;
		}
		
		UINavPC->SetActiveNestedWidget(nullptr);
		if (OuterUINavWidget != nullptr)
		{
			OldOuterUINavWidget->PreviousNestedWidget = this;
		}
	}
	
	NewWidget->ParentWidget = GetMostOuterUINavWidget();
	NewWidget->bParentRemoved = bRemoveParent;
	NewWidget->bShouldDestroyParent = bDestroyParent;
	NewWidget->WidgetComp = WidgetComp;
	if (WidgetComp != nullptr)
	{
		WidgetComp->SetWidget(NewWidget);
	}
	else
	{
		if (!bForceUsePlayerScreen && (!bUsingSplitScreen || NewWidget->bUseFullscreenWhenSplitscreen)) NewWidget->AddToViewport(ZOrder);
		else NewWidget->AddToPlayerScreen(ZOrder);

		APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
		NewWidget->SetUserFocus(PC);
		if (UINavPC->GetInputMode() == EInputMode::UI)
		{
			NewWidget->SetKeyboardFocus();
		}
	}
	OldOuterUINavWidget->CleanSetup();
	
	return NewWidget;
}

void UUINavWidget::ReturnToParent(const bool bRemoveAllParents, const int ZOrder)
{
	if (ParentWidget == nullptr)
	{
		if (bAllowRemoveIfRoot && UINavPC != nullptr)
		{
			IUINavPCReceiver::Execute_OnRootWidgetRemoved(UINavPC->GetOwner());
			UINavPC->SetActiveWidget(nullptr);

			SelectCount = 0;
			SelectedButtonIndex = -1;
			if (WidgetComp != nullptr)
			{
				WidgetComp->SetWidget(nullptr);
			}
			else
			{
				bReturningToParent = true;
				RemoveFromParent();
			}
		}
		return;
	}

	SelectCount = 0;
	SelectedButtonIndex = -1;
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
		if (OuterUINavWidget == nullptr)
		{
			if (bRemoveAllParents)
			{
				IUINavPCReceiver::Execute_OnRootWidgetRemoved(UINavPC->GetOwner());
				UINavPC->SetActiveWidget(nullptr);
				ParentWidget->RemoveAllParents();
				bReturningToParent = true;
				RemoveFromParent();
				Destruct();
			}
			else
			{
				//If parent was removed, add it to viewport
				if (bParentRemoved)
				{
					if (IsValid(ParentWidget))
					{
						ParentWidget->ReturnedFromWidget = this;
						if (!bForceUsePlayerScreen && (!bUsingSplitScreen || ParentWidget->bUseFullscreenWhenSplitscreen)) ParentWidget->AddToViewport(ZOrder);
						else ParentWidget->AddToPlayerScreen(ZOrder);
					}
				}
				else
				{
					UUINavWidget* ParentOuter = ParentWidget->GetMostOuterUINavWidget();
					UINavPC->SetActiveWidget(ParentOuter);
					ParentWidget->ReconfigureSetup();
					if (ParentWidget->PreviousNestedWidget != nullptr)
					{
						UINavPC->SetActiveNestedWidget(ParentWidget->PreviousNestedWidget);
						ParentWidget->PreviousNestedWidget = nullptr;
					}
				}
				bReturningToParent = true;
				RemoveFromParent();
			}
		}
		else
		{
			UINavPC->SetActiveNestedWidget(ParentWidget);
		}
	}
}

void UUINavWidget::RemoveAllParents()
{
	if (ParentWidget != nullptr)
	{
		ParentWidget->RemoveAllParents();
	}
	bReturningToParent = true;
	RemoveFromParent();
	Destruct();
}

int UUINavWidget::GetWidgetHierarchyDepth(UWidget* Widget) const
{
	if (Widget == nullptr) return -1;

	int DepthCount = 0;
	UPanelWidget* Parent = Widget->GetParent();
	while (Parent != nullptr)
	{
		DepthCount++;
		Parent = Parent->GetParent();
	}
	return DepthCount;
}

void UUINavWidget::MenuNavigate(const ENavigationDirection Direction)
{
	UUINavButton* NewButton = FindNextButton(CurrentButton, Direction);
	if (NewButton == nullptr) return;
	NavigateTo(NewButton->ButtonIndex);
}

UUINavWidget* UUINavWidget::GetMostOuterUINavWidget()
{
	UUINavWidget* MostOUter = this;
	while (MostOUter->OuterUINavWidget != nullptr)
	{
		MostOUter = MostOUter->OuterUINavWidget;
	}

	return MostOUter;
}

UUINavWidget* UUINavWidget::GetChildUINavWidget(const int ChildIndex)
{
	return ChildIndex < ChildUINavWidgets.Num() ? ChildUINavWidgets[ChildIndex] : nullptr;
}

bool UUINavWidget::IsSelectorValid()
{
	return  TheSelector != nullptr && TheSelector->GetIsEnabled();
}

UUINavButton* UUINavWidget::FindNextButton(UUINavButton* Button, const ENavigationDirection Direction)
{
	if (Button == nullptr || Direction == ENavigationDirection::None) return nullptr;

	UUINavButton* NewButton = FetchButtonByDirection(Direction, Button);
	if (NewButton == nullptr || NewButton == Button) return nullptr;

	//Check if the button is visible, if not, skip to next button
	const bool bIgnoreDisabledUINavButton = GetDefault<UUINavSettings>()->bIgnoreDisabledUINavButton;
	bool bValid = NewButton->IsValid(bIgnoreDisabledUINavButton);
	if (bValid)
	{
		UUINavComponent* UINavComp = GetUINavComponentAtIndex(NewButton->ButtonIndex);
		if (UINavComp != nullptr && !UINavComp->IsValid(bIgnoreDisabledUINavButton)) bValid = false;
	}
	while (!bValid)
	{
		bValid = false;
		NewButton = FetchButtonByDirection(Direction, NewButton);
		if (NewButton == nullptr) return nullptr;
		UUINavComponent* UINavComp = GetUINavComponentAtIndex(NewButton->ButtonIndex);
		if (UINavComp != nullptr && !UINavComp->IsValid(bIgnoreDisabledUINavButton)) continue;
		if (NewButton == nullptr || NewButton == UINavButtons[ButtonIndex]) return nullptr;

		bValid = NewButton->IsValid(bIgnoreDisabledUINavButton);
	}
	return NewButton;
}

UUINavButton* UUINavWidget::FetchButtonByDirection(const ENavigationDirection Direction, UUINavButton* Button)
{
	UUINavButton* NextButton = nullptr;

	FGrid ButtonGrid;
	bool bIsValid;
	GetButtonGrid(Button->ButtonIndex, ButtonGrid, bIsValid);

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
							const int Offset = ButtonGrid.DimensionX * (ButtonGrid.DimensionY - 1) + Button->IndexInGrid;
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

UUINavButton * UUINavWidget::GetButtonAtIndex(const int InButtonIndex)
{
	if (!UINavButtons.IsValidIndex(InButtonIndex))
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

void UUINavWidget::GetGridAtIndex(const int GridIndex, FGrid& Grid, bool& IsValid)
{
	if (!NavigationGrids.IsValidIndex(GridIndex))
	{
		IsValid = false;
	}
	else
	{
		IsValid = true;
		Grid = NavigationGrids[GridIndex];
	}
}

int UUINavWidget::GetGridIndexFromWidgetObject(UWidget* Widget)
{
	int* GridIndex = GridIndexMap.Find(Widget);
	return GridIndex == nullptr || !NavigationGrids.IsValidIndex(*GridIndex) ? -1 : *GridIndex;
}

void UUINavWidget::GetButtonGrid(const int InButtonIndex, FGrid & ButtonGrid, bool & IsValid)
{
	if (!UINavButtons.IsValidIndex(InButtonIndex))
	{
		IsValid = false;
	}
	else
	{
		if (NavigationGrids.IsValidIndex(UINavButtons[InButtonIndex]->GridIndex))
		{
			IsValid = true;
			ButtonGrid = NavigationGrids[UINavButtons[InButtonIndex]->GridIndex];
			return;
		}
		else
		{
			IsValid = false;
			return;
		}
	}
}

int UUINavWidget::GetButtonIndexInGrid(const int InButtonIndex)
{
	if (!UINavButtons.IsValidIndex(InButtonIndex)) return -1;

	return UINavButtons[InButtonIndex]->IndexInGrid;
}

int UUINavWidget::GetButtonGridIndex(const int InButtonIndex)
{
	if (!UINavButtons.IsValidIndex(InButtonIndex)) return -1;
	
	return UINavButtons[InButtonIndex]->GridIndex;
}

int UUINavWidget::GetGridStartingIndex(const int GridIndex)
{
	if (!NavigationGrids.IsValidIndex(GridIndex)) return -1;

	if (NavigationGrids[GridIndex].FirstButton != nullptr)
	{
		if (NavigationGrids[GridIndex].FirstButton->ButtonIndex < 0)
		{
			if (GridIndex > 0) return (GetGridStartingIndex(GridIndex - 1) +  NavigationGrids[GridIndex - 1].GetDimension());
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
	return 0;
}

UUINavButton * UUINavWidget::GetButtonAtGridIndex(const int GridIndex, int IndexInGrid)
{
	if (!NavigationGrids.IsValidIndex(GridIndex) || IndexInGrid < -1) return nullptr;

	const FGrid& ButtonGrid = NavigationGrids[GridIndex];
	if (ButtonGrid.FirstButton == nullptr) return nullptr;
	const int ButtonGridDimension = ButtonGrid.GetDimension();
	if (IndexInGrid >= ButtonGridDimension) return nullptr;
	if (IndexInGrid == -1) IndexInGrid = ButtonGridDimension - 1;
	const int NewIndex = ButtonGrid.FirstButton->ButtonIndex + IndexInGrid;

	if (NewIndex >= UINavButtons.Num()) return nullptr;

	return UINavButtons[NewIndex];
}

bool UUINavWidget::IsButtonInGrid(const int InButtonIndex, const int GridIndex)
{
	if (!UINavButtons.IsValidIndex(InButtonIndex)) return false;
	
	return UINavButtons[InButtonIndex]->GridIndex == GridIndex;
}

void UUINavWidget::GetButtonCoordinatesInGrid2D(const int InButtonIndex, int& XCoord, int& YCoord)
{
	XCoord = -1;
	YCoord = -1;
	if (!UINavButtons.IsValidIndex(InButtonIndex)) return;
	const UUINavButton* Button = UINavButtons[InButtonIndex];

	const FGrid& Grid = NavigationGrids[Button->GridIndex];
	if (Grid.GridType != EGridType::Grid2D) return;

	YCoord = Button->IndexInGrid / Grid.DimensionX;
	XCoord = Button->IndexInGrid - (YCoord * Grid.DimensionX);
}

UUINavButton * UUINavWidget::GetButtonFromCoordinatesInGrid2D(const int GridIndex, const int XCoord, const int YCoord)
{
	if (!NavigationGrids.IsValidIndex(GridIndex)) return nullptr;

	const FGrid& Grid = NavigationGrids[GridIndex];
	if (Grid.GridType != EGridType::Grid2D ||
		Grid.FirstButton == nullptr ||
		XCoord < 0 ||
		YCoord < 0 ||
		XCoord >= Grid.DimensionX ||
		YCoord >= Grid.DimensionY ||
		XCoord * YCoord + XCoord > Grid.NumGrid2DButtons)
		return nullptr;

	const int Index = Grid.FirstButton->ButtonIndex + YCoord * Grid.DimensionX + XCoord;
	if (Index >= UINavButtons.Num()) return nullptr;

	return UINavButtons[Index];
}

int UUINavWidget::GetCollectionFirstButtonIndex(UUINavCollection * Collection, const int Index)
{
	if (Collection == nullptr || Index == -1) return -1;

	if (Collection->FirstButtonIndex <= Index &&
		Collection->LastButtonIndex >= Index &&
		Collection->FirstButtonIndex <= Collection->LastButtonIndex)
	{
		return Index - Collection->FirstButtonIndex;
	}

	return -1;
}

UUINavComponent * UUINavWidget::GetUINavComponentAtIndex(const int Index)
{
	if (!UINavButtons.IsValidIndex(Index)) return nullptr;
	return UINavButtons[Index]->NavComp;
}

UUINavHorizontalComponent * UUINavWidget::GetUINavHorizontalCompAtIndex(const int Index)
{
	return Cast<UUINavHorizontalComponent>(GetUINavComponentAtIndex(Index));
}

void UUINavWidget::HoverEvent(int Index)
{
	if (!UINavButtons.IsValidIndex(Index)) return;
	
	if ((OuterUINavWidget != nullptr || ChildUINavWidgets.Num() > 0) &&
		UINavPC != nullptr && !UINavPC->ShouldIgnoreHoverEvents())
	{
		UINavPC->SetActiveNestedWidget(this);
	}
	
	if (bIgnoreMouseEvent)
	{
		bIgnoreMouseEvent = false;
		return;
	}

	if (IsRebindingInput())
	{
		CancelRebind();
	}

	if (UINavPC == nullptr || !UINavPC->AllowsDirectionalInput())
	{
		return;
	}

	HoveredButtonIndex = Index;

#if IS_VR_PLATFORM 
	const bool bIsVR = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
#else
	const bool bIsVR = false;
#endif
	
	if (bForcingNavigation &&
		((Index == ButtonIndex) ||
		(!bIsVR &&
		UINavPC->GetCurrentInputType() != EInputType::Mouse &&
		((!bUseLeftThumbstickAsMouse && !UINavPC->bUseLeftThumbstickAsMouse) || !UINavPC->IsMovingLeftStick()))))
	{
		if (bUseButtonStates) RevertButtonStyle(Index);
		return;
	}

	UINavPC->ClearTimer();
	NavigateTo(Index, true);
}

void UUINavWidget::UnhoverEvent(int Index)
{
	if (!UINavButtons.IsValidIndex(Index)) return;
	
	if (bIgnoreMouseEvent)
	{
		bIgnoreMouseEvent = false;
		return;
	}

	if (IsRebindingInput())
	{
		CancelRebind();
	}

	if (!bShouldForceNavigation)
	{
		DispatchNavigation(-1, true);
		OnNavigate(ButtonIndex, -1);
		CollectionNavigateTo(-1);
	}

	HoveredButtonIndex = -1;

	if (bUseButtonStates && bForcingNavigation)
	{
		const UUINavButton* ToButton = UINavButtons[Index];
		if (SelectedButtonIndex != ButtonIndex)
		{
			SwitchButtonStyle(ButtonIndex == Index ? EButtonStyle::Hovered : EButtonStyle::Normal, Index);
		}

		ButtonIndex = (ToButton->ForcedStyle != EButtonStyle::None) ? Index : ButtonIndex;
	}
}

void UUINavWidget::PressEvent(int Index)
{
	if (!UINavButtons.IsValidIndex(Index) || UINavPC == nullptr) return;
	
	if (IsRebindingInput())
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

		if (!UINavPC->AllowsSelectInput()) return;

		FinishPress(true);
	}
}

void UUINavWidget::ReleaseEvent(int Index)
{
	if (!UINavButtons.IsValidIndex(Index) || (!bHasNavigation && SelectCount == 0) || UINavPC == nullptr) return;
	
	if (bIgnoreMouseEvent)
	{
		bIgnoreMouseEvent = false;
		return;
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

void UUINavWidget::FinishPress(const bool bMouse)
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
	if (NewButton->CustomHover.IsBound())
	{
		NewButton->CustomHover.Clear();
	}
	NewButton->CustomHover.AddDynamic(this, &UUINavWidget::HoverEvent);
	if (NewButton->CustomUnhover.IsBound())
	{
		NewButton->CustomUnhover.Clear();
	}
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
	const int KeysPerInput = UINavInputContainer->KeysPerInput;
	UUINavInputBox* const UINavInputBox = UINavInputBoxes[InputBoxIndex / KeysPerInput];
	const FKey OldKey = UINavInputBox->GetKey(InputBoxIndex % KeysPerInput);

	UINavInputBox->UpdateInputKey(PressedKey, InputBoxIndex % KeysPerInput);

	const FKey NewKey = UINavInputBox->GetKey(InputBoxIndex % KeysPerInput);
	if (OldKey != NewKey) UINavInputContainer->OnKeyRebinded(UINavInputBox->InputName, OldKey, PressedKey);
	ReceiveInputType = EReceiveInputType::None;
}

void UUINavWidget::CancelRebind()
{
	const int KeysPerInput = UINavInputContainer->KeysPerInput;
	UINavInputBoxes[InputBoxIndex / KeysPerInput]->RevertToKeyText(InputBoxIndex % KeysPerInput);
	ReceiveInputType = EReceiveInputType::None;
}

void UUINavWidget::NavigateInDirection(const ENavigationDirection Direction)
{
	if (IsRebindingInput())
	{
		CancelRebind();
		return;
	}
	
	if (!bForcingNavigation)
	{
		bForcingNavigation = true;
		if (HoveredButtonIndex == -1)
		{
			NavigateTo(ButtonIndex, false, true);
			return;
		}
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
	if (IsRebindingInput())
	{
		CancelRebind();
		return;
	}

	if (!bForcingNavigation && HoveredButtonIndex == -1)
	{
		bRestoreNavigation = true;
		return;
	}

	if (CurrentButton != nullptr)
	{
		FinishPress(false);

		if (SelectCount == 1)
		{
			USoundBase* PressSound = Cast<USoundBase>(CurrentButton->WidgetStyle.PressedSlateSound.GetResourceObject());
			if (PressSound != nullptr) PlaySound(PressSound);
			bIgnoreMouseEvent = true;
			CurrentButton->OnPressed.Broadcast();
		}
	}
}

void UUINavWidget::MenuSelectRelease()
{
	if (bRestoreNavigation)
	{
		bRestoreNavigation = false;
		bForcingNavigation = true;
		NavigateTo(ButtonIndex, false, true);
		return;
	}
	
	if (bMovingSelector)
	{
		HaltedIndex = SELECT_INDEX;
		return;
	}
	OnPreSelect(ButtonIndex);
}

void UUINavWidget::MenuReturnPress()
{
	if (!bForcingNavigation && HoveredButtonIndex == -1)
	{
		bRestoreNavigation = true;
		return;
	}
	
	bReturning = true;
}

void UUINavWidget::MenuReturnRelease()
{
	if (bRestoreNavigation)
	{
		bRestoreNavigation = false;
		bForcingNavigation = true;
		NavigateTo(ButtonIndex, false, true);
		return;
	}
	
	if (!bReturning) return;

	if (IsRebindingInput())
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

	CollectionOnReturn();
	OnReturn();
}
