// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved


#include "UINavPCComponent.h"
#include "UINavWidget.h"
#include "UINavSettings.h"
#include "UINavPCReceiver.h"
#include "Data/InputIconMapping.h"
#include "Data/InputNameMapping.h"

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

	if (PC != nullptr && PC->IsLocalPlayerController())
	{
		VerifyDefaultInputs();
		FetchUINavActionKeys();
	}
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

	float PosX, PosY;
	PC->GetMousePosition(PosX, PosY);
	if (CurrentInputType != EInputType::Mouse)
	{
		if ((PosX != 0.f && PosX != PreviousX) || (PosY != 0.f && PosY != PreviousY))
		{
			NotifyMouseInputType();
		}
	}
	PreviousX = PosX;
	PreviousY = PosY;
}

void UUINavPCComponent::BindMenuInputs()
{
	UInputComponent* InputComponent = PC->InputComponent;
	if (InputComponent == nullptr) return;

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

}

void UUINavPCComponent::UnbindMenuInputs()
{
	UInputComponent* InputComponent = PC->InputComponent;
	if (InputComponent == nullptr) return;

	int NumActionBindings = InputComponent->GetNumActionBindings();
	for (int i = NumActionBindings - 1; i >= 0; i--)
	{
		if (InputComponent->GetActionBinding(i).ActionDelegate.IsBoundToObject(this))
		{
			InputComponent->RemoveActionBinding(i);
		}
	}
}

void UUINavPCComponent::VerifyDefaultInputs()
{
	UUINavSettings *MySettings = GetMutableDefault<UUINavSettings>();
	if (MySettings->ActionMappings.Num() == 0 && MySettings->AxisMappings.Num() == 0)
	{
		UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
		MySettings->ActionMappings = Settings->GetActionMappings();
		MySettings->AxisMappings = Settings->GetAxisMappings();
		MySettings->SaveConfig();
	}
}

void UUINavPCComponent::BindMouseWorkaround()
{
	UInputComponent* InputComponent = PC->InputComponent;
	if (InputComponent == nullptr) return;


	FInputKeyBinding& Action = InputComponent->BindKey(EKeys::AnyKey, IE_Pressed, this, &UUINavPCComponent::MouseInputWorkaround);
	Action.bExecuteWhenPaused = true;
	Action.bConsumeInput = false;
}

void UUINavPCComponent::UnbindMouseWorkaround()
{
	UInputComponent* InputComponent = PC->InputComponent;
	if (InputComponent == nullptr) return;


	for (int i = 0; i < InputComponent->KeyBindings.Num(); i++)
	{
		if (InputComponent->KeyBindings[i].KeyDelegate.IsBoundToObject(this))
		{
			InputComponent->KeyBindings.RemoveAt(i);
			break;
		}
	}
}

void UUINavPCComponent::SetActiveWidget(UUINavWidget * NewActiveWidget)
{
	if (ActiveWidget != nullptr && NewActiveWidget == nullptr)
	{
		UnbindMenuInputs();
		PressedActions.Empty();
	}
	else if (ActiveWidget == nullptr && NewActiveWidget != nullptr)
	{
		BindMenuInputs();
	}
	ActiveWidget = NewActiveWidget;
}

void UUINavPCComponent::TimerCallback()
{
	switch (CallbackDirection)
	{
		case ENavigationDirection::Up:
			MenuUp();
			break;
		case ENavigationDirection::Down:
			MenuDown();
			break;
		case ENavigationDirection::Left:
			MenuLeft();
			break;
		case ENavigationDirection::Right:
			MenuRight();
			break;
	}
}

void UUINavPCComponent::SetTimer(ENavigationDirection TimerDirection)
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
}

FKey UUINavPCComponent::GetInputKey(FName ActionName, EInputRestriction InputRestriction)
{
	FKey FinalKey = FKey("None");
	TArray<FKey> KeyArray = TArray<FKey>();

	const UInputSettings* Settings = GetDefault<UInputSettings>();

	TArray<FInputActionKeyMapping> ActionMappings;
	Settings->GetActionMappingByName(ActionName, ActionMappings);

	if (ActionMappings.Num() > 0)
	{
		switch (InputRestriction)
		{
			case EInputRestriction::None:
				FinalKey = ActionMappings[0].Key;
				break;
			case EInputRestriction::Keyboard:
				for (FInputActionKeyMapping mapping : ActionMappings)
				{
					if (mapping.Key.IsGamepadKey() || mapping.Key.IsMouseButton()) continue;
					FinalKey = mapping.Key;
				}
				break;
			case EInputRestriction::Mouse:
				for (FInputActionKeyMapping mapping : ActionMappings)
				{
					if (!mapping.Key.IsMouseButton()) continue;
					FinalKey = mapping.Key;
				}
				break;
			case EInputRestriction::Keyboard_Mouse:
				for (FInputActionKeyMapping mapping : ActionMappings)
				{
					if (mapping.Key.IsGamepadKey()) continue;
					FinalKey = mapping.Key;
				}
				break;
			case EInputRestriction::Gamepad:
				for (FInputActionKeyMapping mapping : ActionMappings)
				{
					if (!mapping.Key.IsGamepadKey()) continue;
					FinalKey = mapping.Key;
				}
				break;
		}
	}
	return FinalKey;
}

UTexture2D * UUINavPCComponent::GetInputIcon(FName ActionName, EInputRestriction InputRestriction)
{
	FKey Key = GetInputKey(ActionName, InputRestriction);
	FName KeyName = Key.GetFName();
	if (KeyName.IsEqual(FName("None"))) return nullptr;

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

FString UUINavPCComponent::GetInputName(FName ActionName, EInputRestriction InputRestriction)
{
	FKey Key = GetInputKey(ActionName, InputRestriction);
	FName KeyName = Key.GetFName();
	if (KeyName.IsEqual(FName("None"))) return TEXT("");

	FInputNameMapping* KeyMapping = nullptr;

	if (Key.IsGamepadKey())
	{
		if (GamepadKeyNameData != nullptr && GamepadKeyNameData->GetRowMap().Contains(Key.GetFName()))
		{
			KeyMapping = (FInputNameMapping*)GamepadKeyNameData->GetRowMap()[Key.GetFName()];
		}
	}
	else
	{
		if (KeyboardMouseKeyNameData != nullptr && KeyboardMouseKeyNameData->GetRowMap().Contains(Key.GetFName()))
		{
			KeyMapping = (FInputNameMapping*)KeyboardMouseKeyNameData->GetRowMap()[Key.GetFName()];
		}
	}

	if (KeyMapping == nullptr) return Key.GetDisplayName().ToString();

	return KeyMapping->InputName;
}

TArray<FKey> UUINavPCComponent::GetInputKeysFromName(FName InputName)
{
	TArray<FKey> KeyArray = TArray<FKey>();

	const UInputSettings* Settings = GetDefault<UInputSettings>();

	TArray<FInputActionKeyMapping> ActionMappings;
	Settings->GetActionMappingByName(InputName, ActionMappings);

	if (ActionMappings.Num() > 0)
	{
		for (FInputActionKeyMapping mapping : ActionMappings)
		{
			KeyArray.Add(mapping.Key);
		}
	}
	else
	{
		TArray<FInputAxisKeyMapping> AxisMappings;
		Settings->GetAxisMappingByName(InputName, AxisMappings);
		if (AxisMappings.Num() > 0)
		{
			for (FInputAxisKeyMapping mapping : AxisMappings)
			{
				KeyArray.Add(mapping.Key);
			}
		}
	}
	return KeyArray;
}

EInputType UUINavPCComponent::GetKeyInputType(FKey Key)
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

	return CurrentInputType;
}

EInputType UUINavPCComponent::GetActionInputType(FString Action)
{
	for (FKey key : KeyMap[Action])
	{
		if (PC->WasInputKeyJustPressed(key)) return GetKeyInputType(key);
	}
	return CurrentInputType;
}

void UUINavPCComponent::VerifyInputTypeChangeByKey(FKey Key)
{
	EInputType NewInputType = GetKeyInputType(Key);

	if (NewInputType != CurrentInputType)
	{
		NotifyInputTypeChange(NewInputType);
	}
}

void UUINavPCComponent::VerifyInputTypeChangeByAction(FString Action)
{
	EInputType NewInputType = GetActionInputType(Action);

	if (NewInputType != CurrentInputType)
	{
		NotifyInputTypeChange(NewInputType);
	}
}

void UUINavPCComponent::NotifyKeyPressed(FKey PressedKey)
{
	ExecuteActionByKey(PressedKey, true);
}

void UUINavPCComponent::NotifyKeyReleased(FKey ReleasedKey)
{
	ExecuteActionByKey(ReleasedKey, false);
}

bool UUINavPCComponent::IsReturnKey(FKey PressedKey)
{
	for (FKey key : KeyMap[TEXT("MenuReturn")]) if (key == PressedKey) return true;
	return false;
}

void UUINavPCComponent::ExecuteActionByKey(FKey PressedKey, bool bPressed)
{
	FString ActionName = FindActionByKey(PressedKey);
	if (ActionName.Equals(TEXT(""))) return;

	ExecuteActionByName(ActionName, bPressed);
}

FString UUINavPCComponent::FindActionByKey(FKey ActionKey)
{
	TArray<FString> Actions;
	KeyMap.GenerateKeyArray(Actions);
	for (FString action : Actions)
	{
		for (FKey key : KeyMap[action])
		{
			if (key == ActionKey) return action;
		}
	}
	return TEXT("");
}

FReply UUINavPCComponent::OnActionPressed(FString ActionName, FKey Key)
{
	if (!PressedActions.Contains(ActionName))
	{
		PressedActions.AddUnique(ActionName);
		ExecuteActionByName(ActionName, true);
		VerifyInputTypeChangeByKey(Key);
		return FReply::Handled();
	}
	else return FReply::Unhandled();
}

FReply UUINavPCComponent::OnActionReleased(FString ActionName, FKey Key)
{
	if (PressedActions.Contains(ActionName))
	{
		PressedActions.Remove(ActionName);
		ExecuteActionByName(ActionName, false);
		return FReply::Handled();
	}
	else return FReply::Unhandled();
}

void UUINavPCComponent::ExecuteActionByName(FString Action, bool bPressed)
{
	if (Action.Equals("MenuUp"))
	{
		if (bPressed) StartMenuUp();
		else MenuUpRelease();
	}
	else if (Action.Equals("MenuDown"))
	{
		if (bPressed) StartMenuDown();
		else MenuDownRelease();
	}
	else if (Action.Equals("MenuLeft"))
	{
		if (bPressed) StartMenuLeft();
		else MenuLeftRelease();
	}
	else if (Action.Equals("MenuRight"))
	{
		if (bPressed) StartMenuRight();
		else MenuRightRelease();
	}
	else if (Action.Equals("MenuSelect") && bPressed)
	{
		MenuSelect();
	}
	else if (Action.Equals("MenuReturn") && bPressed)
	{
		MenuReturn();
	}
}

void UUINavPCComponent::NotifyMouseInputType()
{
	if (EInputType::Mouse != CurrentInputType)
	{
		NotifyInputTypeChange(EInputType::Mouse);
	}
}

void UUINavPCComponent::NotifyInputTypeChange(EInputType NewInputType)
{
	IUINavPCReceiver::Execute_OnInputChanged(GetOwner(), CurrentInputType, NewInputType);

	EInputType OldInputType = CurrentInputType;
	CurrentInputType = NewInputType;
	if (ActiveWidget != nullptr) ActiveWidget->OnInputChanged(OldInputType, CurrentInputType);

}

void UUINavPCComponent::MenuUp()
{
	IUINavPCReceiver::Execute_OnNavigated(GetOwner(), ENavigationDirection::Up);

	if (ActiveWidget == nullptr) return;

	ActiveWidget->NavigateInDirection(ENavigationDirection::Up);
}

void UUINavPCComponent::MenuDown()
{
	IUINavPCReceiver::Execute_OnNavigated(GetOwner(), ENavigationDirection::Down);

	if (ActiveWidget == nullptr) return;

	ActiveWidget->NavigateInDirection(ENavigationDirection::Down);
}

void UUINavPCComponent::MenuLeft()
{
	IUINavPCReceiver::Execute_OnNavigated(GetOwner(), ENavigationDirection::Left);

	if (ActiveWidget == nullptr) return;

	ActiveWidget->NavigateInDirection(ENavigationDirection::Left);
}

void UUINavPCComponent::MenuRight()
{
	IUINavPCReceiver::Execute_OnNavigated(GetOwner(), ENavigationDirection::Right);

	if (ActiveWidget == nullptr) return;

	ActiveWidget->NavigateInDirection(ENavigationDirection::Right);
}

void UUINavPCComponent::MenuSelect()
{
	IUINavPCReceiver::Execute_OnSelect(GetOwner());
	VerifyInputTypeChangeByAction(TEXT("MenuSelect"));

	if (ActiveWidget == nullptr) return;

	ClearTimer();
	ActiveWidget->MenuSelect();
}

void UUINavPCComponent::MenuReturn()
{
	IUINavPCReceiver::Execute_OnReturn(GetOwner());
	VerifyInputTypeChangeByAction(TEXT("MenuReturn"));

	if (ActiveWidget == nullptr) return;

	ClearTimer();
	ActiveWidget->MenuReturn();
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

void UUINavPCComponent::MouseInputWorkaround()
{
	GEngine->AddOnScreenDebugMessage(-2, 2.f, FColor::Red, TEXT("Workaround!!"));
	if (ActiveWidget != nullptr && ActiveWidget->bWaitForInput)
	{
		if (PC->WasInputKeyJustPressed(EKeys::LeftMouseButton)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::LeftMouseButton));
		else if (PC->WasInputKeyJustPressed(EKeys::RightMouseButton)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::RightMouseButton));
		else if (PC->WasInputKeyJustPressed(EKeys::MiddleMouseButton)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::MiddleMouseButton));
		else if (PC->WasInputKeyJustPressed(EKeys::MouseScrollUp)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::MouseScrollUp));
		else if (PC->WasInputKeyJustPressed(EKeys::MouseScrollDown)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::MouseScrollDown));
		else if (PC->WasInputKeyJustPressed(EKeys::ThumbMouseButton)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::ThumbMouseButton));
		else if (PC->WasInputKeyJustPressed(EKeys::ThumbMouseButton2)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::ThumbMouseButton2));
		else if (PC->WasInputKeyJustPressed(EKeys::MouseScrollDown)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::MouseScrollDown));
		else if (PC->WasInputKeyJustPressed(EKeys::MouseScrollUp)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::MouseScrollUp));
	}
}

void UUINavPCComponent::StartMenuUp()
{
	if (Direction == ENavigationDirection::Up) return;

	MenuUp();
	VerifyInputTypeChangeByAction(TEXT("MenuUp"));
	Direction = ENavigationDirection::Up;

	if (!bChainNavigation) return;

	SetTimer(ENavigationDirection::Up);
}

void UUINavPCComponent::StartMenuDown()
{
	if (Direction == ENavigationDirection::Down) return;

	MenuDown();
	VerifyInputTypeChangeByAction(TEXT("MenuDown"));
	Direction = ENavigationDirection::Down;

	if (!bChainNavigation) return;

	SetTimer(ENavigationDirection::Down);
}

void UUINavPCComponent::StartMenuLeft()
{
	if (Direction == ENavigationDirection::Left) return;

	MenuLeft();
	VerifyInputTypeChangeByAction(TEXT("MenuLeft"));
	Direction = ENavigationDirection::Left;

	if (!bChainNavigation) return;

	SetTimer(ENavigationDirection::Left);
}

void UUINavPCComponent::StartMenuRight()
{
	if (Direction == ENavigationDirection::Right) return;

	MenuRight();
	VerifyInputTypeChangeByAction(TEXT("MenuRight"));
	Direction = ENavigationDirection::Right;

	if (!bChainNavigation) return;

	SetTimer(ENavigationDirection::Right);
}
