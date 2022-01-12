// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavPCComponent.h"
#include "UINavWidget.h"
#include "UINavSettings.h"
#include "UINavPCReceiver.h"
#include "UINavInputContainer.h"
#include "UINavMacros.h"
#include "GameFramework/InputSettings.h"
#include "Data/AxisType.h"
#include "Data/InputIconMapping.h"
#include "Data/InputNameMapping.h"
#include "UINavBlueprintFunctionLibrary.h"
#include "UINavInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"

UUINavPCComponent::UUINavPCComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = true;

	bAutoActivate = true;
	bCanEverAffectNavigation = false;
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

		VerifyDefaultInputs();
		FetchUINavActionKeys();
		BindMenuEnhancedInputs();
		if (!FCoreDelegates::OnControllerConnectionChange.IsBoundToObject(this))
		{
			FCoreDelegates::OnControllerConnectionChange.AddUObject(this, &UUINavPCComponent::OnControllerConnectionChanged);
		}
	}
}

void UUINavPCComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PC != nullptr && PC->IsLocalPlayerController())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(SharedInputProcessor);
	}
	
	FCoreDelegates::OnControllerConnectionChange.RemoveAll(this);

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
				TimerCallback();
				TimerCounter -= InputHeldWaitTime;
				CountdownPhase = ECountdownPhase::Looping;
			}
			break;
		case ECountdownPhase::Looping:
			TimerCounter += DeltaTime;
			if (TimerCounter >= NavigationChainFrequency)
			{
				TimerCallback();
				TimerCounter -= NavigationChainFrequency;
			}
			break;
	}
	

}

void UUINavPCComponent::BindMenuInputs()
{
	UInputComponent* InputComponent = PC->InputComponent;
	if (InputComponent == nullptr) return;

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (IsValid(EnhancedInputComponent))
	{
		// Get the Enhanced Input Local Player Subsystem from the Local Player related to our Player Controller.
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (bAutoAddUINavInputContext)
			{
				Subsystem->AddMappingContext(GetDefault<UUINavSettings>()->EnhancedInputContext.LoadSynchronous(), UINavInputContextPriority);
			}
		}
	}
	else
	{
		FInputActionBinding& Action1_1 = InputComponent->BindAction("MenuUp", IE_Pressed, this, &UUINavPCComponent::StartMenuUp);
		Action1_1.bExecuteWhenPaused = true;
		Action1_1.bConsumeInput = false;
		FInputActionBinding& Action2_1 = InputComponent->BindAction("MenuDown", IE_Pressed, this, &UUINavPCComponent::StartMenuDown);
		Action2_1.bExecuteWhenPaused = true;
		Action2_1.bConsumeInput = false;
		FInputActionBinding& Action3_1 = InputComponent->BindAction("MenuLeft", IE_Pressed, this, &UUINavPCComponent::StartMenuLeft);
		Action3_1.bExecuteWhenPaused = true;
		Action3_1.bConsumeInput = false;
		FInputActionBinding& Action4_1 = InputComponent->BindAction("MenuRight", IE_Pressed, this, &UUINavPCComponent::StartMenuRight);
		Action4_1.bExecuteWhenPaused = true;
		Action4_1.bConsumeInput = false;
		FInputActionBinding& Action5_1 = InputComponent->BindAction("MenuSelect", IE_Pressed, this, &UUINavPCComponent::MenuSelect);
		Action5_1.bExecuteWhenPaused = true;
		Action5_1.bConsumeInput = false;
		FInputActionBinding& Action6_1 = InputComponent->BindAction("MenuReturn", IE_Pressed, this, &UUINavPCComponent::MenuReturn);
		Action6_1.bExecuteWhenPaused = true;
		Action6_1.bConsumeInput = false;
		FInputActionBinding& Action7_1 = InputComponent->BindAction("MenuNext", IE_Pressed, this, &UUINavPCComponent::MenuNext);
		Action7_1.bExecuteWhenPaused = true;
		Action7_1.bConsumeInput = false;
		FInputActionBinding& Action8_1 = InputComponent->BindAction("MenuPrevious", IE_Pressed, this, &UUINavPCComponent::MenuPrevious);
		Action8_1.bExecuteWhenPaused = true;
		Action8_1.bConsumeInput = false;

		FInputActionBinding& Action1_2 = InputComponent->BindAction("MenuUp", IE_Released, this, &UUINavPCComponent::MenuUpRelease);
		Action1_2.bExecuteWhenPaused = true;
		Action1_2.bConsumeInput = false;
		FInputActionBinding& Action2_2 = InputComponent->BindAction("MenuDown", IE_Released, this, &UUINavPCComponent::MenuDownRelease);
		Action2_2.bExecuteWhenPaused = true;
		Action2_2.bConsumeInput = false;
		FInputActionBinding& Action3_2 = InputComponent->BindAction("MenuLeft", IE_Released, this, &UUINavPCComponent::MenuLeftRelease);
		Action3_2.bExecuteWhenPaused = true;
		Action3_2.bConsumeInput = false;
		FInputActionBinding& Action4_2 = InputComponent->BindAction("MenuRight", IE_Released, this, &UUINavPCComponent::MenuRightRelease);
		Action4_2.bExecuteWhenPaused = true;
		Action4_2.bConsumeInput = false;
		FInputActionBinding& Action5_2 = InputComponent->BindAction("MenuSelect", IE_Released, this, &UUINavPCComponent::MenuSelectRelease);
		Action5_2.bExecuteWhenPaused = true;
		Action5_2.bConsumeInput = false;
		FInputActionBinding& Action6_2 = InputComponent->BindAction("MenuReturn", IE_Released, this, &UUINavPCComponent::MenuReturnRelease);
		Action6_2.bExecuteWhenPaused = true;
		Action6_2.bConsumeInput = false;

		if (CustomInputs.Num() > 0)
		{
			bAllowCustomInputs.Empty();
			bAllowCustomInputs.Init(true, CustomInputs.Num());

			int i = 0;
			for (const FName CustomInput : CustomInputs)
			{
				FInputActionHandlerSignature PressedActionHandler;
				PressedActionHandler.BindUFunction(this, TEXT("OnCustomInput"), i, true);
				FInputActionBinding PressedActionBinding(CustomInput, IE_Pressed);
				PressedActionBinding.ActionDelegate = PressedActionHandler;
				FInputActionBinding& CustomPressedInput = InputComponent->AddActionBinding(PressedActionBinding);
				CustomPressedInput.bExecuteWhenPaused = true;
				CustomPressedInput.bConsumeInput = false;

				FInputActionHandlerSignature ReleasedActionHandler;
				ReleasedActionHandler.BindUFunction(this, TEXT("OnCustomInput"), i, false);
				FInputActionBinding ReleasedActionBinding(CustomInput, IE_Released);
				ReleasedActionBinding.ActionDelegate = ReleasedActionHandler;
				FInputActionBinding& CustomReleasedInput = InputComponent->AddActionBinding(ReleasedActionBinding);
				CustomReleasedInput.bExecuteWhenPaused = true;
				CustomReleasedInput.bConsumeInput = false;

				i++;
			}
		}
	}
}

void UUINavPCComponent::UnbindMenuInputs()
{
	UInputComponent* InputComponent = PC->InputComponent;
	if (InputComponent == nullptr) return;

	const UEnhancedInputComponent* const EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (IsValid(EnhancedInputComponent))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (bAutoAddUINavInputContext)
			{
				Subsystem->RemoveMappingContext(GetDefault<UUINavSettings>()->EnhancedInputContext.LoadSynchronous());
			}
		}
	}
	else
	{
		const int NumActionBindings = InputComponent->GetNumActionBindings();
		for (int i = NumActionBindings - 1; i >= 0; i--)
		{
			if (InputComponent->GetActionBinding(i).ActionDelegate.IsBoundToObject(this))
			{
				InputComponent->RemoveActionBinding(i);
			}
		}
	}
}

void UUINavPCComponent::BindMenuEnhancedInputs()
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (IsValid(EnhancedInputComponent))
	{
		const UUINavEnhancedInputActions* const InputActions = GetDefault<UUINavSettings>()->EnhancedInputActions.LoadSynchronous();

		// TODO: Fix input crazyness when alt+tabbing while navigating the widget
		EnhancedInputComponent->BindAction(InputActions->IA_MenuUp, ETriggerEvent::Started, this, &UUINavPCComponent::StartMenuUp);
		EnhancedInputComponent->BindAction(InputActions->IA_MenuDown, ETriggerEvent::Started, this, &UUINavPCComponent::StartMenuDown);
		EnhancedInputComponent->BindAction(InputActions->IA_MenuLeft, ETriggerEvent::Started, this, &UUINavPCComponent::StartMenuLeft);
		EnhancedInputComponent->BindAction(InputActions->IA_MenuRight, ETriggerEvent::Started, this, &UUINavPCComponent::StartMenuRight);
		EnhancedInputComponent->BindAction(InputActions->IA_MenuSelect, ETriggerEvent::Started, this, &UUINavPCComponent::MenuSelect);
		EnhancedInputComponent->BindAction(InputActions->IA_MenuReturn, ETriggerEvent::Started, this, &UUINavPCComponent::MenuReturn);
		EnhancedInputComponent->BindAction(InputActions->IA_MenuNext, ETriggerEvent::Started, this, &UUINavPCComponent::MenuNext);
		EnhancedInputComponent->BindAction(InputActions->IA_MenuPrevious, ETriggerEvent::Started, this, &UUINavPCComponent::MenuPrevious);

		EnhancedInputComponent->BindAction(InputActions->IA_MenuUp, ETriggerEvent::Canceled, this, &UUINavPCComponent::MenuUpRelease);
		EnhancedInputComponent->BindAction(InputActions->IA_MenuDown, ETriggerEvent::Canceled, this, &UUINavPCComponent::MenuDownRelease);
		EnhancedInputComponent->BindAction(InputActions->IA_MenuLeft, ETriggerEvent::Canceled, this, &UUINavPCComponent::MenuLeftRelease);
		EnhancedInputComponent->BindAction(InputActions->IA_MenuRight, ETriggerEvent::Canceled, this, &UUINavPCComponent::MenuRightRelease);
		EnhancedInputComponent->BindAction(InputActions->IA_MenuSelect, ETriggerEvent::Canceled, this, &UUINavPCComponent::MenuSelectRelease);
		EnhancedInputComponent->BindAction(InputActions->IA_MenuReturn, ETriggerEvent::Canceled, this, &UUINavPCComponent::MenuReturnRelease);
		
		if (CustomEnhancedInputs.Num() > 0)
		{
			bAllowCustomInputs.Empty();
			bAllowCustomInputs.Init(true, CustomEnhancedInputs.Num());

			int i = 0;
			for (const UInputAction* CustomInput : CustomEnhancedInputs)
			{
				EnhancedInputComponent->BindAction(CustomInput, ETriggerEvent::Started, this, &UUINavPCComponent::OnCustomInput, i, true);
				EnhancedInputComponent->BindAction(CustomInput, ETriggerEvent::Completed, this, &UUINavPCComponent::OnCustomInput, i, false);
				++i;
			}
		}
	}
}

void UUINavPCComponent::OnCustomInput(const int InputIndex, const bool bPressed)
{
	CallCustomInput(CustomInputs[InputIndex], bPressed);
}

void UUINavPCComponent::CallCustomInput(const FName ActionName, const bool bPressed)
{
	if (ActiveWidget != nullptr && AllowsCustomInputByName(ActionName))
	{
		const int CustomInputIndex = CustomInputs.Find(ActionName);
		if (CustomInputIndex < 0) return;

		uint8* Buffer = (uint8*)FMemory_Alloca(sizeof(bool));
		FMemory::Memcpy(Buffer, &bPressed, sizeof(bool));

		ActiveWidget->CallCustomInput(ActionName, Buffer);
	}
}

void UUINavPCComponent::OnControllerConnectionChanged(bool bConnected, int32 UserId, int32 UserIndex)
{
	IUINavPCReceiver::Execute_OnControllerConnectionChanged(GetOwner(), bConnected, UserId, UserIndex);
}

void UUINavPCComponent::VerifyDefaultInputs()
{
	UUINavSettings *MySettings = GetMutableDefault<UUINavSettings>();
	if (MySettings->ActionMappings.Num() == 0 && MySettings->AxisMappings.Num() == 0)
	{
		const UInputSettings* Settings = GetDefault<UInputSettings>();
		MySettings->ActionMappings = Settings->GetActionMappings();
		MySettings->AxisMappings = Settings->GetAxisMappings();
		MySettings->SaveConfig();
	}
}

void UUINavPCComponent::BindMouseWorkaround()
{
	UInputComponent* InputComponent = PC->InputComponent;
	if (InputComponent == nullptr) return;

	TArray<FKey> MouseKeys =
	{
		EKeys::LeftMouseButton,
		EKeys::RightMouseButton,
		EKeys::MiddleMouseButton,
		EKeys::MouseScrollUp,
		EKeys::MouseScrollDown,
		EKeys::ThumbMouseButton,
		EKeys::ThumbMouseButton2
	};

	for (FKey MouseKey : MouseKeys)
	{
		FInputKeyBinding NewKey;
		NewKey.KeyDelegate.BindDelegate<FMouseKeyDelegate>(this, &UUINavPCComponent::MouseKeyPressed, EKeys::AnyKey);
		NewKey.bConsumeInput = true;
		NewKey.bExecuteWhenPaused = true;
		InputComponent->KeyBindings.Add(NewKey);
	}
}

void UUINavPCComponent::UnbindMouseWorkaround()
{
	UInputComponent* InputComponent = PC->InputComponent;
	if (InputComponent == nullptr) return;

	const int KeyBindingsNum = InputComponent->KeyBindings.Num();
	for (int i = KeyBindingsNum - 1; i >= 0; i--)
	{
		if (InputComponent->KeyBindings[i].KeyDelegate.IsBoundToObject(this))
		{
			InputComponent->KeyBindings.RemoveAt(i);
		}
	}
}

void UUINavPCComponent::SetActiveWidget(UUINavWidget * NewActiveWidget)
{
	if (NewActiveWidget == ActiveWidget) return;
	
	if (ActiveWidget != nullptr)
	{
		if (NewActiveWidget == nullptr)
		{
			UnbindMenuInputs();
			PressedActions.Empty();
		}

		if (ActiveWidget->OuterUINavWidget == nullptr)
		{
			ActiveWidget->LoseNavigation(NewActiveWidget);
		}
	}
	else if (NewActiveWidget != nullptr)
	{
		BindMenuInputs();
		NewActiveWidget->GainNavigation(nullptr);
	}

	IUINavPCReceiver::Execute_OnActiveWidgetChanged(GetOwner(), ActiveWidget, NewActiveWidget);	
	ActiveWidget = NewActiveWidget;
}

void UUINavPCComponent::SetActiveNestedWidget(UUINavWidget* NewActiveWidget)
{
	if (NewActiveWidget == ActiveWidget || ActiveWidget == nullptr) return;

	UUINavWidget* OldActiveWidget = ActiveWidget;

	if (NewActiveWidget != nullptr)
	{
		NewActiveWidget->SetUserFocus(PC);
		if (GetInputMode() == EInputMode::UI)
		{
			NewActiveWidget->SetKeyboardFocus();
		}

		SetActiveWidget(NewActiveWidget);
	}

	UUINavWidget* MostOuter = OldActiveWidget != nullptr ? OldActiveWidget->GetMostOuterUINavWidget() : NewActiveWidget->GetMostOuterUINavWidget();
	if (MostOuter == nullptr) return;
	
	UUINavWidget* CommonParent = MostOuter;
	
	uint8 Depth = 0;
	const TArray<int>& OldPath = OldActiveWidget != nullptr ? OldActiveWidget->GetUINavWidgetPath() : TArray<int>();
	const TArray<int>& NewPath = NewActiveWidget != nullptr ? NewActiveWidget->GetUINavWidgetPath() : TArray<int>();

	if (OldPath.Num() == Depth && OldActiveWidget != nullptr) OldActiveWidget->LoseNavigation(NewActiveWidget);
	if (NewPath.Num() == Depth && NewActiveWidget != nullptr) NewActiveWidget->GainNavigation(OldActiveWidget);
	
	bShouldIgnoreHoverEvents = true;
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
				if (OldIndex != -1 && OldActiveWidget != nullptr) OldActiveWidget->PropagateLoseNavigation(NewActiveWidget, OldActiveWidget, CommonParent);
				if (NewIndex != -1 && NewActiveWidget != nullptr) NewActiveWidget->PropagateGainNavigation(OldActiveWidget, NewActiveWidget, CommonParent);
				break;
			}
		}
		else
		{
		    CommonParent = CommonParent->GetChildUINavWidget(OldIndex);
		}
		
		if (OldPath.Num() == Depth && OldActiveWidget != nullptr) OldActiveWidget->LoseNavigation(NewActiveWidget);
		if (NewPath.Num() == Depth && NewActiveWidget != nullptr) NewActiveWidget->GainNavigation(OldActiveWidget);
	}

	bShouldIgnoreHoverEvents = false;
}

void UUINavPCComponent::SetAllowAllMenuInput(const bool bAllowInput)
{
	bAllowDirectionalInput = bAllowInput;
	bAllowSelectInput = bAllowInput;
	bAllowReturnInput = bAllowInput;
	bAllowSectionInput = bAllowInput;
	for (bool& allowCustomInput : bAllowCustomInputs)
	{
		allowCustomInput = bAllowInput;
	}
}

void UUINavPCComponent::SetAllowDirectionalInput(const bool bAllowInput)
{
	bAllowDirectionalInput = bAllowInput;
}

void UUINavPCComponent::SetAllowSelectInput(const bool bAllowInput)
{
	bAllowSelectInput = bAllowInput;
}

void UUINavPCComponent::SetAllowReturnInput(const bool bAllowInput)
{
	bAllowReturnInput = bAllowInput;
}

void UUINavPCComponent::SetAllowSectionInput(const bool bAllowInput)
{
	bAllowSectionInput = bAllowInput;
}

void UUINavPCComponent::SetAllowCustomInputByName(const FName InputName, const bool bAllowInput)
{
	const int CustomInputIndex = CustomInputs.Find(InputName);
	if (!bAllowCustomInputs.IsValidIndex(CustomInputIndex)) return;
	bAllowCustomInputs[CustomInputIndex] = bAllowInput;
}

void UUINavPCComponent::SetAllowCustomInputByAction(UInputAction* InputAction, const bool bAllowInput)
{
	const int CustomInputIndex = CustomEnhancedInputs.Find(InputAction);
	if (CustomInputIndex < 0) return;
	bAllowCustomInputs[CustomInputIndex] = bAllowInput;
}

void UUINavPCComponent::SetAllowCustomInputByIndex(const int InputIndex, const bool bAllowInput)
{
	if (!CustomInputs.IsValidIndex(InputIndex) || !bAllowCustomInputs.IsValidIndex(InputIndex)) return;
	bAllowCustomInputs[InputIndex] = bAllowInput;
}

void UUINavPCComponent::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	VerifyInputTypeChangeByKey(InKeyEvent.GetKey());

	if (ActiveWidget != nullptr && GetInputMode() == EInputMode::UI)
	{
		if (!ActiveWidget->IsRebindingInput())
		{
			//Allow fullscreen by pressing F11 or Alt+Enter
			if (!GEngine->GameViewport->TryToggleFullscreenOnInputKey(InKeyEvent.GetKey(), IE_Pressed))
			{
				OnKeyPressed(InKeyEvent.GetKey());
			}
		}
	}
}

void UUINavPCComponent::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (ActiveWidget != nullptr && GetInputMode() == EInputMode::UI)
	{
		if (ActiveWidget->IsRebindingInput())
		{
			FKey Key = InKeyEvent.GetKey();

			if (ActiveWidget->ReceiveInputType == EReceiveInputType::Axis)
			{
				Key = ActiveWidget->UINavInputContainer->GetAxisFromKey(Key);
			}

			ActiveWidget->ProcessKeybind(Key);
		}
		else
		{
			OnKeyReleased(InKeyEvent.GetKey());
		}
	}
}

void UUINavPCComponent::HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent)
{
	if (CurrentInputType != EInputType::Gamepad && InAnalogInputEvent.GetAnalogValue() > 0.5f)
	{
		NotifyInputTypeChange(EInputType::Gamepad);
	}

	if ((ActiveWidget != nullptr && ActiveWidget->bUseLeftThumbstickAsMouse) ||
		bUseLeftThumbstickAsMouse)
	{
		const FKey Key = InAnalogInputEvent.GetKey();
		if (Key == EKeys::Gamepad_LeftX || Key == EKeys::Gamepad_LeftY)
		{
			const bool bIsHorizontal = Key == EKeys::Gamepad_LeftX;
			const float Value = InAnalogInputEvent.GetAnalogValue() / 3.0f;
			if (bIsHorizontal) LeftStickDelta.X = Value;
			else LeftStickDelta.Y = Value;

			if (Value == 0.0f) return;

			const FVector2D OldPosition = SlateApp.GetCursorPos();
			const FVector2D NewPosition(OldPosition.X + (bIsHorizontal ? Value * LeftStickCursorSensitivity : 0.0f),
										OldPosition.Y + (!bIsHorizontal ? -Value * LeftStickCursorSensitivity : 0.0f));
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
	const bool bShouldUseLeftThumbstickAsMouse = (ActiveWidget != nullptr && ActiveWidget->bUseLeftThumbstickAsMouse) || bUseLeftThumbstickAsMouse;
	if (CurrentInputType != EInputType::Mouse && MouseEvent.GetCursorDelta().SizeSquared() > 0.0f && (!bShouldUseLeftThumbstickAsMouse || !IsMovingLeftStick()))
	{
		NotifyInputTypeChange(EInputType::Mouse);
	}
}

void UUINavPCComponent::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (CurrentInputType != EInputType::Mouse)
	{
		NotifyInputTypeChange(EInputType::Mouse);
	}
}

void UUINavPCComponent::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGesture)
{
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

void UUINavPCComponent::TimerCallback()
{
	MenuInput(CallbackDirection);
}

void UUINavPCComponent::SetTimer(const ENavigationDirection TimerDirection)
{
	TimerCounter = 0.f;
	CallbackDirection = TimerDirection;
	CountdownPhase = ECountdownPhase::First;
}

void UUINavPCComponent::ClearTimer()
{
	if (CallbackDirection == ENavigationDirection::None) return;

	TimerCounter = 0.f;
	CallbackDirection = ENavigationDirection::None;
	CountdownPhase = ECountdownPhase::None;
}

void UUINavPCComponent::FetchUINavActionKeys()
{
	if (IsValid(GetEnhancedInputComponent()))
	{
		const UUINavSettings* const UINavSettings = GetDefault<UUINavSettings>();
		const UInputMappingContext* InputContext = UINavSettings->EnhancedInputContext.LoadSynchronous();

		for (const FEnhancedActionKeyMapping& Action : InputContext->GetMappings())
		{
			const FString NewName = Action.Action->GetName();
			TArray<FKey>* KeyArray = KeyMap.Find(NewName);
			if (KeyArray == nullptr)
			{
				KeyMap.Add(NewName, { Action.Key });
			}
			else
			{
				KeyArray->Add(Action.Key);
			}
		}
	}
	else
	{
		const UInputSettings* Settings = GetDefault<UInputSettings>();
		const TArray<FInputActionKeyMapping> Actions = Settings->GetActionMappings();

		for (FInputActionKeyMapping Action : Actions)
		{
			FString NewName = Action.ActionName.ToString();
			if (NewName.Left(4).Compare(TEXT("Menu")) != 0)
				continue;

			TArray<FKey>* KeyArray = KeyMap.Find(NewName);
			if (KeyArray == nullptr)
			{
				KeyMap.Add(NewName, { Action.Key });
			}
			else
			{
				KeyArray->Add(Action.Key);
			}
		}
		
		TArray<FString> keys;
		KeyMap.GetKeys(keys);
		if (keys.Num() < 6)
		{
			DISPLAYERROR("Not all Menu Inputs have been setup!");
		}
		else if (keys.Num() < 8)
		{
			DISPLAYWARNING("You can add them from the UINavInput.ini file in the plugin's Content folder to your project's DefaultInput.ini file.");
			DISPLAYWARNING("Keep in mind that the MenuNext and MenuPrevious inputs have been added recently.");
			DISPLAYWARNING("Not all Menu Inputs have been setup!");
		}
	}
}

FKey UUINavPCComponent::GetInputKey(FName InputName, const EInputRestriction InputRestriction) const
{
	const FString InputString = InputName.ToString();
	const FString AxisScale = InputString.Right(1);
	EAxisType AxisType = EAxisType::None;
	if (AxisScale.Equals(TEXT("+"))) AxisType = EAxisType::Positive;
	else if (AxisScale.Equals(TEXT("-"))) AxisType = EAxisType::Negative;
	
	const UInputSettings* Settings = GetDefault<UInputSettings>();

	if (AxisType == EAxisType::None)
	{
		TArray<FInputActionKeyMapping> ActionMappings;
		Settings->GetActionMappingByName(InputName, ActionMappings);

		const int Iterations = ActionMappings.Num();
		for (int i = Iterations - 1; i >= 0; --i)
		{
			if (UUINavBlueprintFunctionLibrary::RespectsRestriction(ActionMappings[i].Key, InputRestriction))
				return ActionMappings[i].Key;
		}
		return FKey();
	}
	else
	{
		InputName = FName(*InputString.Left(InputString.Len() - 1));
		TArray<FInputAxisKeyMapping> AxisMappings;
		Settings->GetAxisMappingByName(InputName, AxisMappings);

		const int Iterations = AxisMappings.Num();
		for (int i = Iterations - 1; i >= 0; --i)
		{
			if (UUINavBlueprintFunctionLibrary::RespectsRestriction(AxisMappings[i].Key, InputRestriction))
			{
				if ((AxisMappings[i].Scale > 0.0f && AxisType == EAxisType::Positive) ||
					(AxisMappings[i].Scale < 0.0f && AxisType == EAxisType::Negative))
				{
					return AxisMappings[i].Key;
				}
				else
				{
					const FKey PotentialAxisKey = GetKeyFromAxis(AxisMappings[i].Key, AxisType == EAxisType::Positive);
					if (PotentialAxisKey.IsValid())
					{
						return PotentialAxisKey;
					}
				}
			}
		}
		return FKey();
	}
}

FKey UUINavPCComponent::GetEnhancedInputKey(const UInputAction* Action, const EInputRestriction InputRestriction) const
{
	if (const UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		const TArray<FKey> ActionKeys = Subsystem->QueryKeysMappedToAction(Action);
		for (const FKey& Key : ActionKeys)
		{
			if (UUINavBlueprintFunctionLibrary::RespectsRestriction(Key, InputRestriction))
			{
				if (Action->ValueType == EInputActionValueType::Boolean)
				{
					return Key;
				}
				else
				{
					return Key;
					
					// TODO: Must fetch Input Context modifiers somehow
					/*if ((AxisMappings[i].Scale > 0.0f && AxisType == EAxisType::Positive) ||
						(AxisMappings[i].Scale < 0.0f && AxisType == EAxisType::Negative))
					{
						return Key;
					}
					else
					{
						const FKey PotentialAxisKey = GetKeyFromAxis(Key, AxisType == EAxisType::Positive);
						if (PotentialAxisKey.IsValid())
						{
							return PotentialAxisKey;
						}
					}*/
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
			KeyIcon = (FInputIconMapping*)GamepadKeyIconData->GetRowMap()[Key.GetFName()];
		}
	}
	else
	{
		if (KeyboardMouseKeyIconData != nullptr && KeyboardMouseKeyIconData->GetRowMap().Contains(Key.GetFName()))
		{
			KeyIcon = (FInputIconMapping*)KeyboardMouseKeyIconData->GetRowMap()[Key.GetFName()];
		}
	}

	if (KeyIcon == nullptr) return nullptr;

	UTexture2D* NewTexture = KeyIcon->InputIcon.LoadSynchronous();
	return NewTexture;
}

UTexture2D * UUINavPCComponent::GetInputIcon(const FName ActionName, const EInputRestriction InputRestriction) const
{
	return GetKeyIcon(GetInputKey(ActionName, InputRestriction));
}

UTexture2D* UUINavPCComponent::GetEnhancedInputIcon(const UInputAction* Action,
	const EInputRestriction InputRestriction) const
{
	return GetKeyIcon(GetEnhancedInputKey(Action, InputRestriction));
}

FText UUINavPCComponent::GetKeyText(const FKey Key) const
{
	if (!Key.IsValid()) return FText();

	FInputNameMapping* Keyname = nullptr;

	if (Key.IsGamepadKey())
	{
		if (GamepadKeyNameData != nullptr && GamepadKeyNameData->GetRowMap().Contains(Key.GetFName()))
		{
			Keyname = (FInputNameMapping*)GamepadKeyNameData->GetRowMap()[Key.GetFName()];
		}
	}
	else
	{
		if (KeyboardMouseKeyNameData != nullptr && KeyboardMouseKeyNameData->GetRowMap().Contains(Key.GetFName()))
		{
			Keyname = (FInputNameMapping*)KeyboardMouseKeyNameData->GetRowMap()[Key.GetFName()];
		}
	}

	if (Keyname == nullptr) return Key.GetDisplayName();

	return Keyname->InputText;
}

void UUINavPCComponent::GetInputRebindData(const FName InputName, FInputRebindData& OutData, bool& bSuccess) const
{
	if (InputRebindDataTable != nullptr && InputRebindDataTable->GetRowMap().Contains(InputName))
	{
		FInputRebindData* InputRebindData = (FInputRebindData*)InputRebindDataTable->GetRowMap()[InputName];
		if (InputRebindData != nullptr)
		{
			OutData = *InputRebindData;
			bSuccess = true;
			return;
		}
	}
	bSuccess = false;
}

FText UUINavPCComponent::GetInputText(const FName InputName) const
{
	FInputRebindData InputRebindData;
	bool bSuccess = false;
	GetInputRebindData(InputName, InputRebindData, bSuccess);

	return bSuccess ? InputRebindData.InputText : FText();
}

TArray<FKey> UUINavPCComponent::GetInputKeysFromName(const FName InputName) const
{
	TArray<FKey> KeyArray = TArray<FKey>();

	const UInputSettings* Settings = GetDefault<UInputSettings>();

	TArray<FInputActionKeyMapping> ActionMappings;
	Settings->GetActionMappingByName(InputName, ActionMappings);

	if (ActionMappings.Num() > 0)
	{
		for (FInputActionKeyMapping Mapping : ActionMappings)
		{
			KeyArray.Add(Mapping.Key);
		}
	}
	else
	{
		TArray<FInputAxisKeyMapping> AxisMappings;
		Settings->GetAxisMappingByName(InputName, AxisMappings);
		if (AxisMappings.Num() > 0)
		{
			for (FInputAxisKeyMapping Mapping : AxisMappings)
			{
				KeyArray.Add(Mapping.Key);
			}
		}
	}
	return KeyArray;
}

void UUINavPCComponent::GetInputKeys(FName InputName, TArray<FKey>& OutKeys)
{
	OutKeys.Empty();
	const FString InputString = InputName.ToString();
	const FString AxisScale = InputString.Right(1);
	EAxisType AxisType = EAxisType::None;
	if (AxisScale.Equals(TEXT("+"))) AxisType = EAxisType::Positive;
	else if (AxisScale.Equals(TEXT("-"))) AxisType = EAxisType::Negative;

	const UInputSettings* Settings = GetDefault<UInputSettings>();

	if (AxisType == EAxisType::None)
	{
		TArray<FInputActionKeyMapping> ActionMappings;
		Settings->GetActionMappingByName(InputName, ActionMappings);

		const int Iterations = ActionMappings.Num();
		for (int i = Iterations - 1; i >= 0; --i)
		{
			OutKeys.Add(ActionMappings[i].Key);
		}
	}
	else
	{
		InputName = FName(*InputString.Left(InputString.Len() - 1));
		TArray<FInputAxisKeyMapping> AxisMappings;
		Settings->GetAxisMappingByName(InputName, AxisMappings);

		const int Iterations = AxisMappings.Num();
		for (int i = Iterations - 1; i >= 0; --i)
		{
			if ((AxisMappings[i].Scale > 0.0f && AxisType == EAxisType::Positive) ||
				(AxisMappings[i].Scale < 0.0f && AxisType == EAxisType::Negative))
			{
				OutKeys.Add(AxisMappings[i].Key);
			}
			else
			{
				const FKey PotentialAxisKey = GetKeyFromAxis(AxisMappings[i].Key, AxisType == EAxisType::Positive);
				if (PotentialAxisKey.IsValid())
				{
					OutKeys.Add(PotentialAxisKey);
				}
			}
		}
	}
}

void UUINavPCComponent::GetEnhancedInputKeys(const UInputAction* Action, TArray<FKey>& OutKeys)
{
	if (const UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		OutKeys = Subsystem->QueryKeysMappedToAction(Action);
	}
}

EInputType UUINavPCComponent::GetKeyInputType(const FKey Key)
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

EInputType UUINavPCComponent::GetMenuActionInputType(const FString Action) const
{
	for (const FKey& Key : KeyMap[Action])
	{
		if (PC->WasInputKeyJustPressed(Key)) return GetKeyInputType(Key);
	}
	return CurrentInputType;
}

FKey UUINavPCComponent::GetKeyFromAxis(const FKey Key, const bool bPositive) const
{
	const FAxis2D_Keys* Axis2DKeys = Axis2DToKeyMap.Find(Key);
	if (Axis2DKeys == nullptr) return FKey();

	return bPositive ? Axis2DKeys->PositiveKey : Axis2DKeys->NegativeKey;
}

bool UUINavPCComponent::Is2DAxis(const FKey Key) const
{
	return Axis2DToKeyMap.Contains(Key);
}

void UUINavPCComponent::VerifyInputTypeChangeByKey(const FKey Key)
{
	const EInputType NewInputType = GetKeyInputType(Key);

	if (NewInputType != CurrentInputType)
	{
		NotifyInputTypeChange(NewInputType);
	}
}

void UUINavPCComponent::NotifyKeyPressed(const FKey PressedKey)
{
	ExecuteActionByKey(PressedKey, true);
}

void UUINavPCComponent::NotifyKeyReleased(const FKey ReleasedKey)
{
	ExecuteActionByKey(ReleasedKey, false);
}

void UUINavPCComponent::ExecuteActionByKey(const FKey ActionKey, const bool bPressed)
{
	TArray<FString> ActionNames = FindActionByKey(ActionKey);
	if (ActionNames.Num() == 0) return;

	for (const FString& ActionName : ActionNames)
	{
		ExecuteActionByName(ActionName, bPressed);
	}
}

TArray<FString> UUINavPCComponent::FindActionByKey(const FKey ActionKey) const
{
	TArray<FString> Actions;
	TArray<FString> TriggeredActions;
	KeyMap.GenerateKeyArray(Actions);
	for (FString Action : Actions)
	{
		for (const FKey& Key : KeyMap[Action])
		{
			if (Key == ActionKey)
			{
				TriggeredActions.Add(Action);
			}
		}
	}
	return TriggeredActions;
}

FReply UUINavPCComponent::OnKeyPressed(const FKey PressedKey)
{
	const TArray<FString> ActionNames = FindActionByKey(PressedKey);
	if (ActionNames.Num() == 0) return FReply::Unhandled();

	FReply Reply = FReply::Unhandled();
	for (const FString& ActionName : ActionNames)
	{
		if (OnActionPressed(ActionName, PressedKey).IsEventHandled())
		{
			Reply = FReply::Handled();
		}
	}

	return Reply;
}

FReply UUINavPCComponent::OnKeyReleased(const FKey PressedKey)
{
	const TArray<FString> ActionNames = FindActionByKey(PressedKey);
	if (ActionNames.Num() == 0) return FReply::Unhandled();

	FReply Reply = FReply::Unhandled();
	for (const FString& ActionName : ActionNames)
	{
		if (OnActionReleased(ActionName, PressedKey).IsEventHandled())
		{
			Reply = FReply::Handled();
		}
	}

	return Reply;
}

FReply UUINavPCComponent::OnActionPressed(const FString ActionName, const FKey Key)
{
	if (!PressedActions.Contains(ActionName))
	{
		PressedActions.AddUnique(ActionName);
		ExecuteActionByName(ActionName, true);
		return FReply::Handled();
	}
	else return FReply::Unhandled();
}

FReply UUINavPCComponent::OnActionReleased(const FString ActionName, const FKey Key)
{
	if (PressedActions.Contains(ActionName))
	{
		PressedActions.Remove(ActionName);
		ExecuteActionByName(ActionName, false);
		return FReply::Handled();
	}
	else return FReply::Unhandled();
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

void UUINavPCComponent::ExecuteActionByName(const FString Action, const bool bPressed)
{
	if (Action.Contains("MenuUp"))
	{
		if (bPressed) StartMenuUp();
		else MenuUpRelease();
	}
	else if (Action.Contains("MenuDown"))
	{
		if (bPressed) StartMenuDown();
		else MenuDownRelease();
	}
	else if (Action.Contains("MenuLeft"))
	{
		if (bPressed) StartMenuLeft();
		else MenuLeftRelease();
	}
	else if (Action.Contains("MenuRight"))
	{
		if (bPressed) StartMenuRight();
		else MenuRightRelease();
	}
	else if (Action.Contains("MenuSelect"))
	{
		if (bPressed) MenuSelect();
		else
		{
			if (ActiveWidget->GetSelectCount() > 1) PressedActions.Add(Action);
			MenuSelectRelease();
		}
	}
	else if (Action.Contains("MenuReturn"))
	{
		if (bPressed) MenuReturn();
		else MenuReturnRelease();
	}
	else if (Action.Contains("MenuNext") && bPressed)
	{
		MenuNext();
	}
	else if (Action.Contains("MenuPrevious") && bPressed)
	{
		MenuPrevious();
	}
	else
	{
		CallCustomInput(FName(*Action), bPressed);
	}
}

void UUINavPCComponent::NotifyMouseInputType()
{
	if (EInputType::Mouse != CurrentInputType)
	{
		NotifyInputTypeChange(EInputType::Mouse);
	}
}

void UUINavPCComponent::NotifyInputTypeChange(const EInputType NewInputType)
{
	IUINavPCReceiver::Execute_OnInputChanged(GetOwner(), CurrentInputType, NewInputType);

	const EInputType OldInputType = CurrentInputType;
	CurrentInputType = NewInputType;
	if (ActiveWidget != nullptr)
	{
		ActiveWidget->AttemptUnforceNavigation(CurrentInputType);
		ActiveWidget->OnInputChanged(OldInputType, CurrentInputType);
	}
	InputTypeChangedDelegate.Broadcast(CurrentInputType);
}

UEnhancedInputComponent* UUINavPCComponent::GetEnhancedInputComponent() const
{
	return IsValid(PC) ? Cast<UEnhancedInputComponent>(PC->InputComponent) : nullptr;
}

void UUINavPCComponent::MenuInput(const ENavigationDirection InDirection)
{
	IUINavPCReceiver::Execute_OnNavigated(GetOwner(), InDirection);

	if (ActiveWidget == nullptr || !bAllowDirectionalInput) return;

	ActiveWidget->NavigateInDirection(InDirection);
}

void UUINavPCComponent::MenuSelect()
{
	if (ActiveWidget == nullptr || !bAllowSelectInput) return;

	ClearTimer();
	ActiveWidget->MenuSelectPress();
}

void UUINavPCComponent::MenuSelectRelease()
{
	IUINavPCReceiver::Execute_OnSelect(GetOwner());

	if (ActiveWidget == nullptr || !bAllowSelectInput) return;

	ClearTimer();
	ActiveWidget->MenuSelectRelease();
}

void UUINavPCComponent::MenuReturn()
{
	if (ActiveWidget == nullptr || !bAllowReturnInput) return;

	ClearTimer();
	ActiveWidget->MenuReturnPress();
}

void UUINavPCComponent::MenuReturnRelease()
{
	IUINavPCReceiver::Execute_OnReturn(GetOwner());

	if (ActiveWidget == nullptr || !bAllowReturnInput) return;

	ClearTimer();
	ActiveWidget->MenuReturnRelease();
}

void UUINavPCComponent::MenuNext()
{
	IUINavPCReceiver::Execute_OnNext(GetOwner());

	if (ActiveWidget == nullptr || !bAllowSectionInput) return;

	ClearTimer();
	ActiveWidget->OnNext();
}

void UUINavPCComponent::MenuPrevious()
{
	IUINavPCReceiver::Execute_OnPrevious(GetOwner());

	if (ActiveWidget == nullptr || !bAllowSectionInput) return;

	ClearTimer();
	ActiveWidget->OnPrevious();
}

void UUINavPCComponent::MenuUpRelease()
{
	if (Direction != ENavigationDirection::Up) return;
	Direction = ENavigationDirection::None;

	ClearTimer();
}

void UUINavPCComponent::MenuDownRelease()
{
	if (Direction != ENavigationDirection::Down) return;
	Direction = ENavigationDirection::None;

	ClearTimer();
}

void UUINavPCComponent::MenuLeftRelease()
{
	if (Direction != ENavigationDirection::Left) return;
	Direction = ENavigationDirection::None;

	ClearTimer();
}

void UUINavPCComponent::MenuRightRelease()
{
	if (Direction != ENavigationDirection::Right) return;
	Direction = ENavigationDirection::None;

	ClearTimer();
}

void UUINavPCComponent::MouseKeyPressed(const FKey MouseKey)
{
	if (ActiveWidget != nullptr && ActiveWidget->IsRebindingInput())
	{
		ActiveWidget->ProcessKeybind(MouseKey);
	}
}

void UUINavPCComponent::StartMenuUp()
{
	if (Direction == ENavigationDirection::Up) return;

	MenuInput(ENavigationDirection::Up);
	Direction = ENavigationDirection::Up;

	if (!bChainNavigation || !bAllowDirectionalInput) return;

	SetTimer(ENavigationDirection::Up);
}

void UUINavPCComponent::StartMenuDown()
{
	if (Direction == ENavigationDirection::Down) return;

	MenuInput(ENavigationDirection::Down);
	Direction = ENavigationDirection::Down;

	if (!bChainNavigation || !bAllowDirectionalInput) return;

	SetTimer(ENavigationDirection::Down);
}

void UUINavPCComponent::StartMenuLeft()
{
	if (Direction == ENavigationDirection::Left) return;

	MenuInput(ENavigationDirection::Left);
	Direction = ENavigationDirection::Left;

	if (!bChainNavigation || !bAllowDirectionalInput) return;

	SetTimer(ENavigationDirection::Left);
}

void UUINavPCComponent::StartMenuRight()
{
	if (Direction == ENavigationDirection::Right) return;

	MenuInput(ENavigationDirection::Right);
	Direction = ENavigationDirection::Right;

	if (!bChainNavigation || !bAllowDirectionalInput) return;

	SetTimer(ENavigationDirection::Right);
}
