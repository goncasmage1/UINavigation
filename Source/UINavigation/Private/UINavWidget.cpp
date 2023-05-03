// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavWidget.h"
#include "UINavButton.h"
#include "UINavHorizontalComponent.h"
#include "UINavComponent.h"
#include "UINavInputBox.h"
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
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/CanvasPanelSlot.h"
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
	OuterUINavWidget = GetOuterObject<UUINavWidget>(this);
	if (OuterUINavWidget != nullptr)
	{
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

	TraverseHierarchy();

	//If this widget doesn't need to create the selector, skip to setup
	if (!IsSelectorValid())
	{
		UINavSetup();
	}
	else
	{
		SetupSelector();
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			UINavSetup();
		});
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
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			UINavSetup();
		});
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

		UUINavButton* UINavButton = Cast<UUINavButton>(Widget);
		if (UINavButton != nullptr)
		{
			DISPLAYERROR_STATIC(this, "This widget has UINavButtons outside a UINavComponent!");
		}
	}
}

void UUINavWidget::SetEnableUINavButtons(const bool bEnable, const bool bRecursive)
{
	/*for (UUINavButton* Button : UINavButtons)
	{
		if (Button->bAutoCollapse)
		{
			Button->SetIsEnabled(bEnable);
		}
	}*/

	if (!bRecursive) return;
	
	for (UUINavWidget* ChildUINavWidget : ChildUINavWidgets)
	{
		ChildUINavWidget->SetEnableUINavButtons(bEnable, bRecursive);
	}
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

	if (OuterUINavWidget == nullptr)
	{
		UINavPC->SetActiveWidget(this);
	}

	//Re-enable all buttons (bug fix)
	if (OuterUINavWidget == nullptr)
	{
		SetEnableUINavButtons(true, true);
	}

	bCompletedSetup = true;

	UUINavComponent* InitialComponent = GetInitialFocusComponent();
	if (IsValid(InitialComponent))
	{
		InitialComponent->SetFocus();
		if (!GetDefault<UUINavSettings>()->bForceNavigation && !IsValid(HoveredComponent))
		{
			UnforceNavigation();
		}
	}

	OnSetupCompleted();

	if (PromptWidgetClass != nullptr)
	{
		OnPromptDecided(PromptWidgetClass, PromptData);
	}
}

UUINavComponent* UUINavWidget::GetInitialFocusComponent_Implementation()
{
	return FirstComponent;
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

		if (IsSelectorValid())
		{
			TheSelector->SetVisibility(ESlateVisibility::HitTestInvisible);
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

	if (bNewWidgetIsChild && !bMaintainNavigationForChild)
	{
		return;
	}

	if (NewActiveWidget == nullptr ||
		(!bNewWidgetIsChild && NewActiveWidget->bMaintainNavigationForChild))
	{
		UpdateNavigationVisuals(nullptr);
	}

	CallOnNavigate(CurrentComponent, nullptr);

	bHasNavigation = false;

	if (!bNewWidgetIsChild) CurrentComponent = nullptr;

	OnLostNavigation(NewActiveWidget, bNewWidgetIsChild);
}

void UUINavWidget::OnLostNavigation_Implementation(UUINavWidget* NewActiveWidget, const bool bToChild)
{
}

void UUINavWidget::SetCurrentComponent(UUINavComponent* Component)
{
	CurrentComponent = Component;

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

void UUINavWidget::NativeTick(const FGeometry & MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	if (IsSelectorValid())
	{
		if (bShouldTickUpdateSelector)
		{
			if (MoveCurve != nullptr) BeginSelectorMovement(UpdateSelectorPrevComponent, UpdateSelectorNextComponent);
			else UpdateSelectorLocation(UpdateSelectorNextComponent);
			bShouldTickUpdateSelector = false;
		}

		if (bMovingSelector)
		{
			HandleSelectorMovement(DeltaTime);
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

void UUINavWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	bDestroying = true;
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

FReply UUINavWidget::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	FReply Reply = Super::NativeOnFocusReceived(InGeometry, InFocusEvent);

	if (IsValid(CurrentComponent))
	{
		CurrentComponent->SetFocus();
		return Reply;
	}

	UUINavComponent* InitialComponent = GetInitialFocusComponent();
	if (IsValid(InitialComponent))
	{
		InitialComponent->SetFocus();
		if (!GetDefault<UUINavSettings>()->bForceNavigation && !IsValid(HoveredComponent))
		{
			UnforceNavigation();
		}
	}

	return Reply;
}

void UUINavWidget::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusLost(InFocusEvent);
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
		/*if (HaltedIndex != -1)
		{
			if (HaltedIndex == SELECT_INDEX) OnPreSelect(ButtonIndex);
			else if (HaltedIndex == RETURN_INDEX)
			{
				OnReturn();
			}
			else NavigateTo(HaltedIndex);

			HaltedIndex = -1;
		}*/
		return;
	}

	TheSelector->SetRenderTranslation(SelectorOrigin + Distance*MoveCurve->GetFloatValue(MovementCounter));
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

void UUINavWidget::ExecuteAnimations(UUINavComponent* FromComponent, UUINavComponent* ToComponent)
{
	if (IsValid(FromComponent) && FromComponent != ToComponent && IsValid(FromComponent->GetComponentAnimation()) && FromComponent->UseComponentAnimation())
	{
		if (FromComponent->IsAnimationPlaying(FromComponent->GetComponentAnimation()))
		{
			FromComponent->ReverseAnimation(FromComponent->GetComponentAnimation());
		}
		else
		{
			FromComponent->PlayAnimation(FromComponent->GetComponentAnimation(), 0.0f, 1, EUMGSequencePlayMode::Reverse);
		}
	}

	if (IsValid(ToComponent) && IsValid(ToComponent->GetComponentAnimation()) && ToComponent->UseComponentAnimation())
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

void UUINavWidget::OnPromptDecided(const TSubclassOf<UUINavPromptWidget> PromptClass, const UPromptDataBase* const InPromptData)
{
	PromptWidgetClass = nullptr;

	FString ClassString = PromptClass->GetFName().ToString();
	ClassString.RemoveAt(ClassString.Len() - 2, 2);
	const FName EventName = FName(*(ClassString.Append(TEXT("_Decided"))));
	UFunction* CustomFunction = FindFunction(EventName);
	if (CustomFunction != nullptr)
	{
		if (CustomFunction->ParmsSize == sizeof(UPromptDataBase*))
		{
			uint8* Buffer = static_cast<uint8*>(FMemory_Alloca(sizeof(UPromptDataBase*)));
			FMemory::Memcpy(Buffer, &InPromptData, sizeof(InPromptData));
			ProcessEvent(CustomFunction, Buffer);
		}
		else
		{
			DISPLAYERROR(FString::Printf(TEXT("%s Prompt Event could not be found!"), *EventName.ToString()));
		}
	}
}

void UUINavWidget::UpdateNavigationVisuals(UUINavComponent* Component, const bool bBypassForcedNavigation /*= false*/)
{
	if (IsValid(Component) && IsSelectorValid())
	{
		UpdateSelectorPrevComponent = CurrentComponent;
		UpdateSelectorNextComponent = Component;
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			bShouldTickUpdateSelector = true;
		});
	}

	UpdateTextColor(Component);

	ExecuteAnimations(CurrentComponent, Component);
}

void UUINavWidget::BeginSelectorMovement(UUINavComponent* FromComponent, UUINavComponent* ToComponent)
{
	if (MoveCurve == nullptr) return;

	SelectorOrigin = (bMovingSelector || !IsValid(FromComponent)) ? TheSelector->RenderTransform.Translation : GetButtonLocation(FromComponent);
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
		else
		{
			UnforceNavigation();
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

void UUINavWidget::UnforceNavigation()
{
	bForcingNavigation = false;
	UpdateNavigationVisuals(nullptr, true);
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

void UUINavWidget::OnHorizCompNavigateLeft_Implementation(UUINavComponent* Component)
{

}

void UUINavWidget::OnHorizCompNavigateRight_Implementation(UUINavComponent* Component)
{

}

void UUINavWidget::OnHorizCompUpdated_Implementation(UUINavComponent* Component)
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
			return NewWidget;
		}
		
		/*if (OuterUINavWidget != nullptr)
		{
			OldOuterUINavWidget->PreviousNestedWidget = this;
		}*/
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
					/*if (ParentWidget->PreviousNestedWidget != nullptr)
					{
						UINavPC->SetActiveNestedWidget(ParentWidget->PreviousNestedWidget);
						ParentWidget->PreviousNestedWidget = nullptr;
					}*/
				}
				bReturningToParent = true;
				RemoveFromParent();
			}
		}
		else
		{
			OuterUINavWidget->SetFocus();
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

	if (IsValid(OuterUINavWidget) && !OuterUINavWidget->bMaintainNavigationForChild)
	{
		OnNavigate(CurrentComponent, NavigatedToComponent);

		CurrentComponent = NavigatedToComponent;

		OuterUINavWidget->NavigatedTo(NavigatedToComponent, false);
		return;
	}

	if (CurrentComponent != NavigatedToComponent)
	{
		UpdateNavigationVisuals(NavigatedToComponent);
	}

	CallOnNavigate(bHadNavigation == bHasNavigation ? CurrentComponent : nullptr, NavigatedToComponent);

	SetCurrentComponent(NavigatedToComponent);
}

void UUINavWidget::CallOnNavigate(UUINavComponent* FromComponent, UUINavComponent* ToComponent)
{
	OnNavigate(FromComponent, ToComponent);

	if (IsValid(FromComponent))
	{
		FromComponent->OnNavigatedFrom();
	}

	if (IsValid(ToComponent))
	{
		ToComponent->OnNavigatedTo();
	}
}

void UUINavWidget::StartedSelect()
{
	PropagateOnStartSelect(CurrentComponent);
}

void UUINavWidget::StoppedSelect()
{
	PropagateOnSelect(CurrentComponent);
	PropagateOnStopSelect(CurrentComponent);
}

void UUINavWidget::StartedReturn()
{
}

void UUINavWidget::StoppedReturn()
{
	if (IsValid(UINavPC))
	{
		OnReturn();
	}
}

bool UUINavWidget::TryConsumeNavigation()
{
	if (!GetDefault<UUINavSettings>()->bForceNavigation && !bForcingNavigation)
	{
		ForceNavigation();
		return true;
	}

	return false;
}

template<typename T>
T* UUINavWidget::GetOuterObject(const UObject* const Object)
{
	if (!IsValid(Object))
	{
		return nullptr;
	}

	T* OuterObject = Cast<T>(Object->GetOuter());
	if (OuterObject != nullptr)
	{
		return OuterObject;
	}

	return GetOuterObject<T>(Object->GetOuter());
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

bool UUINavWidget::IsSelectorValid()
{
	return  TheSelector != nullptr && TheSelector->bIsEnabled;
}

void UUINavWidget::OnHoveredComponent(UUINavComponent* Component)
{
	if (!IsValid(Component)) return;

	if (UINavPC == nullptr)
	{
		return;
	}

	UINavPC->CancelRebind();

	SetHoveredComponent(Component);

	const bool bUsingLeftStickAsMouse = bUseLeftThumbstickAsMouse || UINavPC->bUseLeftThumbstickAsMouse || UINavPC->IsMovingLeftStick();
#if IS_VR_PLATFORM 
	const bool bIsVR = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
#else
	const bool bIsVR = false;
#endif
	if (!bUsingLeftStickAsMouse && !bIsVR && Component == CurrentComponent && UINavPC->GetActiveSubWidget() == this)
	{
		Component->RevertButtonStyle();
	}

	if (!bForcingNavigation)
	{
		bForcingNavigation = true;
		if (Component == CurrentComponent)
		{
			UpdateNavigationVisuals(CurrentComponent, true);
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

	UINavPC->CancelRebind();

	SetHoveredComponent(nullptr);

	if (!GetDefault<UUINavSettings>()->bForceNavigation)
	{
		UnforceNavigation();
	}
	else
	{
		if (SelectedComponent != Component)
		{
			Component->SwitchButtonStyle(Component == CurrentComponent ? EButtonStyle::Hovered : EButtonStyle::Normal);
		}

		if (Component->ForcedStyle != EButtonStyle::None)
		{
			Component->SetFocus();
		}
	}
}

void UUINavWidget::OnPressedComponent(UUINavComponent* Component)
{
	if (!IsValid(Component) || UINavPC == nullptr) return;

	if (!UINavPC->AllowsSelectInput()) return;

	SetSelectedComponent(Component);

	SelectCount++;

	PropagateOnStartSelect(CurrentComponent);
}

void UUINavWidget::OnReleasedComponent(UUINavComponent* Component)
{
	if (!IsValid(Component) || (!bHasNavigation && SelectCount == 0) || UINavPC == nullptr) return;

	/*if (bMovingSelector)
	{
		HaltedIndex = SELECT_INDEX;
		return;
	}*/

	if (!Component->NavButton->IsHovered()) Component->RevertButtonStyle();

	if (CurrentComponent == nullptr || SelectedComponent == nullptr || !IsValid(Component)) return;

	const bool bIsSelectedButton = SelectedComponent == Component && (Component->NavButton->IsHovered());

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
