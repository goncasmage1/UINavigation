// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavPCComponent.h"
#include "UINavWidget.h"
#include "UINavComponent.h"
#include "UINavSettings.h"
#include "UINavDefaultInputSettings.h"
#include "UINavPCReceiver.h"
#include "UINavInputContainer.h"
#include "UINavMacros.h"
#include "UINavInputBox.h"
#include "UINavigationConfig.h"
#include "GameFramework/InputSettings.h"
#include "Data/AxisType.h"
#include "Data/InputIconMapping.h"
#include "Data/InputNameMapping.h"
#include "UINavBlueprintFunctionLibrary.h"
#include "UINavInputProcessor.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "InputCoreTypes.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputLibrary.h"
#include "EnhancedPlayerInput.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Templates/SharedPointer.h"
#include "Engine/GameViewportClient.h"
#include "Internationalization/Internationalization.h"

const FKey UUINavPCComponent::MouseUp("MouseUp");
const FKey UUINavPCComponent::MouseDown("MouseDown");
const FKey UUINavPCComponent::MouseRight("MouseRight");
const FKey UUINavPCComponent::MouseLeft("MouseLeft");
bool UUINavPCComponent::bInitialized = false;

UUINavPCComponent::UUINavPCComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = true;

	bAutoActivate = true;
	bCanEverAffectNavigation = false;

	if (!bInitialized)
	{
		bInitialized = true;
		EKeys::AddKey(FKeyDetails(MouseUp, NSLOCTEXT("InputKeys", "MouseUp", "Mouse Up"), FKeyDetails::MouseButton));
		EKeys::AddKey(FKeyDetails(MouseDown, NSLOCTEXT("InputKeys", "MouseDown", "Mouse Down"), FKeyDetails::MouseButton));
		EKeys::AddKey(FKeyDetails(MouseRight, NSLOCTEXT("InputKeys", "MouseRight", "Mouse Right"), FKeyDetails::MouseButton));
		EKeys::AddKey(FKeyDetails(MouseLeft, NSLOCTEXT("InputKeys", "MouseLeft", "Mouse Left"), FKeyDetails::MouseButton));
	}
}

void UUINavPCComponent::Activate(bool bReset)
{
	PC = Cast<APlayerController>(GetOwner());
	if (PC == nullptr)
	{
		DISPLAYERROR(TEXT("UINavPCComponent Owner isn't a Player Controller!"));
		return;
	}

	if (!PC->GetClass()->ImplementsInterface(UUINavPCReceiver::StaticClass()))
	{
		DISPLAYERROR(TEXT("Player Controller doesn't implement UINavPCReceiver interface!"));
		return;
	}
}

void UUINavPCComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PC != nullptr && PC->IsLocalPlayerController() && !SharedInputProcessor.IsValid())
	{
		RefreshNavigationKeys();

		if (IsValid(GetEnhancedInputComponent()))
		{
			const UUINavSettings* const UINavSettings = GetDefault<UUINavSettings>();
			const UInputMappingContext* InputContext = UINavSettings->EnhancedInputContext.LoadSynchronous();
			const UUINavEnhancedInputActions* const InputActions = UINavSettings->EnhancedInputActions.LoadSynchronous();
		
			if (!IsValid(InputContext) ||
				!IsValid(InputActions) ||
				!IsValid(InputActions->IA_MenuUp) ||
				!IsValid(InputActions->IA_MenuDown) ||
				!IsValid(InputActions->IA_MenuLeft) ||
				!IsValid(InputActions->IA_MenuRight) ||
				!IsValid(InputActions->IA_MenuSelect) ||
				!IsValid(InputActions->IA_MenuReturn) ||
				!IsValid(InputActions->IA_MenuNext) ||
				!IsValid(InputActions->IA_MenuPrevious))
			{
				DISPLAYERROR("Not all Enhanced Menu Inputs have been setup!");
				return;
			}
		}
		
		SharedInputProcessor = MakeShareable(new FUINavInputProcessor());
		SharedInputProcessor->SetUINavPC(this);
		FSlateApplication::Get().RegisterInputPreProcessor(SharedInputProcessor);

		CacheGameInputContexts();
		TryResetDefaultInputs();

		IPlatformInputDeviceMapper& PlatformInputMapper = IPlatformInputDeviceMapper::Get();
		if (!PlatformInputMapper.GetOnInputDeviceConnectionChange().IsBoundToObject(this))
		{
			PlatformInputMapper.GetOnInputDeviceConnectionChange().AddUObject(this, &UUINavPCComponent::OnControllerConnectionChanged);
		}
	}
}

void UUINavPCComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PC != nullptr && PC->IsLocalPlayerController())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(SharedInputProcessor);
	}
	
	IPlatformInputDeviceMapper::Get().GetOnInputDeviceConnectionChange().RemoveAll(this);

	Super::EndPlay(EndPlayReason);
}

void UUINavPCComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (CountdownPhase)
	{
		case ECountdownPhase::First:
			TimerCounter += DeltaTime;
			if (TimerCounter >= InputHeldWaitTime)
			{
				NavigateInDirection(CallbackDirection);
				TimerCounter -= InputHeldWaitTime;
				CountdownPhase = ECountdownPhase::Looping;
			}
			break;
		case ECountdownPhase::Looping:
			TimerCounter += DeltaTime;
			if (TimerCounter >= NavigationChainFrequency)
			{
				NavigateInDirection(CallbackDirection);
				TimerCounter -= NavigationChainFrequency;
			}
			break;
	}

	if (!bReceivedAnalogInput)
	{
		ThumbstickDelta = FVector2D::ZeroVector;
	}
	else
	{
		bReceivedAnalogInput = false;
	}

	if (bIgnoreFocusByNavigation)
	{
		bIgnoreFocusByNavigation = false;
	}
}

void UUINavPCComponent::RequestRebuildMappings()
{
	UEnhancedInputLibrary::ForEachSubsystem([](IEnhancedInputSubsystemInterface* Subsystem)
	{
		if (Subsystem)
		{
			Subsystem->RequestRebuildControlMappings();
		}
	});
}

void UUINavPCComponent::OnControllerConnectionChanged(EInputDeviceConnectionState NewConnectionState, FPlatformUserId UserId, FInputDeviceId UserIndex)
{
	IUINavPCReceiver::Execute_OnControllerConnectionChanged(GetOwner(), NewConnectionState == EInputDeviceConnectionState::Connected, static_cast<int32>(UserId), static_cast<int32>(UserIndex.GetId()));
}

void UUINavPCComponent::CacheGameInputContexts()
{
	if (CachedInputContexts.Num() == 0)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> AssetsData;
		AssetRegistryModule.Get().GetAssetsByClass(UInputMappingContext::StaticClass()->GetClassPathName(), AssetsData);
		for (const FAssetData& AssetData : AssetsData)
		{
			const UInputMappingContext* const InputContext = Cast<UInputMappingContext>(AssetData.GetAsset());
			if (!IsValid(InputContext))
			{
				continue;
			}

			CachedInputContexts.Add(InputContext);
		}
	}
}

void UUINavPCComponent::TryResetDefaultInputs()
{
	UUINavDefaultInputSettings* DefaultInputSettings = GetMutableDefault<UUINavDefaultInputSettings>();
	if (DefaultInputSettings->DefaultEnhancedInputMappings.Num() == 0)
	{
		for (const UInputMappingContext* const InputContext : CachedInputContexts)
		{
			DefaultInputSettings->DefaultEnhancedInputMappings.Add(TSoftObjectPtr<UInputMappingContext>(FAssetData(InputContext).ToSoftObjectPath()), InputContext->GetMappings());
		}
		DefaultInputSettings->SaveConfig();
	}
}

void UUINavPCComponent::ProcessRebind(const FKey Key)
{
	if (!IsValid(ListeningInputBox))
	{
		return;
	}

	if (Key == EKeys::LeftMouseButton)
	{
		bIgnoreSelectRelease = true;
	}

	ListeningInputBox->UpdateInputKey(Key);
	ListeningInputBox = nullptr;
}

void UUINavPCComponent::CancelRebind()
{
	if (IsValid(ListeningInputBox))
	{
		ListeningInputBox->CancelUpdateInputKey(ERevertRebindReason::None);
		ListeningInputBox = nullptr;
	}
}

void UUINavPCComponent::SetActiveWidget(UUINavWidget * NewActiveWidget)
{
	if (NewActiveWidget == ActiveWidget) return;

	UUINavWidget* CommonParent = ActiveWidget != nullptr ? ActiveWidget->GetMostOuterUINavWidget() : NewActiveWidget->GetMostOuterUINavWidget();
	
	if (ActiveWidget != nullptr)
	{
		if (NewActiveWidget == nullptr)
		{
			IUINavPCReceiver::Execute_OnRootWidgetRemoved(GetOwner());
		}
	}
	
	if (NewActiveWidget != nullptr)
	{
		if (ActiveWidget == nullptr)
		{
			IUINavPCReceiver::Execute_OnRootWidgetAdded(GetOwner());
		}
	}

	IUINavPCReceiver::Execute_OnActiveWidgetChanged(GetOwner(), ActiveWidget, NewActiveWidget);
	ActiveWidget = NewActiveWidget;
	ActiveSubWidget = nullptr;
	RefreshNavigationKeys();
}

void UUINavPCComponent::NotifyNavigatedTo(UUINavWidget* NavigatedWidget)
{
	if (!IsValid(NavigatedWidget) || NavigatedWidget == ActiveSubWidget)
	{
		return;
	}

	if (NavigatedWidget == ActiveWidget && !IsValid(ActiveSubWidget) && IsValid(ActiveWidget->GetCurrentComponent()))
	{
		return;
	}

	UUINavWidget* OldActiveWidget = ActiveSubWidget != nullptr ? ActiveSubWidget : ActiveWidget;
	UUINavWidget* OldActiveSubWidget = ActiveSubWidget;

	if (!IsValid(ActiveWidget))
	{
		SetActiveWidget(NavigatedWidget);
	}

	UUINavWidget* CommonParent = OldActiveWidget != nullptr ? OldActiveWidget->GetMostOuterUINavWidget() : NavigatedWidget->GetMostOuterUINavWidget();
	if (CommonParent == nullptr)
	{
		return;
	}

	ActiveSubWidget = CommonParent != NavigatedWidget ? NavigatedWidget : nullptr;

	uint8 Depth = 0;
	const TArray<int>& OldPath = OldActiveWidget != nullptr ? OldActiveWidget->GetUINavWidgetPath() : TArray<int>();
	const TArray<int>& NewPath = NavigatedWidget != nullptr ? NavigatedWidget->GetUINavWidgetPath() : TArray<int>();

	if (OldPath.Num() == Depth && OldActiveWidget != nullptr) OldActiveWidget->LoseNavigation(NavigatedWidget);
	if (NewPath.Num() == Depth && NavigatedWidget != nullptr) NavigatedWidget->GainNavigation(OldActiveWidget);

	while (true)
	{
		const int OldIndex = OldPath.Num() > Depth ? OldPath[Depth] : -1;
		const int NewIndex = NewPath.Num() > Depth ? NewPath[Depth] : -1;
		Depth++;

		if (OldIndex == -1 && NewIndex == -1) break;

		if (OldIndex != NewIndex)
		{
			if (Depth == FMath::Max(OldPath.Num(), NewPath.Num()))
			{
				if (OldIndex != -1 && OldActiveWidget != nullptr) OldActiveWidget->PropagateLoseNavigation(NavigatedWidget, OldActiveWidget, CommonParent);
				if (NewIndex != -1 && NavigatedWidget != nullptr) NavigatedWidget->PropagateGainNavigation(OldActiveWidget, NavigatedWidget, CommonParent);
				break;
			}
		}
		else
		{
			CommonParent = CommonParent->GetChildUINavWidget(OldIndex);
		}

		if (OldPath.Num() == Depth && OldActiveWidget != nullptr) OldActiveWidget->LoseNavigation(NavigatedWidget);
		if (NewPath.Num() == Depth && NavigatedWidget != nullptr) NavigatedWidget->GainNavigation(OldActiveWidget);
	}

	if (ActiveWidget != NavigatedWidget)
	{
		SetActiveWidget(NavigatedWidget);
	}
}

UUINavWidget* UUINavPCComponent::GoToWidget(TSubclassOf<UUINavWidget> NewWidgetClass, const bool bRemoveParent, const bool bDestroyParent, const int ZOrder)
{
	if (NewWidgetClass == nullptr)
	{
		DISPLAYERROR("GoToWidget: No Widget Class found");
		return nullptr;
	}

	UUINavWidget* NewWidget = CreateWidget<UUINavWidget>(PC, NewWidgetClass);
	return GoToBuiltWidget(NewWidget, bRemoveParent, bDestroyParent, ZOrder);
}

UUINavWidget* UUINavPCComponent::GoToBuiltWidget(UUINavWidget* NewWidget, const bool bRemoveParent, const bool bDestroyParent, const int ZOrder)
{
	if (NewWidget == nullptr) return nullptr;

	if (ActiveWidget == nullptr)
	{
		NewWidget->AddToViewport();
		NewWidget->SetFocus();
		return NewWidget;
	}

	return ActiveWidget->GoToBuiltWidget(NewWidget, bRemoveParent, bDestroyParent, ZOrder);
}

EThumbstickAsMouse UUINavPCComponent::UsingThumbstickAsMouse() const
{
	return IsValid(ActiveWidget) && ActiveWidget->UseThumbstickAsMouse != EThumbstickAsMouse::None ? ActiveWidget->UseThumbstickAsMouse : UseThumbstickAsMouse;
}

void UUINavPCComponent::RefreshNavigationKeys()
{
	FSlateApplication::Get().SetNavigationConfig(MakeShared<FUINavigationConfig>(bAllowSelectInput, bAllowReturnInput, bUseAnalogDirectionalInput && UsingThumbstickAsMouse() != EThumbstickAsMouse::LeftThumbstick));
}

void UUINavPCComponent::SetAllowAllMenuInput(const bool bAllowInput)
{
	bAllowDirectionalInput = bAllowInput;
	bAllowSelectInput = bAllowInput;
	bAllowReturnInput = bAllowInput;
	bAllowSectionInput = bAllowInput;

	RefreshNavigationKeys();
}

void UUINavPCComponent::SetAllowDirectionalInput(const bool bAllowInput)
{
	bAllowDirectionalInput = bAllowInput;
}

void UUINavPCComponent::SetAllowSelectInput(const bool bAllowInput)
{
	bAllowSelectInput = bAllowInput;
	RefreshNavigationKeys();
}

void UUINavPCComponent::SetAllowReturnInput(const bool bAllowInput)
{
	bAllowReturnInput = bAllowInput;
	RefreshNavigationKeys();
}

void UUINavPCComponent::SetAllowSectionInput(const bool bAllowInput)
{
	bAllowSectionInput = bAllowInput;
}

void UUINavPCComponent::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	LastPressedKey = InKeyEvent.GetKey();
	LastPressedKeyUserIndex = InKeyEvent.GetUserIndex();
	VerifyInputTypeChangeByKey(InKeyEvent.GetKey());
}

void UUINavPCComponent::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (IsValid(ListeningInputBox))
	{
		FKey Key = InKeyEvent.GetKey();

		if (ListeningInputBox->IsAxis())
		{
			Key = GetAxisFromKey(Key);
		}
		ProcessRebind(InKeyEvent.GetKey());
	}
}

void UUINavPCComponent::HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent)
{
	if (CurrentInputType != EInputType::Gamepad && FMath::Abs(InAnalogInputEvent.GetAnalogValue()) > 0.1f)
	{
		NotifyInputTypeChange(EInputType::Gamepad);
	}

	const EThumbstickAsMouse ThumbstickAsMouse = UsingThumbstickAsMouse();
	if (ThumbstickAsMouse != EThumbstickAsMouse::None)
	{
		const FKey Key = InAnalogInputEvent.GetKey();
		if ((ThumbstickAsMouse == EThumbstickAsMouse::LeftThumbstick && (Key == EKeys::Gamepad_LeftX || Key == EKeys::Gamepad_LeftY)) ||
			(ThumbstickAsMouse == EThumbstickAsMouse::RightThumbstick && (Key == EKeys::Gamepad_RightX || Key == EKeys::Gamepad_RightY)))
		{
			if (ThumbstickDelta == FVector2D::ZeroVector)
			{
				RefreshNavigationKeys();
			}
			bReceivedAnalogInput = true;

			const bool bIsHorizontal = Key == EKeys::Gamepad_LeftX || Key == EKeys::Gamepad_RightX;
			const float Value = InAnalogInputEvent.GetAnalogValue() / 3.0f;
			if (bIsHorizontal) ThumbstickDelta.X = FMath::Abs(Value) > 0.001f ? Value : 0.0f;
			else ThumbstickDelta.Y = FMath::Abs(Value) > 0.001f ? Value : 0.0f;

			if (ThumbstickDelta.SizeSquared() < 0.01f) return;

			const FVector2D OldPosition = SlateApp.GetCursorPos();
			const FVector2D NewPosition(OldPosition.X + (bIsHorizontal ? Value * ThumbstickCursorSensitivity : 0.0f),
										OldPosition.Y + (!bIsHorizontal ? -Value * ThumbstickCursorSensitivity : 0.0f));
			SlateApp.SetCursorPos(NewPosition);
			// Since the cursor may have been locked and its location clamped, get the actual new position
			if (const TSharedPtr<FSlateUser> SlateUser = SlateApp.GetUser(SlateApp.CursorUserIndex))
			{
				//create a new mouse event
				const bool bIsPrimaryUser = FSlateApplication::CursorUserIndex == SlateUser->GetUserIndex();
				const FPointerEvent MouseEvent(
					SlateApp.CursorPointerIndex,
					NewPosition,
					OldPosition,
					bIsPrimaryUser ? SlateApp.GetPressedMouseButtons() : TSet<FKey>(),
					EKeys::Invalid,
					0,
					bIsPrimaryUser ? SlateApp.GetModifierKeys() : FModifierKeysState()
				);
				//process the event
				SlateApp.ProcessMouseMoveEvent(MouseEvent);
			}
		}
	}
}

void UUINavPCComponent::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetCursorDelta().SizeSquared() > 0.0f && (UsingThumbstickAsMouse() == EThumbstickAsMouse::None || !IsMovingThumbstick()))
	{
		if (CurrentInputType != EInputType::Mouse)
		{
			NotifyInputTypeChange(EInputType::Mouse);
		}

		if (IsValid(ListeningInputBox))
		{
			FKey MouseKey;
			const float MouseMoveThreshold = GetDefault<UUINavSettings>()->MouseMoveRebindThreshold;
			const FVector2D MovementDelta = MouseEvent.GetCursorDelta();

			if (MovementDelta.Y > MouseMoveThreshold) 
			{
				MouseKey = MouseDown;
			}
			else if (MovementDelta.Y < -MouseMoveThreshold)
			{
				MouseKey = MouseUp;
			}
			else if (MovementDelta.X > MouseMoveThreshold)
			{
				MouseKey = MouseRight;
			}
			else if (MovementDelta.X < -MouseMoveThreshold)
			{
				MouseKey = MouseLeft;
			}

			ProcessRebind(MouseKey);
		}
	}
}

void UUINavPCComponent::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (IsValid(ListeningInputBox))
	{
		if (ListeningInputBox->IsAxis())
		{
			CancelRebind();
		}
		ProcessRebind(MouseEvent.GetEffectingButton());
	}

	if (CurrentInputType != EInputType::Mouse)
	{
		NotifyInputTypeChange(EInputType::Mouse);
	}
}

void UUINavPCComponent::HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (CurrentInputType != EInputType::Mouse)
	{
		NotifyInputTypeChange(EInputType::Mouse);
	}
}

void UUINavPCComponent::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGesture)
{
	if (IsValid(ListeningInputBox))
	{
		ProcessRebind(InWheelEvent.GetWheelDelta() > 0.f ? EKeys::MouseScrollUp : EKeys::MouseScrollDown);
	}

	if (CurrentInputType != EInputType::Mouse && InWheelEvent.GetWheelDelta() != 0.0f)
	{
		NotifyInputTypeChange(EInputType::Mouse);
	}
}

void UUINavPCComponent::SimulateMousePress()
{
	FSlateApplication& SlateApp = FSlateApplication::Get();
	const FPointerEvent MouseDownEvent(
		0,
		SlateApp.CursorPointerIndex,
		SlateApp.GetCursorPos(),
		SlateApp.GetLastCursorPos(),
		SlateApp.GetPressedMouseButtons(),
		EKeys::LeftMouseButton,
		0,
		SlateApp.GetPlatformApplication()->GetModifierKeys()
	);
	const TSharedPtr<FGenericWindow> GenWindow;
	SlateApp.ProcessMouseButtonDownEvent(GenWindow, MouseDownEvent);
}

void UUINavPCComponent::SimulateMouseRelease()
{
	FSlateApplication& SlateApp = FSlateApplication::Get();
	const FPointerEvent MouseUpEvent(
		0,
		SlateApp.CursorPointerIndex,
		SlateApp.GetCursorPos(),
		SlateApp.GetLastCursorPos(),
		SlateApp.GetPressedMouseButtons(),
		EKeys::LeftMouseButton,
		0,
		SlateApp.GetPlatformApplication()->GetModifierKeys()
	);
	TSharedPtr<FGenericWindow> GenWindow;
	SlateApp.ProcessMouseButtonUpEvent(MouseUpEvent);
}

void UUINavPCComponent::SimulateMouseClick()
{
	SimulateMousePress();
	SimulateMouseRelease();
}

FKey UUINavPCComponent::GetKeyUsedForNavigation(const EUINavigation Direction) const
{
	if (Direction == EUINavigation::Invalid)
	{
		return FKey();
	}

	const TArray<FKey>* const DirectionKeys = PressedNavigationDirections.Find(Direction);
	if (DirectionKeys == nullptr || DirectionKeys->Num() == 0)
	{
		return FKey();
	}

	return DirectionKeys->Last();
}

FKey UUINavPCComponent::GetMostRecentlyPressedKey(const ENavigationGenesis Genesis) const
{
	if (LastPressedKey.IsValid() && (Genesis == ENavigationGenesis::Controller) == LastPressedKey.IsGamepadKey())
	{
		return LastPressedKey;
	}

	return FKey();
}

void UUINavPCComponent::SetTimer(const EUINavigation TimerDirection)
{
	TimerCounter = 0.f;
	CallbackDirection = TimerDirection;
	CountdownPhase = ECountdownPhase::First;
}

void UUINavPCComponent::ClearNavigationTimer()
{
	if (CallbackDirection == EUINavigation::Invalid) return;

	TimerCounter = 0.f;
	CallbackDirection = EUINavigation::Invalid;
	CountdownPhase = ECountdownPhase::None;
}

FKey UUINavPCComponent::GetEnhancedInputKey(const UInputAction* Action, const EInputAxis Axis, const EAxisType Scale, const EInputRestriction InputRestriction) const
{
	if (UUINavBlueprintFunctionLibrary::IsUINavInputAction(Action))
	{
		const UInputMappingContext* const UINavInputContext = GetDefault<UUINavSettings>()->EnhancedInputContext.LoadSynchronous();
		for (const FEnhancedActionKeyMapping& Mapping : UINavInputContext->GetMappings())
		{
			if (Mapping.Action == Action && UUINavBlueprintFunctionLibrary::RespectsRestriction(Mapping.Key, InputRestriction))
			{
				if (Action->ValueType == EInputActionValueType::Boolean || Scale == EAxisType::None)
				{
					return Mapping.Key;
				}
				else
				{
					return GetKeyFromAxis(Mapping.Key, Scale == EAxisType::Positive, Axis);
				}
			}
		}
	}
	else if (const UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		const TArray<FKey> ActionKeys = Subsystem->QueryKeysMappedToAction(Action);
		for (const FKey& Key : ActionKeys)
		{
			if (UUINavBlueprintFunctionLibrary::RespectsRestriction(Key, InputRestriction))
			{
				if (Action->ValueType == EInputActionValueType::Boolean || Scale == EAxisType::None)
				{
					return Key;
				}
				else
				{
					return GetKeyFromAxis(Key, Scale == EAxisType::Positive, Axis);
				}
			}
		}
	}

	for (const UInputMappingContext* const InputContext : CachedInputContexts)
	{
		for (const FEnhancedActionKeyMapping& Mapping : InputContext->GetMappings())
		{
			if (Mapping.Action == Action && UUINavBlueprintFunctionLibrary::RespectsRestriction(Mapping.Key, InputRestriction))
			{
				if (Action->ValueType == EInputActionValueType::Boolean || Scale == EAxisType::None)
				{
					return Mapping.Key;
				}
				else
				{
					return GetKeyFromAxis(Mapping.Key, Scale == EAxisType::Positive, Axis);
				}
			}
		}
	}
	
	return FKey();
}

UTexture2D * UUINavPCComponent::GetKeyIcon(const FKey Key) const
{
	FInputIconMapping* KeyIcon = nullptr;

	if (Key.IsGamepadKey())
	{
		if (GamepadKeyIconData != nullptr && GamepadKeyIconData->GetRowMap().Contains(Key.GetFName()))
		{
			KeyIcon = reinterpret_cast<FInputIconMapping*>(GamepadKeyIconData->GetRowMap()[Key.GetFName()]);
		}
	}
	else
	{
		if (KeyboardMouseKeyIconData != nullptr && KeyboardMouseKeyIconData->GetRowMap().Contains(Key.GetFName()))
		{
			KeyIcon = reinterpret_cast<FInputIconMapping*>(KeyboardMouseKeyIconData->GetRowMap()[Key.GetFName()]);
		}
	}

	if (KeyIcon == nullptr) return nullptr;

	UTexture2D* NewTexture = KeyIcon->InputIcon.LoadSynchronous();
	return NewTexture;
}

UTexture2D* UUINavPCComponent::GetEnhancedInputIcon(const UInputAction* Action, const EInputAxis Axis, const EAxisType Scale, const EInputRestriction InputRestriction) const
{
	return GetKeyIcon(GetEnhancedInputKey(Action, Axis, Scale, InputRestriction));
}

FText UUINavPCComponent::GetEnhancedInputText(const UInputAction* Action, const EInputAxis Axis, const EAxisType Scale, const EInputRestriction InputRestriction) const
{
	return GetKeyText(GetEnhancedInputKey(Action, Axis, Scale, InputRestriction));
}

FText UUINavPCComponent::GetKeyText(const FKey Key) const
{
	if (!Key.IsValid()) return FText();

	FInputNameMapping* Keyname = nullptr;

	if (Key.IsGamepadKey())
	{
		if (GamepadKeyNameData != nullptr && GamepadKeyNameData->GetRowMap().Contains(Key.GetFName()))
		{
			Keyname = reinterpret_cast<FInputNameMapping*>(GamepadKeyNameData->GetRowMap()[Key.GetFName()]);
		}
	}
	else
	{
		if (KeyboardMouseKeyNameData != nullptr && KeyboardMouseKeyNameData->GetRowMap().Contains(Key.GetFName()))
		{
			Keyname = reinterpret_cast<FInputNameMapping*>(KeyboardMouseKeyNameData->GetRowMap()[Key.GetFName()]);
		}
	}

	if (Keyname == nullptr) return Key.GetDisplayName();

	return Keyname->InputText;
}

void UUINavPCComponent::GetEnhancedInputKeys(const UInputAction* Action, TArray<FKey>& OutKeys)
{
	if (const UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		OutKeys = Subsystem->QueryKeysMappedToAction(Action);
	}
}

EInputType UUINavPCComponent::GetKeyInputType(const FKey& Key)
{
	if (Key.IsGamepadKey())
	{
		return EInputType::Gamepad;
	}
	else if (Key.IsMouseButton())
	{
		return EInputType::Mouse;
	}
	else
	{
		return EInputType::Keyboard;
	}
}

const FKey UUINavPCComponent::GetKeyFromAxis(const FKey& Key, const bool bPositive, const EInputAxis Axis) const
{
	const FAxis2D_Keys* Axis2DKeys = Axis2DToAxis1DMap.Find(Key);
	const FKey CheckedKey = Axis2DKeys == nullptr ? Key : (Axis == EInputAxis::X ? Axis2DKeys->PositiveKey : Axis2DKeys->NegativeKey);

	const FAxis2D_Keys* AxisKeys = AxisToKeyMap.Find(CheckedKey);
	if (AxisKeys == nullptr) return FKey();

	return bPositive ? AxisKeys->PositiveKey : AxisKeys->NegativeKey;
}

const FKey UUINavPCComponent::GetAxisFromScaledKey(const FKey& Key, bool& OutbPositive) const
{
	for (const TPair<FKey, FAxis2D_Keys>& AxisKeys : AxisToKeyMap)
	{
		if (AxisKeys.Value.PositiveKey == Key)
		{
			OutbPositive = true;
			return AxisKeys.Key;
		}
		
		if (AxisKeys.Value.NegativeKey == Key)
		{
			OutbPositive = false;
			return AxisKeys.Key;
		}
	}
	return FKey();
}

const FKey UUINavPCComponent::GetAxisFromKey(const FKey& Key) const
{
	const FKey* AxisKey = KeyToAxisMap.Find(Key);
	return AxisKey == nullptr ? Key : *AxisKey;
}

const FKey UUINavPCComponent::GetAxis1DFromAxis2D(const FKey& Key, const EInputAxis Axis) const
{
	const FAxis2D_Keys* Axis2DKeys = Axis2DToAxis1DMap.Find(Key);
	if (Axis2DKeys == nullptr) return FKey();

	return Axis == EInputAxis::X ? Axis2DKeys->PositiveKey : Axis2DKeys->NegativeKey;
}

const FKey UUINavPCComponent::GetAxis2DFromAxis1D(const FKey& Key) const
{
	for (const TPair<FKey, FAxis2D_Keys>& AxisKeys : Axis2DToAxis1DMap)
	{
		if (AxisKeys.Value.PositiveKey == Key) return AxisKeys.Key;
		if (AxisKeys.Value.NegativeKey == Key) return AxisKeys.Key;
	}
	return FKey();
}

const FKey UUINavPCComponent::GetOppositeAxisKey(const FKey& Key, bool& bOutIsPositive) const
{
	for (const TPair<FKey, FAxis2D_Keys>& AxisKeys : AxisToKeyMap)
	{
		if (AxisKeys.Value.PositiveKey == Key)
		{
			bOutIsPositive = false;
			return AxisKeys.Value.NegativeKey;
		}
		if (AxisKeys.Value.NegativeKey == Key)
		{
			bOutIsPositive = true;
			return AxisKeys.Value.PositiveKey;
		}
	}
	return FKey();
}

const FKey UUINavPCComponent::GetOppositeAxis2DAxis(const FKey& Key) const
{
	for (const TPair<FKey, FAxis2D_Keys>& Axis2DAxes : Axis2DToAxis1DMap)
	{
		if (Axis2DAxes.Value.PositiveKey == Key) return Axis2DAxes.Value.NegativeKey;
		if (Axis2DAxes.Value.NegativeKey == Key) return Axis2DAxes.Value.PositiveKey;
	}
	return FKey();
}

bool UUINavPCComponent::IsAxis2D(const FKey Key) const
{
	return Axis2DToAxis1DMap.Contains(Key);
}

bool UUINavPCComponent::IsAxis(const FKey& Key) const
{
	return IsAxis2D(Key) || AxisToKeyMap.Contains(Key);
}

void UUINavPCComponent::VerifyInputTypeChangeByKey(const FKey Key)
{
	const EInputType NewInputType = GetKeyInputType(Key);

	if (NewInputType != CurrentInputType)
	{
		NotifyInputTypeChange(NewInputType);
	}
}

EInputMode UUINavPCComponent::GetInputMode() const
{
	if (PC != nullptr)
	{
		UGameViewportClient* GameViewportClient = PC->GetWorld()->GetGameViewport();

		const bool bIgnore = GameViewportClient->IgnoreInput();
		const EMouseCaptureMode Capt = GameViewportClient->GetMouseCaptureMode();

		if (bIgnore == false && Capt == EMouseCaptureMode::CaptureDuringMouseDown)
		{
			return EInputMode::GameUI;
		}
		else if (bIgnore == true && Capt == EMouseCaptureMode::NoCapture)
		{
			return EInputMode::UI;
		}
		else
		{
			return EInputMode::Game;
		}
	}
	return EInputMode::None;
}

void UUINavPCComponent::NotifyMouseInputType()
{
	if (EInputType::Mouse != CurrentInputType)
	{
		NotifyInputTypeChange(EInputType::Mouse);
	}
}

void UUINavPCComponent::ListenToInputRebind(UUINavInputBox* InputBox)
{
	ListeningInputBox = InputBox;
}

bool UUINavPCComponent::GetAndConsumeIgnoreSelectRelease()
{
	const bool bIgnore = bIgnoreSelectRelease;
	bIgnoreSelectRelease = false;
	return bIgnore;
}

bool UUINavPCComponent::IsListeningToInputRebind() const
{
	return IsValid(ListeningInputBox);
}

void UUINavPCComponent::NotifyInputTypeChange(const EInputType NewInputType)
{
	IUINavPCReceiver::Execute_OnInputChanged(GetOwner(), CurrentInputType, NewInputType);

	const EInputType OldInputType = CurrentInputType;
	CurrentInputType = NewInputType;
	if (ActiveWidget != nullptr)
	{
		ActiveWidget->AttemptUnforceNavigation(CurrentInputType);
		// TODO: Propagate this!
		ActiveWidget->OnInputChanged(OldInputType, CurrentInputType);
	}
	InputTypeChangedDelegate.Broadcast(CurrentInputType);
}

UEnhancedInputComponent* UUINavPCComponent::GetEnhancedInputComponent() const
{
	return IsValid(PC) ? Cast<UEnhancedInputComponent>(PC->InputComponent) : nullptr;
}

void UUINavPCComponent::NavigateInDirection(const EUINavigation InDirection)
{
	IUINavPCReceiver::Execute_OnNavigated(GetOwner(), InDirection);

	TArray<FKey>* DirectionKeys = PressedNavigationDirections.Find(InDirection);
	if (DirectionKeys == nullptr)
	{
		return;
	}

	AllowDirection = InDirection;

	const FKey NavigationKey = DirectionKeys->Last();
	if (!NavigationKey.IsValid())
	{
		return;
	}

	FKeyEvent KeyEvent;
	FSlateApplication& SlateApplication = FSlateApplication::Get();
	if (!NavigationKey.IsGamepadKey())
	{
		const uint32* KeyCodePtr;
		const uint32* CharacterCodePtr;
		FInputKeyManager::Get().GetCodesFromKey(NavigationKey, KeyCodePtr, CharacterCodePtr);
		const int32 KeyCode = KeyCodePtr != nullptr ? *KeyCodePtr : -1;

		FKey const Key = FInputKeyManager::Get().GetKeyFromCodes(KeyCode, *CharacterCodePtr);
		KeyEvent = FKeyEvent(Key, SlateApplication.GetPlatformApplication()->GetModifierKeys(), SlateApplication.GetUserIndexForKeyboard(), true, *CharacterCodePtr, KeyCode);
	}
	else
	{
		KeyEvent = FKeyEvent(NavigationKey, SlateApplication.GetPlatformApplication()->GetModifierKeys(), LastPressedKeyUserIndex, true, 0, 0);
	}

	SlateApplication.ProcessKeyDownEvent(KeyEvent);
}

void UUINavPCComponent::MenuNext()
{
	IUINavPCReceiver::Execute_OnNext(GetOwner());

	/*if (ActiveWidget == nullptr || !bAllowSectionInput) return;

	ClearNavigationTimer();*/
}

void UUINavPCComponent::MenuPrevious()
{
	IUINavPCReceiver::Execute_OnPrevious(GetOwner());

	/*if (ActiveWidget == nullptr || !bAllowSectionInput) return;

	ClearNavigationTimer();*/
}

void UUINavPCComponent::NotifyNavigationKeyPressed(const FKey& Key, const EUINavigation Direction)
{
	TArray<FKey>* DirectionKeys = PressedNavigationDirections.Find(Direction);
	if (DirectionKeys != nullptr)
	{
		if (DirectionKeys->Contains(Key))
		{
			bIgnoreNavigationKey = true;
			return;
		}
		else
		{
			DirectionKeys->AddUnique(Key);
		}
	}
	else
	{
		PressedNavigationDirections.Add(Direction, { Key });
	}

	bIgnoreNavigationKey = false;

	ClearNavigationTimer();
}

void UUINavPCComponent::NotifyNavigationKeyReleased(const FKey& Key, const EUINavigation Direction)
{
	TArray<FKey>* DirectionKeys = PressedNavigationDirections.Find(Direction);
	if (DirectionKeys == nullptr)
	{
		return;
	}

	DirectionKeys->Remove(Key);

	if (DirectionKeys->Num() == 0)
	{
		PressedNavigationDirections.Remove(Direction);
	}

	ClearNavigationTimer();
}

bool UUINavPCComponent::TryNavigateInDirection(const EUINavigation Direction, const ENavigationGenesis Genesis)
{
	FKey PressedKey = GetKeyUsedForNavigation(Direction);
	if (!PressedKey.IsValid())
	{
		PressedKey = GetMostRecentlyPressedKey(Genesis);
		if (!PressedKey.IsValid())
		{
			return false;
		}

		NotifyNavigationKeyPressed(PressedKey, Direction);
	}

	if (Direction == EUINavigation::Invalid)
	{
		return true;
	}

	if (IsValid(ListeningInputBox))
	{
		return false;
	}

	if (AllowDirection == Direction)
	{
		AllowDirection = EUINavigation::Invalid;
		return true;
	}

	TArray<FKey>* DirectionKeys = PressedNavigationDirections.Find(Direction);
	if ((DirectionKeys == nullptr || DirectionKeys->Contains(PressedKey)) && bIgnoreNavigationKey)
	{
		return false;
	}

	bIgnoreNavigationKey = true;

	IUINavPCReceiver::Execute_OnNavigated(GetOwner(), Direction);

	SetTimer(Direction);

	return true;
}
