// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavWidget.h"
#include "UINavHorizontalComponent.h"
#include "UINavComponent.h"
#include "UINavigationConfig.h"
#include "UINavInputBox.h"
#include "UINavPCComponent.h"
#include "UINavPCReceiver.h"
#include "UINavPromptWidget.h"
#include "UINavSettings.h"
#include "UINavWidgetComponent.h"
#include "UINavBlueprintFunctionLibrary.h"
#include "UINavMacros.h"
#include "UINavSectionsWidget.h"
#include "UINavSectionButton.h"
#include "ComponentActions/UINavComponentAction.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ActorComponent.h"
#include "Components/ListView.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/GameViewportClient.h"
#include "Engine/ViewportSplitScreen.h"
#include "Curves/CurveFloat.h"

const TArray<FString> UUINavWidget::AllowedObjectTypesToFocus = {
		TEXT("SObjectWidget"),
		TEXT("SButton"),
		TEXT("SUINavButton"),
		TEXT("SSpinBox"),
		TEXT("SEditableText"),
		TEXT("SMultilineEditableText")
};

UUINavWidget::UUINavWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UUINavWidget::NativeConstruct()
{
	bBeingRemoved = false;

	bForcingNavigation = GetDefault<UUINavSettings>()->bForceNavigation;

	const UWorld* const World = GetWorld();
	OuterUINavWidget = GetOuterObject<UUINavWidget>(this);
	if (OuterUINavWidget != nullptr)
	{
		if (!IsValid(OuterUINavWidget->GetFirstComponent()))
		{
			OuterUINavWidget->SetFirstComponent(FirstComponent);
		}

		ParentWidget = OuterUINavWidget;
		PreSetup(!bCompletedSetup);

		ConfigureUINavPC();

		Super::NativeConstruct();
		return;
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
	}

	PreSetup(!bCompletedSetup);
	InitialSetup();

	Super::NativeConstruct();
}

void UUINavWidget::InitialSetup(const bool bRebuilding)
{
	if (!bRebuilding)
	{
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

	TraverseHierarchy();

	SetupSections();

	//If this widget doesn't need to create the selector, skip to setup
	if (!IsSelectorValid())
	{
		UINavSetup();
	}
	else
	{
		SetupSelector();
		UINavSetupWaitForTick = 0;
	}
}

void UUINavWidget::ReconfigureSetup()
{
	bSetupStarted = true;

	if (!IsSelectorValid())
	{
		UINavSetup();
	}
	else
	{
		SetupSelector();
		UINavSetupWaitForTick = 0;
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
	
	bSetupStarted = false;
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

void UUINavWidget::TraverseHierarchy()
{
	//Find UINavButtons in the widget hierarchy
	TArray<UWidget*> Widgets;
	WidgetTree->GetAllWidgets(Widgets);
	for (UWidget* Widget : Widgets)
	{
		UUINavWidget* ChildUINavWidget = Cast<UUINavWidget>(Widget);
		if (ChildUINavWidget != nullptr)
		{
			ChildUINavWidget->AddParentToPath(ChildUINavWidgets.Num());
			ChildUINavWidgets.Add(ChildUINavWidget);
		}
	}
}

void UUINavWidget::SetupSections()
{
	if (!IsValid(UINavSectionsPanel) && !IsValid(UINavSwitcher))
	{
		return;
	}

	if (!IsValid(UINavSwitcher))
	{
		DISPLAYERROR("UINavSectionsPanel doesn't have corresponding UINavSwitcher!");
		return;
	}

	if (!IsValid(UINavSectionsPanel))
	{
		return;
	}

	UUINavSectionsWidget* SectionsWidget = Cast<UUINavSectionsWidget>(UINavSectionsPanel);
	UPanelWidget* SectionsPanel = IsValid(SectionsWidget) ? SectionsWidget->SectionButtonsPanel : Cast<UPanelWidget>(UINavSectionsPanel);
	if (!IsValid(SectionsPanel))
	{
		DISPLAYERROR("UINavSectionsPanel isn't a PanelWidget child or UINavSectionsWidget!");
		return;
	}

	if (SectionButtons.IsEmpty())
	{
#if WITH_EDITOR
		static const TArray<TSubclassOf<UWidget>> ButtonClassArray = { UButton::StaticClass(), UUINavSectionButton::StaticClass(), UUINavComponent::StaticClass() };
#else
		static const TArray<TSubclassOf<UWidget>> ButtonClassArray = { UButton::StaticClass(), UUINavSectionButton::StaticClass() };
#endif

		for (UWidget* const ChildWidget : SectionsPanel->GetAllChildren())
		{
			UWidget* TargetWidget = UUINavBlueprintFunctionLibrary::FindWidgetOfClassesInWidget(ChildWidget, ButtonClassArray);

			if (TargetWidget->IsA<UUINavComponent>())
			{
				DISPLAYERROR("UINavSectionsPanel has a UINavComponent. It should only have normal Buttons!");
				return;
			}

			UButton* SectionButton = nullptr;

			const UUINavSectionButton* const ChildSectionButtonWidget = Cast<UUINavSectionButton>(TargetWidget);
			if (IsValid(ChildSectionButtonWidget))
			{
				SectionButton = ChildSectionButtonWidget->SectionButton;
			}

			if (!IsValid(SectionButton))
			{
				SectionButton = Cast<UButton>(TargetWidget);
			}

			if (!IsValid(SectionButton))
			{
				continue;
			}

			SectionButtons.Add(SectionButton);
		}
	}

	for (int i = 0; i < SectionButtons.Num(); ++i)
	{
		switch (i)
		{
		case 0:
			SectionButtons[i]->OnClicked.AddUniqueDynamic(this, &UUINavWidget::OnSectionButtonPressed1);
				break;
		case 1:
			SectionButtons[i]->OnClicked.AddUniqueDynamic(this, &UUINavWidget::OnSectionButtonPressed2);
				break;
		case 2:
			SectionButtons[i]->OnClicked.AddUniqueDynamic(this, &UUINavWidget::OnSectionButtonPressed3);
			break;
		case 3:
			SectionButtons[i]->OnClicked.AddUniqueDynamic(this, &UUINavWidget::OnSectionButtonPressed4);
			break;
		case 4:
			SectionButtons[i]->OnClicked.AddUniqueDynamic(this, &UUINavWidget::OnSectionButtonPressed5);
			break;
		case 5:
			SectionButtons[i]->OnClicked.AddUniqueDynamic(this, &UUINavWidget::OnSectionButtonPressed6);
			break;
		case 6:
			SectionButtons[i]->OnClicked.AddUniqueDynamic(this, &UUINavWidget::OnSectionButtonPressed7);
			break;
		case 7:
			SectionButtons[i]->OnClicked.AddUniqueDynamic(this, &UUINavWidget::OnSectionButtonPressed8);
			break;
		case 8:
			SectionButtons[i]->OnClicked.AddUniqueDynamic(this, &UUINavWidget::OnSectionButtonPressed9);
			break;
		case 9:
			SectionButtons[i]->OnClicked.AddUniqueDynamic(this, &UUINavWidget::OnSectionButtonPressed10);
			break;
		}
	}

	if (SectionWidgets.IsEmpty())
	{
		static const TArray<TSubclassOf<UWidget>> WidgetClassArray = { UUINavWidget::StaticClass(), UUINavComponent::StaticClass() };
		for (UWidget* const ChildWidget : UINavSwitcher->GetAllChildren())
		{
			UWidget* TargetWidget = UUINavBlueprintFunctionLibrary::FindWidgetOfClassesInWidget(ChildWidget, WidgetClassArray);
			if (IsValid(TargetWidget))
			{
				SectionWidgets.Add(TargetWidget);
			}
		}
	}
}

void UUINavWidget::SetupSelector()
{
	UCanvasPanelSlot* SelectorSlot = Cast<UCanvasPanelSlot>(TheSelector->Slot);
	if (IsValid(SelectorSlot))
	{
		SelectorSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		SelectorSlot->SetPosition(FVector2D(0.f, 0.f));
	}
	else
	{
		DISPLAYERROR("Selector must be a direct child of a Canvas Panel!");
	}
}

void UUINavWidget::UINavSetup()
{
	if (UINavPC == nullptr) return;

	UUINavWidget* CurrentActiveWidget = UINavPC->GetActiveWidget();
	const bool bShouldTakeFocus =
		!IsValid(CurrentActiveWidget) ||
		CurrentActiveWidget == OuterUINavWidget ||
		CurrentActiveWidget == ParentWidget ||
		CurrentActiveWidget->ParentWidget == this ||
		ReturnedFromWidget != nullptr;

	if (ReturnedFromWidget != nullptr && IsValid(CurrentComponent))
	{
		if (bShouldTakeFocus)
		{
			CurrentComponent->SetFocus();
		}
		if (!GetDefault<UUINavSettings>()->bForceNavigation && !IsValid(HoveredComponent))
		{
			UnforceNavigation(false);
		}
	}
	else if (bShouldTakeFocus && !TryFocusOnInitialComponent())
	{
		UINavPC->NotifyNavigatedTo(this);
	}

	bCompletedSetup = true;
	ReturnedFromWidget = nullptr;
	IgnoreHoverComponent = nullptr;

	PropagateOnSetupCompleted();
}

UUINavComponent* UUINavWidget::GetInitialFocusComponent_Implementation()
{
	return FirstComponent;
}

bool UUINavWidget::TryFocusOnInitialComponent()
{
	UUINavComponent* InitialComponent = GetInitialFocusComponent();
	if (IsValid(InitialComponent))
	{
		InitialComponent->SetFocus();
		return true;
	}

	return false;
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

	if (IsValid(FirstComponent))
	{
		bHasNavigation = true;
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

	const bool bHaveSameOuter = NewActiveWidget->GetMostOuterUINavWidget() == GetMostOuterUINavWidget();

	const bool bNewWidgetIsChild = NewActiveWidget != nullptr && NewActiveWidget->GetUINavWidgetPath().Num() > 0 ?
		UUINavBlueprintFunctionLibrary::ContainsArray<int>(NewActiveWidget->GetUINavWidgetPath(), UINavWidgetPath) && bHaveSameOuter :
		false;

	if (bNewWidgetIsChild && !bMaintainNavigationForChild)
	{
		return;
	}

	if (!NewActiveWidget->bMaintainNavigationForChild || !bNewWidgetIsChild)
	{
		UpdateNavigationVisuals(nullptr, true, false, true);
	}

	CallOnNavigate(CurrentComponent, nullptr);

	bHasNavigation = false;

	if (!bNewWidgetIsChild && bHaveSameOuter && bClearNavigationStateWhenChild) CurrentComponent = nullptr;

	OnLostNavigation(NewActiveWidget, bNewWidgetIsChild);
}

void UUINavWidget::OnLostNavigation_Implementation(UUINavWidget* NewActiveWidget, const bool bToChild)
{
}

void UUINavWidget::SetCurrentComponent(UUINavComponent* Component)
{
	const bool bShouldUnforceNavigation = !IsValid(CurrentComponent) && !GetDefault<UUINavSettings>()->bForceNavigation && !IsValid(HoveredComponent);

	CurrentComponent = Component;

	if (bShouldUnforceNavigation)
	{
		UnforceNavigation(false);
	}

	if (IsValid(OuterUINavWidget) && !OuterUINavWidget->bMaintainNavigationForChild)
	{
		OuterUINavWidget->SetCurrentComponent(Component);
	}
}

void UUINavWidget::SetHoveredComponent(UUINavComponent* Component)
{
	HoveredComponent = Component;

	if (IsValid(OuterUINavWidget) && !OuterUINavWidget->bMaintainNavigationForChild)
	{
		OuterUINavWidget->SetHoveredComponent(Component);
	}
}

void UUINavWidget::SetSelectedComponent(UUINavComponent* Component)
{
	SelectedComponent = Component;

	if (IsValid(OuterUINavWidget) && !OuterUINavWidget->bMaintainNavigationForChild)
	{
		OuterUINavWidget->SetSelectedComponent(Component);
	}
}

void UUINavWidget::SetPressingReturn(const bool InbPressingReturn)
{
	bPressingReturn = true;

	if (OuterUINavWidget != nullptr)
	{
		OuterUINavWidget->SetPressingReturn(InbPressingReturn);
	}
}

FReply UUINavWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	FReply Reply = Super::NativeOnKeyDown(InGeometry, InKeyEvent);
	if (!IsValid(CurrentComponent)) UUINavWidget::HandleOnKeyDown(Reply, this, nullptr, InKeyEvent);
	return Reply;
}

FReply UUINavWidget::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	FReply Reply = Super::NativeOnKeyUp(InGeometry, InKeyEvent);
	if (!IsValid(CurrentComponent)) UUINavWidget::HandleOnKeyUp(Reply, this, nullptr, InKeyEvent);
	return Reply;
}

void UUINavWidget::NativeTick(const FGeometry & MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	if (IsSelectorValid())
	{
		if (UINavSetupWaitForTick >= 0)
		{
			if (UINavSetupWaitForTick >= 1)
			{
				UINavSetup();
				UINavSetupWaitForTick = -1;
			}
			else
			{
				UINavSetupWaitForTick++;
			}
		}

		if (UpdateSelectorWaitForTick >= 0)
		{
			if (UpdateSelectorWaitForTick >= 1)
			{
				if (MoveCurve != nullptr) BeginSelectorMovement(UpdateSelectorPrevComponent, UpdateSelectorNextComponent);
				else UpdateSelectorLocation(UpdateSelectorNextComponent);
				UpdateSelectorWaitForTick = -1;
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

void UUINavWidget::RemoveFromParent()
{
	bBeingRemoved = true;
	if (OuterUINavWidget == nullptr && !bReturningToParent && !bDestroying && !GetFName().IsNone() && IsValid(this) &&
	    (ParentWidget != nullptr || (bAllowRemoveIfRoot && UINavPC != nullptr)))
	{
		ReturnToParent();
		return;
	}
	bReturningToParent = false;

	Super::RemoveFromParent();
}

FReply UUINavWidget::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	FReply Reply = Super::NativeOnFocusReceived(InGeometry, InFocusEvent);

	if (OuterUINavWidget != nullptr)
	{
		if (IsValid(CurrentComponent))
		{
			CurrentComponent->SetFocus();
		}
		else if (!TryFocusOnInitialComponent())
		{
			UINavPC->NotifyNavigatedTo(this);
		}

		return Reply;
	}

	if (IsValid(CurrentComponent))
	{
		CurrentComponent->SetFocus();
		return Reply;
	}

	TryFocusOnInitialComponent();

	return Reply;
}

void UUINavWidget::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusLost(InFocusEvent);
}

void UUINavWidget::NativeOnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusChanging(PreviousFocusPath, NewWidgetPath, InFocusEvent);

	if (!IsValid(CurrentComponent))
	{
		UUINavWidget::HandleOnFocusChanging(this, nullptr, PreviousFocusPath, NewWidgetPath, InFocusEvent);
	}
}

FNavigationReply UUINavWidget::NativeOnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply)
{
	FNavigationReply Reply = Super::NativeOnNavigation(MyGeometry, InNavigationEvent, InDefaultReply);
	if (!IsValid(CurrentComponent)) UUINavWidget::HandleOnNavigation(Reply, this, InNavigationEvent);
	return Reply;
}

void UUINavWidget::HandleOnFocusChanging(UUINavWidget* Widget, UUINavComponent* Component, const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent)
{
	if (!NewWidgetPath.IsValid() ||
		NewWidgetPath.Widgets.Num() == 0 ||
		Widget->UINavPC->GetInputMode() == EInputMode::Game ||
		(InFocusEvent.GetCause() == EFocusCause::Mouse &&
			Widget->UINavPC->GetInputMode() == EInputMode::GameUI &&
			GetDefault<UUINavSettings>()->bAllowFocusOnViewportInGameAndUI))
	{
		return;
	}

	if (InFocusEvent.GetCause() == EFocusCause::Navigation && Widget->UINavPC->IgnoreFocusByNavigation())
	{
		UUserWidget* PreviousUserWidget = UUINavWidget::FindUserWidgetInWidgetPath(PreviousFocusPath, PreviousFocusPath.GetLastWidget().Pin());
		if (IsValid(PreviousUserWidget))
		{
			PreviousUserWidget->SetFocus();
		}

		return;
	}

	const FString LastWidgetTypeStr = NewWidgetPath.GetLastWidget()->GetTypeAsString();
	const bool LastWidgetIsButton = LastWidgetTypeStr == TEXT("SButton");
	UUserWidget* ParentWidget = LastWidgetIsButton ? UUINavWidget::FindUserWidgetInWidgetPath(NewWidgetPath, NewWidgetPath.GetLastWidget()) : nullptr;
	if (InFocusEvent.GetCause() == EFocusCause::WindowActivate ||
		!UUINavWidget::AllowedObjectTypesToFocus.Contains(LastWidgetTypeStr) ||
		(LastWidgetIsButton && !IsValid(ParentWidget)))
	{
		if (IsValid(Component))
		{
			Component->NavButton->SetFocus();
		}
		else
		{
			Widget->SetFocus();
		}

		return;
	}

	if (IsValid(Component))
	{
		const bool bHadFocus = PreviousFocusPath.ContainsWidget(&Component->TakeWidget().Get());
		const bool bHasFocus = NewWidgetPath.ContainsWidget(&Component->TakeWidget().Get());
		const bool bHasButtonFocus = NewWidgetPath.ContainsWidget(&Component->NavButton->TakeWidget().Get());

		if (!bHadFocus && bHasFocus)
		{
			Component->HandleFocusReceived();
		}
		else if (bHadFocus && !bHasFocus)
		{
			Component->HandleFocusLost();
		}

		if (bHasFocus && !bHasButtonFocus)
		{
			Component->NavButton->SetFocus();
		}
	}
}

void UUINavWidget::HandleOnNavigation(FNavigationReply& Reply, UUINavWidget* Widget, const FNavigationEvent& InNavigationEvent)
{
	if (!IsValid(Widget) || !IsValid(Widget->UINavPC))
	{
		return;
	}

	if (!Widget->UINavPC->AllowsNavigatingDirection(InNavigationEvent.GetNavigationType()))
	{
		Reply = FNavigationReply::Stop();
		return;
	}

	if (Widget->TryConsumeNavigation())
	{
		Reply = FNavigationReply::Stop();
		return;
	}

	if (!Widget->UINavPC->TryNavigateInDirection(InNavigationEvent.GetNavigationType(), InNavigationEvent.GetNavigationGenesis()))
	{
		Reply = FNavigationReply::Stop();
		return;
	}

	const bool bStopNextPrevious = GetDefault<UUINavSettings>()->bStopNextPreviousNavigation;
	const bool bAllowsSectionInput = Widget->UINavPC->AllowsSectionInput();

	if (InNavigationEvent.GetNavigationType() == EUINavigation::Next)
	{
		if (bAllowsSectionInput)
		{
			Widget->PropagateOnNext();

			IUINavPCReceiver::Execute_OnNext(Widget->UINavPC->GetOwner());
		}

		if (bStopNextPrevious || !bAllowsSectionInput)
		{
			Widget->UINavPC->SetIgnoreFocusByNavigation(true);
			Reply = FNavigationReply::Stop();
			return;
		}
	}
	else if (InNavigationEvent.GetNavigationType() == EUINavigation::Previous)
	{
		if (bAllowsSectionInput)
		{
			IUINavPCReceiver::Execute_OnPrevious(Widget->UINavPC->GetOwner());
			Widget->PropagateOnPrevious();
		}

		if (bStopNextPrevious || !bAllowsSectionInput)
		{
			Widget->UINavPC->SetIgnoreFocusByNavigation(true);
			Reply = FNavigationReply::Stop();
			return;
		}
	}
	else if (InNavigationEvent.GetNavigationType() != EUINavigation::Invalid)
	{
		IUINavPCReceiver::Execute_OnNavigated(Widget->UINavPC->GetOwner(), InNavigationEvent.GetNavigationType());
	}
}

void UUINavWidget::HandleOnKeyDown(FReply& Reply, UUINavWidget* Widget, UUINavComponent* Component, const FKeyEvent& InKeyEvent)
{
	if (!IsValid(Widget) || !IsValid(Widget->UINavPC))
	{
		return;
	}

	if (IsValid(Component) && Widget->UINavPC->IsListeningToInputRebind())
	{
		Component->bIgnoreDueToRebind = true;
		return;
	}

	const bool bHandleReply = Widget->OuterUINavWidget == nullptr && GetDefault<UUINavSettings>()->bConsumeNavigationInputs;
	if (FSlateApplication::Get().GetNavigationActionFromKey(InKeyEvent) == EUINavigationAction::Accept)
	{
		if (!Widget->TryConsumeNavigation())
		{
			Widget->StartedSelect();
			if (bHandleReply)
			{
				Reply = FReply::Handled();
			}
		}
	}
	else if (FSlateApplication::Get().GetNavigationActionFromKey(InKeyEvent) == EUINavigationAction::Back)
	{
		if (!Widget->TryConsumeNavigation())
		{
			Widget->StartedReturn();
			if (bHandleReply)
			{
				Reply = FReply::Handled();
			}
		}
	}

	TSharedRef<FUINavigationConfig> NavConfig = StaticCastSharedRef<FUINavigationConfig>(FSlateApplication::Get().GetNavigationConfig());
	EUINavigation Direction = NavConfig->GetNavigationDirectionFromKey(InKeyEvent);
	if (Direction == EUINavigation::Invalid)
	{
		Direction = NavConfig->GetNavigationDirectionFromAnalogKey(InKeyEvent);
	}
	if (Direction != EUINavigation::Invalid)
	{
		Widget->UINavPC->NotifyNavigationKeyPressed(InKeyEvent.GetKey(), Direction);
		if (bHandleReply)
		{
			Reply = FReply::Handled();
		}
	}
}

void UUINavWidget::HandleOnKeyUp(FReply& Reply, UUINavWidget* Widget, UUINavComponent* Component, const FKeyEvent& InKeyEvent)
{
	if (!IsValid(Widget) || !IsValid(Widget->UINavPC))
	{
		return;
	}

	if (IsValid(Component) && (Component->bIgnoreDueToRebind || Widget->UINavPC->IsListeningToInputRebind()))
	{
		Component->bIgnoreDueToRebind = false;
		Widget->UINavPC->ProcessRebind(InKeyEvent);
		return;
	}

	const bool bHandleReply = Widget->OuterUINavWidget == nullptr && GetDefault<UUINavSettings>()->bConsumeNavigationInputs;

	if (FSlateApplication::Get().GetNavigationActionFromKey(InKeyEvent) == EUINavigationAction::Accept)
	{
		if (!Widget->TryConsumeNavigation())
		{
			Widget->StoppedSelect();
			if (bHandleReply)
			{
				Reply = FReply::Handled();
			}
		}
	}
	else if (FSlateApplication::Get().GetNavigationActionFromKey(InKeyEvent) == EUINavigationAction::Back)
	{
		if (!Widget->TryConsumeNavigation())
		{
			Widget->StoppedReturn();
			if (bHandleReply)
			{
				Reply = FReply::Handled();
			}
		}
	}
	else
	{
		TSharedRef<FUINavigationConfig> NavConfig = StaticCastSharedRef<FUINavigationConfig>(FSlateApplication::Get().GetNavigationConfig());
		EUINavigation Direction = NavConfig->GetNavigationDirectionFromKey(InKeyEvent);
		if (Direction == EUINavigation::Invalid)
		{
			Direction = NavConfig->GetNavigationDirectionFromAnalogKey(InKeyEvent);
		}
		if (Direction != EUINavigation::Invalid)
		{
			Widget->UINavPC->NotifyNavigationKeyReleased(InKeyEvent.GetKey(), Direction);
			if (bHandleReply)
			{
				Reply = FReply::Handled();
			}
		}
	}
}

void UUINavWidget::HandleSelectorMovement(const float DeltaTime)
{
	if (MoveCurve == nullptr) return;

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

void UUINavWidget::GoToNextSection()
{
	if (!IsValid(UINavSwitcher))
	{
		return;
	}

	const int32 ActiveIndex = UINavSwitcher->GetActiveWidgetIndex();
	const int32 NumSections = UINavSwitcher->GetChildrenCount();
	if (ActiveIndex == NumSections - 1)
	{
		GoToSection(0);
	}

	GoToSection(ActiveIndex + 1);
}

void UUINavWidget::GoToPreviousSection()
{
	if (!IsValid(UINavSwitcher))
	{
		return;
	}

	const int32 ActiveIndex = UINavSwitcher->GetActiveWidgetIndex();
	const int32 NumSections = UINavSwitcher->GetChildrenCount();
	if (ActiveIndex == 0)
	{
		GoToSection(NumSections - 1);
	}

	GoToSection(ActiveIndex - 1);
}

void UUINavWidget::GoToSection(const int32 SectionIndex)
{
	if (!IsValid(UINavSwitcher) ||
		!IsValid(UINavSwitcher->GetWidgetAtIndex(SectionIndex)) ||
		UINavSwitcher->GetActiveWidgetIndex() == SectionIndex ||
		!SectionWidgets.IsValidIndex(SectionIndex))
	{
		return;
	}
	
	const int32 OldIndex = UINavSwitcher->GetActiveWidgetIndex();
	UINavSwitcher->SetActiveWidgetIndex(SectionIndex);
	UWidget* TargetWidget = SectionWidgets[SectionIndex];
	if (IsValid(TargetWidget))
	{
		TargetWidget->SetFocus();
	}
	OnChangedSection(OldIndex, SectionIndex);
}

void UUINavWidget::OnSectionButtonPressed1()
{
	GoToSection(0);
}

void UUINavWidget::OnSectionButtonPressed2()
{
	GoToSection(1);
}

void UUINavWidget::OnSectionButtonPressed3()
{
	GoToSection(2);
}

void UUINavWidget::OnSectionButtonPressed4()
{
	GoToSection(3);
}

void UUINavWidget::OnSectionButtonPressed5()
{
	GoToSection(4);
}

void UUINavWidget::OnSectionButtonPressed6()
{
	GoToSection(5);
}

void UUINavWidget::OnSectionButtonPressed7()
{
	GoToSection(6);
}

void UUINavWidget::OnSectionButtonPressed8()
{
	GoToSection(7);
}

void UUINavWidget::OnSectionButtonPressed9()
{
	GoToSection(8);
}

void UUINavWidget::OnSectionButtonPressed10()
{
	GoToSection(9);
}

void UUINavWidget::UpdateSelectorLocation(UUINavComponent* Component)
{
	if (TheSelector == nullptr || !IsValid(FirstComponent)) return;
	TheSelector->SetRenderTranslation(GetButtonLocation(Component));
}

FVector2D UUINavWidget::GetButtonLocation(UUINavComponent* Component) const
{
	if (!IsValid(Component))
	{
		return FVector2D();
	}

	const FGeometry Geom = Component->NavButton->GetCachedGeometry();
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

void UUINavWidget::ExecuteAnimations(UUINavComponent* FromComponent, UUINavComponent* ToComponent, const bool bHadNavigation, const bool bFinishInstantly /*= false*/)
{
	if (IsValid(FromComponent) &&
		FromComponent != ToComponent &&
		IsValid(FromComponent->GetComponentAnimation()) &&
		FromComponent->UseComponentAnimation() &&
		bHadNavigation &&
		(bForcingNavigation || !IsValid(ToComponent)))
	{
		if (FromComponent->IsAnimationPlaying(FromComponent->GetComponentAnimation()))
		{
			if (bFinishInstantly)
			{
				FromComponent->StopAnimation(FromComponent->GetComponentAnimation());
			}
			else
			{
				FromComponent->ReverseAnimation(FromComponent->GetComponentAnimation());
			}
		}
		else
		{
			if (bFinishInstantly)
			{
				RevertAnimation(FromComponent);
			}
			else
			{
				FromComponent->PlayAnimation(FromComponent->GetComponentAnimation(), 0.0f, 1, EUMGSequencePlayMode::Reverse);
			}
		}
	}

	if (IsValid(ToComponent) &&
		IsValid(ToComponent->GetComponentAnimation()) &&
		ToComponent->UseComponentAnimation())
	{
		if (ToComponent->IsAnimationPlaying(ToComponent->GetComponentAnimation()))
		{
			ToComponent->ReverseAnimation(ToComponent->GetComponentAnimation());
		}
		else
		{
			ToComponent->PlayAnimation(ToComponent->GetComponentAnimation(), 0.0f, 1, EUMGSequencePlayMode::Forward);
		}
	}
}

void UUINavWidget::RevertAnimation(UUINavComponent* Component)
{
	if (IsValid(Component) && IsValid(Component->GetComponentAnimation()) && Component->UseComponentAnimation())
	{
		Component->PlayAnimation(Component->GetComponentAnimation(), 0.0f, 1, EUMGSequencePlayMode::Reverse);
		Component->SetAnimationCurrentTime(Component->GetComponentAnimation(), 0.0f);
	}
}

void UUINavWidget::UpdateButtonStates(UUINavComponent* Component)
{
	if (IsValid(CurrentComponent))
	{
		CurrentComponent->SwitchButtonStyle(EButtonStyle::Normal);
	}
	if (IsValid(Component))
	{
		Component->SwitchButtonStyle(EButtonStyle::Hovered);
	}
	else if (IsValid(CurrentComponent))
	{
		CurrentComponent->RevertButtonStyle();
	}
}

void UUINavWidget::UpdateTextColor(UUINavComponent* Component)
{
	if (IsValid(CurrentComponent))
	{
		CurrentComponent->SwitchTextColorToDefault();
	}
	if (IsValid(Component))
	{
		Component->SwitchTextColorToNavigated();
	}
}

void UUINavWidget::SetSelectorScale(FVector2D NewScale)
{
	if (TheSelector == nullptr) return;
	TheSelector->SetRenderScale(NewScale);
}

void UUINavWidget::SetSelectorVisible(const bool bVisible)
{
	bShowSelector = bVisible;
	ToggleSelectorVisibility(bVisible);
}

void UUINavWidget::ToggleSelectorVisibility(const bool bVisible)
{
	if (TheSelector == nullptr || (bVisible && !bShowSelector)) return;

	const ESlateVisibility Vis = bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
	TheSelector->SetVisibility(Vis);
}

bool UUINavWidget::IsSelectorVisible()
{
	if (TheSelector == nullptr) return false;
	return bShowSelector && TheSelector->GetVisibility() == ESlateVisibility::HitTestInvisible;
}

void UUINavWidget::OnNavigate_Implementation(UUINavComponent* FromComponent, UUINavComponent* TomComponent)
{

}

void UUINavWidget::OnSelect_Implementation(UUINavComponent* Component)
{

}

void UUINavWidget::PropagateOnSelect(UUINavComponent* Component)
{
	OnSelect(Component);
	if (IsValid(OuterUINavWidget) && !OuterUINavWidget->bMaintainNavigationForChild)
	{
		OuterUINavWidget->PropagateOnSelect(Component);
	}
}

void UUINavWidget::OnStartSelect_Implementation(UUINavComponent* Component)
{

}

void UUINavWidget::PropagateOnStartSelect(UUINavComponent* Component)
{
	OnStartSelect(Component);
	if (IsValid(OuterUINavWidget) && !OuterUINavWidget->bMaintainNavigationForChild)
	{
		OuterUINavWidget->PropagateOnStartSelect(Component);
	}
}

void UUINavWidget::OnStopSelect_Implementation(UUINavComponent* Component)
{

}

void UUINavWidget::PropagateOnStopSelect(UUINavComponent* Component)
{
	OnStopSelect(Component);
	if (IsValid(OuterUINavWidget) && !OuterUINavWidget->bMaintainNavigationForChild)
	{
		OuterUINavWidget->PropagateOnStopSelect(Component);
	}
}

void UUINavWidget::UpdateNavigationVisuals(UUINavComponent* Component, const bool bHadNavigation, const bool bBypassForcedNavigation /*= false*/, const bool bFinishInstantly /*= false*/)
{
	ToggleSelectorVisibility(IsValid(Component));

	if (IsValid(Component) && TheSelector != nullptr && TheSelector->GetIsEnabled())
	{
		UpdateSelectorPrevComponent = CurrentComponent;
		UpdateSelectorNextComponent = Component;
		UpdateSelectorWaitForTick = 0;
	}

	UpdateTextColor(Component);

	UpdateButtonStates(Component);

	ExecuteAnimations(CurrentComponent, Component, bHadNavigation, bFinishInstantly);
}

void UUINavWidget::BeginSelectorMovement(UUINavComponent* FromComponent, UUINavComponent* ToComponent)
{
	if (MoveCurve == nullptr) return;

	SelectorOrigin = (bMovingSelector || !IsValid(FromComponent)) ? TheSelector->GetRenderTransform().Translation : GetButtonLocation(FromComponent);
	SelectorDestination = GetButtonLocation(ToComponent);
	Distance = SelectorDestination - SelectorOrigin;

	float MinTime, MaxTime;
	MoveCurve->GetTimeRange(MinTime, MaxTime);
	MovementTime = MaxTime - MinTime;
	MovementCounter = 0.0f;

	bMovingSelector = true;
}

void UUINavWidget::AttemptUnforceNavigation(const EInputType NewInputType)
{
	if (!GetDefault<UUINavSettings>()->bForceNavigation && NewInputType == EInputType::Mouse)
	{
		if (IsValid(HoveredComponent))
		{
			if (HoveredComponent != CurrentComponent)
			{
				HoveredComponent->SetFocus();
			}
		}
		else if (bForcingNavigation)
		{
			UnforceNavigation(true);
		}
	}
}

void UUINavWidget::ForceNavigation()
{
	bForcingNavigation = true;
	UpdateNavigationVisuals(CurrentComponent, true);
	if (IsValid(CurrentComponent))
	{
		CurrentComponent->SwitchButtonStyle(EButtonStyle::Hovered);
	}
}

void UUINavWidget::UnforceNavigation(const bool bHadNavigation)
{
	bForcingNavigation = false;
	UpdateNavigationVisuals(nullptr, bHadNavigation);
	if (IsValid(CurrentComponent))
	{
		CurrentComponent->RevertButtonStyle();
	}
}

void UUINavWidget::OnReturn_Implementation()
{
	if(GetDefault<UUINavSettings>()->bRemoveWidgetOnReturn) ReturnToParent();
}

void UUINavWidget::OnNext_Implementation()
{
	if (IsValid(UINavSwitcher))
	{
		GoToNextSection();
	}
}

void UUINavWidget::PropagateOnNext()
{
	OnNext();
	if (IsValid(OuterUINavWidget) && !OuterUINavWidget->bMaintainNavigationForChild)
	{
		OuterUINavWidget->PropagateOnNext();
	}
}

void UUINavWidget::OnPrevious_Implementation()
{
	if (IsValid(UINavSwitcher))
	{
		GoToPreviousSection();
	}
}

void UUINavWidget::PropagateOnPrevious()
{
	OnPrevious();
	if (IsValid(OuterUINavWidget) && !OuterUINavWidget->bMaintainNavigationForChild)
	{
		OuterUINavWidget->PropagateOnPrevious();
	}
}

void UUINavWidget::OnChangedSection_Implementation(const int32 FromIndex, const int32 ToIndex)
{

}

void UUINavWidget::OnInputChanged_Implementation(const EInputType From, const EInputType To)
{

}

void UUINavWidget::PropagateOnInputChanged(const EInputType From, const EInputType To)
{
	OnInputChanged(From, To);
	if (IsValid(OuterUINavWidget) && !OuterUINavWidget->bMaintainNavigationForChild)
	{
		OuterUINavWidget->PropagateOnInputChanged(From, To);
	}
}

void UUINavWidget::PreSetup_Implementation(const bool bFirstSetup)
{

}

void UUINavWidget::OnSetupCompleted_Implementation()
{

}

void UUINavWidget::PropagateOnSetupCompleted()
{
	OnSetupCompleted();

	for (UUINavWidget* ChildUINavWidget : ChildUINavWidgets)
	{
		ChildUINavWidget->PropagateOnSetupCompleted();
	}
}

void UUINavWidget::OnHorizCompNavigateLeft_Implementation(UUINavComponent* Component)
{

}

void UUINavWidget::PropagateOnHorizCompNavigateLeft(UUINavComponent* Component)
{
	OnHorizCompNavigateLeft(Component);
	if (IsValid(OuterUINavWidget) && !OuterUINavWidget->bMaintainNavigationForChild)
	{
		OuterUINavWidget->PropagateOnHorizCompNavigateLeft(Component);
	}
}

void UUINavWidget::OnHorizCompNavigateRight_Implementation(UUINavComponent* Component)
{

}

void UUINavWidget::PropagateOnHorizCompNavigateRight(UUINavComponent* Component)
{
	OnHorizCompNavigateRight(Component);
	if (IsValid(OuterUINavWidget) && !OuterUINavWidget->bMaintainNavigationForChild)
	{
		OuterUINavWidget->PropagateOnHorizCompNavigateRight(Component);
	}
}

void UUINavWidget::OnHorizCompUpdated_Implementation(UUINavComponent* Component)
{

}

void UUINavWidget::PropagateOnHorizCompUpdated(UUINavComponent* Component)
{
	OnHorizCompUpdated(Component);
	if (IsValid(OuterUINavWidget) && !OuterUINavWidget->bMaintainNavigationForChild)
	{
		OuterUINavWidget->PropagateOnHorizCompUpdated(Component);
	}
}

UUINavWidget* UUINavWidget::GoToWidget(TSubclassOf<UUINavWidget> NewWidgetClass, const bool bRemoveParent /*= true*/, const bool bDestroyParent, const int ZOrder)
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

UUINavWidget* UUINavWidget::GoToPromptWidget(TSubclassOf<UUINavPromptWidget> NewWidgetClass, const FPromptWidgetDecided& Event, const FText Title, const FText Message, const bool bRemoveParent /*= false*/, const int ZOrder /*= 0*/)
{
	if (NewWidgetClass == nullptr)
	{
		DISPLAYERROR("GoToPromptWidget: No Widget Class found");
		return nullptr;
	}

	if (!Event.IsBound())
	{
		DISPLAYERROR("GoToPromptWidget: Event isn't bound");
		return nullptr;
	}

	APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
	UUINavPromptWidget* NewWidget = CreateWidget<UUINavPromptWidget>(PC, NewWidgetClass);
	NewWidget->Title = Title;
	NewWidget->Message = Message;
	NewWidget->SetCallback(Event);
	return GoToBuiltWidget(NewWidget, bRemoveParent, false, ZOrder);
}

UUINavWidget * UUINavWidget::GoToBuiltWidget(UUINavWidget* NewWidget, const bool bRemoveParent, const bool bDestroyParent, const int ZOrder)
{
	if (NewWidget == nullptr) return nullptr;

	UUINavWidget* OldOuterUINavWidget = GetMostOuterUINavWidget();
	UUINavWidget* NewOuterUINavWidget = NewWidget->GetMostOuterUINavWidget();

	if (IsValid(HoveredComponent))
	{
		IgnoreHoverComponent = HoveredComponent;
	}
	
	if (OuterUINavWidget != nullptr || NewOuterUINavWidget == this)
	{
		if (NewOuterUINavWidget == OldOuterUINavWidget)
		{
			NewWidget->SetFocus();
			return NewWidget;
		}
	}
	
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
		if (!bForceUsePlayerScreen && (!bUsingSplitScreen || NewWidget->bUseFullscreenWhenSplitscreen)) NewWidget->AddToViewport(ZOrder);
		else NewWidget->AddToPlayerScreen(ZOrder);

		APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
		NewWidget->SetUserFocus(PC);
		if (UINavPC->GetInputMode() == EInputMode::UI)
		{
			NewWidget->SetKeyboardFocus();
		}
	}
	CleanSetup();
	
	SelectCount = 0;
	SetSelectedComponent(nullptr);

	SetHoveredComponent(nullptr);

	return NewWidget;
}

void UUINavWidget::ReturnToParent(const bool bRemoveAllParents, const int ZOrder)
{
 	if (ParentWidget == nullptr)
	{
		if (bAllowRemoveIfRoot && UINavPC != nullptr)
		{
			UINavPC->SetActiveWidget(nullptr);

			SelectCount = 0;
			SetSelectedComponent(nullptr);
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
	SetSelectedComponent(nullptr);
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
					ParentWidget->ReturnedFromWidget = this;
					ParentWidget->ReconfigureSetup();
				}
				bReturningToParent = true;
				RemoveFromParent();
			}
		}
		else
		{
			OuterUINavWidget->ReturnToParent(bRemoveAllParents, ZOrder);
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

void UUINavWidget::NavigatedTo(UUINavComponent* NavigatedToComponent, const bool bNotifyUINavPC /*= true*/)
{
	if (!IsValid(UINavPC) ||
		(CurrentComponent == NavigatedToComponent && UINavPC->GetActiveSubWidget() == this))
	{
		return;
	}

	const bool bHadNavigation = bHasNavigation;

	if (bNotifyUINavPC)
	{
		UINavPC->NotifyNavigatedTo(this);
	}

	UINavPC->CancelRebind();

	if (IsValid(OuterUINavWidget) && !OuterUINavWidget->bMaintainNavigationForChild)
	{
		OnNavigate(CurrentComponent, NavigatedToComponent);

		CurrentComponent = NavigatedToComponent;

		OuterUINavWidget->NavigatedTo(NavigatedToComponent, false);
		return;
	}

	if (bForcingNavigation || (CurrentComponent != NavigatedToComponent && CurrentComponent != nullptr))
	{
		UpdateNavigationVisuals(NavigatedToComponent, !bHoverRestoredNavigation);
	}
	else
	{
		ToggleSelectorVisibility(bForcingNavigation || IsValid(HoveredComponent));
		RevertAnimation(CurrentComponent);
	}

	if (!bForcingNavigation)
	{
		bForcingNavigation = true;
	}

	CallOnNavigate(bHadNavigation == bHasNavigation ? CurrentComponent : nullptr, NavigatedToComponent);

	SetCurrentComponent(NavigatedToComponent);

	if (bHoverRestoredNavigation)
	{
		bHoverRestoredNavigation = false;
	}
}

void UUINavWidget::CallOnNavigate(UUINavComponent* FromComponent, UUINavComponent* ToComponent)
{
	OnNavigate(FromComponent, ToComponent);

	if (IsValid(FromComponent))
	{
		FromComponent->OnNavigatedFrom();
		FromComponent->OnNavigatedFromEvent.Broadcast();
		FromComponent->OnNativeNavigatedFromEvent.Broadcast();
		FromComponent->ExecuteComponentActions(EComponentAction::OnNavigatedFrom);
	}

	if (IsValid(ToComponent))
	{
		USoundBase* NavigatedSound = ToComponent->GetOnNavigatedSound();
		if (NavigatedSound != nullptr)
		{
			PlaySound(NavigatedSound);
		}
		ToComponent->OnNavigatedTo();
		ToComponent->OnNavigatedToEvent.Broadcast();
		ToComponent->OnNativeNavigatedToEvent.Broadcast();
		ToComponent->ExecuteComponentActions(EComponentAction::OnNavigatedTo);
	}
}

void UUINavWidget::StartedSelect()
{
	PropagateOnStartSelect(CurrentComponent);
}

void UUINavWidget::StoppedSelect()
{
	if (SelectedComponent == CurrentComponent)
	{
		PropagateOnSelect(CurrentComponent);
	}
	PropagateOnStopSelect(CurrentComponent);
}

void UUINavWidget::StartedReturn()
{
	SetPressingReturn(true);
	if (GetDefault<UUINavSettings>()->bReturnOnPress)
	{
		ExecuteReturn(/*bPress*/ true);
	}
}

void UUINavWidget::StoppedReturn()
{
	if (!GetDefault<UUINavSettings>()->bReturnOnPress)
	{
		ExecuteReturn(/*bPress*/ false);
	}

	SetPressingReturn(false);
}

void UUINavWidget::ExecuteReturn(const bool bPress)
{
	if (!IsValid(UINavPC))
	{
		return;
	}

	if (OuterUINavWidget != nullptr)
	{
		if (OnChildReturn())
		{
			OuterUINavWidget->ExecuteReturn(bPress);
		}
		else
		{
			IUINavPCReceiver::Execute_OnReturn(UINavPC->GetOwner());
		}
	}
	else if (bPress || bPressingReturn)
	{
		OnReturn();
		IUINavPCReceiver::Execute_OnReturn(UINavPC->GetOwner());
	}
}

bool UUINavWidget::TryConsumeNavigation()
{
	if (!bForcingNavigation && !GetDefault<UUINavSettings>()->bForceNavigation)
	{
		ForceNavigation();
		return true;
	}

	return IsValid(SelectedComponent);
}

bool UUINavWidget::IsBeingRemoved() const
{
	if (!IsValid(OuterUINavWidget))
	{
		return bBeingRemoved;
	}

	return OuterUINavWidget->IsBeingRemoved();
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

UUINavWidget* UUINavWidget::GetChildUINavWidget(const int ChildIndex) const
{
	return ChildIndex < ChildUINavWidgets.Num() ? ChildUINavWidgets[ChildIndex] : nullptr;
}

EThumbstickAsMouse UUINavWidget::GetUseThumbstickAsMouse() const
{
	if (UseThumbstickAsMouse != EThumbstickAsMouse::None)
	{
		return UseThumbstickAsMouse;
	}

	if (IsValid(OuterUINavWidget))
	{
		return OuterUINavWidget->GetUseThumbstickAsMouse();
	}

	return EThumbstickAsMouse::None;
}

void UUINavWidget::AddParentToPath(const int IndexInParent)
{
	UINavWidgetPath.EmplaceAt(0, IndexInParent);

	for (UUINavWidget* ChildUINavWidget : ChildUINavWidgets)
	{
		ChildUINavWidget->AddParentToPath(IndexInParent);
	}
}

void UUINavWidget::SetFirstComponent(UUINavComponent* Component)
{
	if (IsValid(FirstComponent))
	{
		return;
	}

	FirstComponent = Component;

	if (IsValid(OuterUINavWidget) && !IsValid(OuterUINavWidget->GetFirstComponent()))
	{
		OuterUINavWidget->SetFirstComponent(Component);
	}
}

void UUINavWidget::RemovedComponent(UUINavComponent* Component)
{
	if (IsValid(Component))
	{
		if (Component == SelectedComponent)
		{
			OnReleasedComponent(SelectedComponent);
		}

		if (Component == CurrentComponent)
		{
			SetCurrentComponent(nullptr);
		}
	}
	
	if (IsValid(FirstComponent) && Component == FirstComponent)
	{
		FirstComponent = nullptr;
	}
}

bool UUINavWidget::IsSelectorValid()
{
	return TheSelector != nullptr && TheSelector->GetIsEnabled() && bShowSelector;
}

void UUINavWidget::OnHoveredComponent(UUINavComponent* Component)
{
	if (!IsValid(Component) || UINavPC == nullptr || (UINavPC->HidingMouseCursor() && !UINavPC->OverrideConsiderHover())) return;

	UINavPC->CancelRebind();

	SetHoveredComponent(Component);

	if (Component == CurrentComponent && UINavPC->GetActiveSubWidget() == this)
	{
		Component->RevertButtonStyle();
	}

	if (!bForcingNavigation)
	{
		bForcingNavigation = true;

		if (Component != CurrentComponent)
		{
			bHoverRestoredNavigation = true;
		}
		else
		{
			UpdateNavigationVisuals(CurrentComponent, false, true);
		}
	}

	if (Component != CurrentComponent || UINavPC->GetActiveSubWidget() != this)
	{
		Component->SetFocus();
	}
}

void UUINavWidget::OnUnhoveredComponent(UUINavComponent* Component)
{
	if (!IsValid(Component)) return;

	if (IgnoreHoverComponent == nullptr || IgnoreHoverComponent != Component)
	{
		SetHoveredComponent(nullptr);
	}

	if (!GetDefault<UUINavSettings>()->bForceNavigation)
	{
		UnforceNavigation(true);
	}
	else
	{
		if (SelectedComponent != Component)
		{
			Component->SwitchButtonStyle(Component == CurrentComponent ? EButtonStyle::Hovered : EButtonStyle::Normal);
		}

		if (CurrentComponent == Component && (IgnoreHoverComponent == nullptr || IgnoreHoverComponent != Component))
		{
			Component->SetFocus();
		}
		else
		{
			Component->RevertButtonStyle();
		}
	}
}

void UUINavWidget::OnPressedComponent(UUINavComponent* Component)
{
	if (!IsValid(Component) || UINavPC == nullptr) return;

	if (!UINavPC->AllowsSelectInput() || !UINavPC->IsWidgetActive(this)) return;

	SetSelectedComponent(Component);

	SelectCount++;

	PropagateOnStartSelect(CurrentComponent);
}

void UUINavWidget::OnReleasedComponent(UUINavComponent* Component)
{
	if (!IsValid(Component) || (!bHasNavigation && SelectCount == 0) || UINavPC == nullptr) return;

	if (!Component->NavButton->IsHovered()) Component->RevertButtonStyle();

	if (CurrentComponent == nullptr || SelectedComponent == nullptr || !IsValid(Component)) return;

	const bool bIsSelectedButton = SelectedComponent == Component;

	if (IsValid(Component))
	{
		Component->SwitchButtonStyle(Component->NavButton->IsPressed() || SelectCount > 1 ? EButtonStyle::Pressed : (Component == CurrentComponent ? EButtonStyle::Hovered : EButtonStyle::Normal));

		if (SelectCount > 0) SelectCount--;
		if (SelectCount == 0)
		{
			SetSelectedComponent(nullptr);

			if (bIsSelectedButton)
			{
				PropagateOnSelect(Component);
			}
			PropagateOnStopSelect(Component);
		}
	}

	if (Component != CurrentComponent) Component->SetFocus();
}
