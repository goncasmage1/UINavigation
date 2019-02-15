// Copyright (C) 2018 Gonçalo Marques - All Rights Reserved

#include "UINavController.h"
#include "UINavWidget.h"
#include "UINavSettings.h"


void AUINavController::SetupInputComponent()
{
	Super::SetupInputComponent();

	FInputActionBinding& Action1_1 = InputComponent->BindAction("MenuUp", IE_Pressed, this, &AUINavController::StartMenuUp);
	Action1_1.bExecuteWhenPaused = true;
	Action1_1.bConsumeInput = false;
	FInputActionBinding& Action2_1 = InputComponent->BindAction("MenuDown", IE_Pressed, this, &AUINavController::StartMenuDown);
	Action2_1.bExecuteWhenPaused = true;
	Action2_1.bConsumeInput = false;
	FInputActionBinding& Action3_1 = InputComponent->BindAction("MenuLeft", IE_Pressed, this, &AUINavController::StartMenuLeft);
	Action3_1.bExecuteWhenPaused = true;
	Action3_1.bConsumeInput = false;
	FInputActionBinding& Action4_1 = InputComponent->BindAction("MenuRight", IE_Pressed, this, &AUINavController::StartMenuRight);
	Action4_1.bExecuteWhenPaused = true;
	Action4_1.bConsumeInput = false;
	FInputActionBinding& Action5_1 = InputComponent->BindAction("MenuSelect", IE_Pressed, this, &AUINavController::MenuSelect);
	Action5_1.bExecuteWhenPaused = true;
	Action5_1.bConsumeInput = false;
	FInputActionBinding& Action6_1 = InputComponent->BindAction("MenuReturn", IE_Pressed, this, &AUINavController::MenuReturn);
	Action6_1.bExecuteWhenPaused = true;
	Action6_1.bConsumeInput = false;

	FInputActionBinding& Action1_2 = InputComponent->BindAction("MenuUp", IE_Released, this, &AUINavController::MenuUpRelease);
	Action1_2.bExecuteWhenPaused = true;
	Action1_2.bConsumeInput = false;
	FInputActionBinding& Action2_2 = InputComponent->BindAction("MenuDown", IE_Released, this, &AUINavController::MenuDownRelease);
	Action2_2.bExecuteWhenPaused = true;
	Action2_2.bConsumeInput = false;
	FInputActionBinding& Action3_2 = InputComponent->BindAction("MenuLeft", IE_Released, this, &AUINavController::MenuLeftRelease);
	Action3_2.bExecuteWhenPaused = true;
	Action3_2.bConsumeInput = false;
	FInputActionBinding& Action4_2 = InputComponent->BindAction("MenuRight", IE_Released, this, &AUINavController::MenuRightRelease);
	Action4_2.bExecuteWhenPaused = true;
	Action4_2.bConsumeInput = false;

	FInputKeyBinding& Action1_3 = InputComponent->BindKey(EKeys::AnyKey, IE_Pressed, this, &AUINavController::MouseInputWorkaround);
	Action1_3.bExecuteWhenPaused = true;
	Action1_3.bConsumeInput = false;
}

void AUINavController::BeginPlay()
{
	Super::BeginPlay();

	VerifyDefaultInputs();
}

void AUINavController::VerifyDefaultInputs()
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

void AUINavController::Possess(APawn * InPawn)
{
	Super::Possess(InPawn);

	FetchUINavActionKeys();
}

void AUINavController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	GetMousePosition(PosX, PosY);
	if (CurrentInputType != EInputType::Mouse)
	{
		if (PosX != PreviousX || PosY != PreviousY)
		{
			NotifyMouseInputType();
		}
	}
	PreviousX = PosX;
	PreviousY = PosY;
}

void AUINavController::TimerCallback()
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

void AUINavController::SetTimer(ENavigationDirection TimerDirection)
{
	TimerCounter = 0.f;
	CallbackDirection = TimerDirection;
	CountdownPhase = ECountdownPhase::First;
}


void AUINavController::ClearTimer()
{
	if (CallbackDirection == ENavigationDirection::None) return;

	TimerCounter = 0.f;
	CallbackDirection = ENavigationDirection::None;
	CountdownPhase = ECountdownPhase::None;
}

void AUINavController::FetchUINavActionKeys()
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
}

void AUINavController::SetActiveWidget(UUINavWidget* NewWidget)
{
	ActiveWidget = NewWidget;
}

EInputType AUINavController::GetKeyInputType(FKey Key)
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

EInputType AUINavController::GetActionInputType(FString Action)
{
	for (FKey key : KeyMap[Action])
	{
		if (WasInputKeyJustPressed(key)) return GetKeyInputType(key);
	}
	return CurrentInputType;
}

void AUINavController::VerifyInputTypeChangeByKey(FKey Key)
{
	EInputType NewInputType = GetKeyInputType(Key);

	if (NewInputType != CurrentInputType)
	{
		NotifyInputTypeChange(NewInputType);
	}
}

void AUINavController::VerifyInputTypeChangeByAction(FString Action)
{
	EInputType NewInputType = GetActionInputType(Action);

	if (NewInputType != CurrentInputType)
	{
		NotifyInputTypeChange(NewInputType);
	}
}

void AUINavController::NotifyKeyPressed(FKey PressedKey)
{
	ExecuteActionByKey(PressedKey, true);
}

void AUINavController::NotifyKeyReleased(FKey ReleasedKey)
{
	ExecuteActionByKey(ReleasedKey, false);
}

bool AUINavController::IsReturnKey(FKey PressedKey)
{
	for (FKey key : KeyMap[TEXT("MenuReturn")]) if (key == PressedKey) return true;
	return false;
}

void AUINavController::ExecuteActionByKey(FKey PressedKey, bool bPressed)
{
	FString ActionName = FindActionByKey(PressedKey);
	if (ActionName.Equals(TEXT(""))) return;

	ExecuteActionByName(ActionName, bPressed);
}

FString AUINavController::FindActionByKey(FKey ActionKey)
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

FReply AUINavController::OnActionPressed(FString ActionName)
{
	if (!PressedActions.Contains(ActionName))
	{
		PressedActions.AddUnique(ActionName);
		ExecuteActionByName(ActionName, true);
		return FReply::Unhandled();
	}
	else return FReply::Handled();
}

FReply AUINavController::OnActionReleased(FString ActionName)
{
	if (PressedActions.Contains(ActionName))
	{
		PressedActions.Remove(ActionName);
		ExecuteActionByName(ActionName, false);
		return FReply::Unhandled();
	}
	else return FReply::Handled();
}

void AUINavController::ExecuteActionByName(FString Action, bool bPressed)
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

void AUINavController::NotifyMouseInputType()
{
	if (EInputType::Mouse != CurrentInputType)
	{
		NotifyInputTypeChange(EInputType::Mouse);
	}
}

void AUINavController::NotifyInputTypeChange(EInputType NewInputType)
{
	OnInputChanged(CurrentInputType, NewInputType);

	if (ActiveWidget != nullptr) ActiveWidget->OnInputChanged(CurrentInputType, NewInputType);

	CurrentInputType = NewInputType;
}

void AUINavController::OnRootWidgetRemoved_Implementation()
{

}

void AUINavController::OnInputChanged_Implementation(EInputType From, EInputType To)
{

}

void AUINavController::OnNavigated_Implementation(ENavigationDirection NewDirection)
{

}

void AUINavController::OnSelect_Implementation()
{
}

void AUINavController::OnReturn_Implementation()
{
}

void AUINavController::MenuUp()
{
	OnNavigated(ENavigationDirection::Up);
	VerifyInputTypeChangeByAction(TEXT("MenuUp"));

	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	ActiveWidget->NavigateInDirection(ENavigationDirection::Up);
}

void AUINavController::MenuDown()
{
	OnNavigated(ENavigationDirection::Down);
	VerifyInputTypeChangeByAction(TEXT("MenuDown"));

	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	ActiveWidget->NavigateInDirection(ENavigationDirection::Down);
}

void AUINavController::MenuLeft()
{
	OnNavigated(ENavigationDirection::Left);
	VerifyInputTypeChangeByAction(TEXT("MenuLeft"));

	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	ActiveWidget->NavigateInDirection(ENavigationDirection::Left);
}

void AUINavController::MenuRight()
{
	OnNavigated(ENavigationDirection::Right);
	VerifyInputTypeChangeByAction(TEXT("MenuRight"));

	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	ActiveWidget->NavigateInDirection(ENavigationDirection::Right);
}

void AUINavController::MenuSelect()
{
	OnSelect();
	VerifyInputTypeChangeByAction(TEXT("MenuSelect"));

	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	ClearTimer();
	ActiveWidget->MenuSelect();
}

void AUINavController::MenuReturn()
{
	OnReturn();
	VerifyInputTypeChangeByAction(TEXT("MenuReturn"));

	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	ClearTimer();
	ActiveWidget->MenuReturn();
}

void AUINavController::MenuUpRelease()
{
	if (Direction != ENavigationDirection::Up) return;
	Direction = ENavigationDirection::None;

	ClearTimer();
}

void AUINavController::MenuDownRelease()
{
	if (Direction != ENavigationDirection::Down) return;
	Direction = ENavigationDirection::None;

	ClearTimer();
}

void AUINavController::MenuLeftRelease()
{
	if (Direction != ENavigationDirection::Left) return;
	Direction = ENavigationDirection::None;

	ClearTimer();
}

void AUINavController::MenuRightRelease()
{
	if (Direction != ENavigationDirection::Right) return;
	Direction = ENavigationDirection::None;

	ClearTimer();
}

void AUINavController::MouseInputWorkaround()
{
	if (ActiveWidget != nullptr && ActiveWidget->bWaitForInput)
	{
		if (WasInputKeyJustPressed(EKeys::LeftMouseButton)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::LeftMouseButton));
		else if (WasInputKeyJustPressed(EKeys::RightMouseButton)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::RightMouseButton));
		else if (WasInputKeyJustPressed(EKeys::MiddleMouseButton)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::MiddleMouseButton));
		else if (WasInputKeyJustPressed(EKeys::MouseScrollUp)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::MouseScrollUp));
		else if (WasInputKeyJustPressed(EKeys::MouseScrollDown)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::MouseScrollDown));
		else if (WasInputKeyJustPressed(EKeys::ThumbMouseButton)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::ThumbMouseButton));
		else if (WasInputKeyJustPressed(EKeys::ThumbMouseButton2)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::ThumbMouseButton2));
		else if (WasInputKeyJustPressed(EKeys::MouseScrollDown)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::MouseScrollDown));
		else if (WasInputKeyJustPressed(EKeys::MouseScrollUp)) ActiveWidget->ProcessMouseKeybind(FKey(EKeys::MouseScrollUp));
	}
}

void AUINavController::StartMenuUp()
{
	if (Direction == ENavigationDirection::Up) return;

	MenuUp();
	Direction = ENavigationDirection::Up;

	if (!bChainNavigation || ActiveWidget == nullptr || !ActiveWidget->IsInViewport()) return;

	SetTimer(ENavigationDirection::Up);
}

void AUINavController::StartMenuDown()
{
	if (Direction == ENavigationDirection::Down) return;

	MenuDown();
	Direction = ENavigationDirection::Down;

	if (!bChainNavigation || ActiveWidget == nullptr || !ActiveWidget->IsInViewport()) return;
	
	SetTimer(ENavigationDirection::Down);
}

void AUINavController::StartMenuLeft()
{
	if (Direction == ENavigationDirection::Left) return;

	MenuLeft();
	Direction = ENavigationDirection::Left;

	if (!bChainNavigation || ActiveWidget == nullptr || !ActiveWidget->IsInViewport()) return;

	SetTimer(ENavigationDirection::Left);
}

void AUINavController::StartMenuRight()
{
	if (Direction == ENavigationDirection::Right) return;

	MenuRight();
	Direction = ENavigationDirection::Right;

	if (!bChainNavigation || ActiveWidget == nullptr || !ActiveWidget->IsInViewport()) return;

	SetTimer(ENavigationDirection::Right);
}
