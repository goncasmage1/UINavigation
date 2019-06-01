// Fill out your copyright notice in the Description page of Project Settings.


#include "UINavPCComponent.h"
#include "UINavWidget.h"
#include "UINavSettings.h"
#include "Data/InputIconMapping.h"
#include "Data/InputNameMapping.h"

UUINavPCComponent::UUINavPCComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	bAutoActivate = true;
	bCanEverAffectNavigation = false;

}

void UUINavPCComponent::Activate(bool bReset)
{
	PC = Cast<APlayerController>(GetOwner());
	if (PC == nullptr)
	{
		DISPLAYERROR(TEXT("UINavPCComponent Owner isn't a Player Controller!"));
	}
}

void UUINavPCComponent::BeginPlay()
{
	Super::BeginPlay();

	VerifyDefaultInputs();
	FetchUINavActionKeys();
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

void UUINavPCComponent::SetupInput()
{
	UInputComponent* InputComponent = PC->InputComponent;

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

	FInputKeyBinding& Action1_3 = InputComponent->BindKey(EKeys::AnyKey, IE_Pressed, this, &UUINavPCComponent::MouseInputWorkaround);
	Action1_3.bExecuteWhenPaused = true;
	Action1_3.bConsumeInput = false;
}

void UUINavPCComponent::VerifyDefaultInputs()
{
	UUINavSettings *MySettings = GetMutableDefault<UUINavSettings>();
	if (MySettings->ActionMappings.Num() == 0 && MySettings->AxisMappings.Num() == 0)
	{
		UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
		MySettings->ActionMappings = Settings->ActionMappings;
		MySettings->AxisMappings = Settings->AxisMappings;
		MySettings->SaveConfig();
	}
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
	const TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;
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
	if (keys.Num() != 6)
	{
		DISPLAYERROR("Not all Menu Inputs have been setup!");
	}
}

FKey UUINavPCComponent::GetMenuActionKey(FString ActionName, EInputRestriction InputRestriction)
{
	FKey FinalKey = FKey("None");
	if (!KeyMap.Contains(ActionName)) return FinalKey;

	TArray<FKey> keys = KeyMap[ActionName];
	if (keys.Num() == 0) return FinalKey;

	switch (InputRestriction)
	{
		case EInputRestriction::None:
			FinalKey = keys[0];
			break;
		case EInputRestriction::Keyboard:
			for (FKey key : keys)
			{
				if (key.IsGamepadKey() || key.IsMouseButton()) continue;
				FinalKey = key;
			}
			break;
		case EInputRestriction::Mouse:
			for (FKey key : keys)
			{
				if (!key.IsMouseButton()) continue;
				FinalKey = key;
			}
			break;
		case EInputRestriction::Keyboard_Mouse:
			for (FKey key : keys)
			{
				if (key.IsGamepadKey()) continue;
				FinalKey = key;
			}
			break;
		case EInputRestriction::Gamepad:
			for (FKey key : keys)
			{
				if (!key.IsGamepadKey()) continue;
				FinalKey = key;
			}
			break;
	}

	return FinalKey;
}

UTexture2D * UUINavPCComponent::GetMenuActionIcon(FString ActionName, EInputRestriction InputRestriction)
{
	FKey Key = GetMenuActionKey(ActionName, InputRestriction);
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

FString UUINavPCComponent::GetMenuActionName(FString ActionName, EInputRestriction InputRestriction)
{
	FKey Key = GetMenuActionKey(ActionName, InputRestriction);
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

FReply UUINavPCComponent::OnActionPressed(FString ActionName)
{
	if (!PressedActions.Contains(ActionName))
	{
		PressedActions.AddUnique(ActionName);
		ExecuteActionByName(ActionName, true);
		return FReply::Handled();
	}
	else return FReply::Unhandled();
}

FReply UUINavPCComponent::OnActionReleased(FString ActionName)
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
	OnInputChanged(CurrentInputType, NewInputType);

	if (ActiveWidget != nullptr) ActiveWidget->OnInputChanged(CurrentInputType, NewInputType);

	CurrentInputType = NewInputType;
}

void UUINavPCComponent::OnRootWidgetRemoved_Implementation()
{

}

void UUINavPCComponent::OnInputChanged_Implementation(EInputType From, EInputType To)
{

}

void UUINavPCComponent::OnNavigated_Implementation(ENavigationDirection NewDirection)
{

}

void UUINavPCComponent::OnSelect_Implementation()
{
}

void UUINavPCComponent::OnReturn_Implementation()
{
}

void UUINavPCComponent::MenuUp()
{
	OnNavigated(ENavigationDirection::Up);
	VerifyInputTypeChangeByAction(TEXT("MenuUp"));

	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	ActiveWidget->NavigateInDirection(ENavigationDirection::Up);
}

void UUINavPCComponent::MenuDown()
{
	OnNavigated(ENavigationDirection::Down);
	VerifyInputTypeChangeByAction(TEXT("MenuDown"));

	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	ActiveWidget->NavigateInDirection(ENavigationDirection::Down);
}

void UUINavPCComponent::MenuLeft()
{
	OnNavigated(ENavigationDirection::Left);
	VerifyInputTypeChangeByAction(TEXT("MenuLeft"));

	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	ActiveWidget->NavigateInDirection(ENavigationDirection::Left);
}

void UUINavPCComponent::MenuRight()
{
	OnNavigated(ENavigationDirection::Right);
	VerifyInputTypeChangeByAction(TEXT("MenuRight"));

	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	ActiveWidget->NavigateInDirection(ENavigationDirection::Right);
}

void UUINavPCComponent::MenuSelect()
{
	OnSelect();
	VerifyInputTypeChangeByAction(TEXT("MenuSelect"));

	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	ClearTimer();
	ActiveWidget->MenuSelect();
}

void UUINavPCComponent::MenuReturn()
{
	OnReturn();
	VerifyInputTypeChangeByAction(TEXT("MenuReturn"));

	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

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
	Direction = ENavigationDirection::Up;

	if (!bChainNavigation || ActiveWidget == nullptr || !ActiveWidget->IsInViewport()) return;

	SetTimer(ENavigationDirection::Up);
}

void UUINavPCComponent::StartMenuDown()
{
	if (Direction == ENavigationDirection::Down) return;

	MenuDown();
	Direction = ENavigationDirection::Down;

	if (!bChainNavigation || ActiveWidget == nullptr || !ActiveWidget->IsInViewport()) return;

	SetTimer(ENavigationDirection::Down);
}

void UUINavPCComponent::StartMenuLeft()
{
	if (Direction == ENavigationDirection::Left) return;

	MenuLeft();
	Direction = ENavigationDirection::Left;

	if (!bChainNavigation || ActiveWidget == nullptr || !ActiveWidget->IsInViewport()) return;

	SetTimer(ENavigationDirection::Left);
}

void UUINavPCComponent::StartMenuRight()
{
	if (Direction == ENavigationDirection::Right) return;

	MenuRight();
	Direction = ENavigationDirection::Right;

	if (!bChainNavigation || ActiveWidget == nullptr || !ActiveWidget->IsInViewport()) return;

	SetTimer(ENavigationDirection::Right);
}
