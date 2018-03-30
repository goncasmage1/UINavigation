// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavController.h"
#include "UINavWidget.h"


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
}

void AUINavController::Possess(APawn * InPawn)
{
	Super::Possess(InPawn);

	FetchUINavActionKeys();
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

bool AUINavController::IsGamepadConnected()
{
	return FSlateApplication::Get().IsGamepadAttached();
}

void AUINavController::SetActiveWidget(UUINavWidget* NewWidget)
{
	ActiveWidget = NewWidget;
}

EInputType AUINavController::GetLastInputType(FString ActionName)
{
	TArray<FKey>* Keys = KeyMap.Find(ActionName);
	if (Keys == nullptr) return EInputType::None;

	for (FKey Key : *Keys)
	{
		if (WasInputKeyJustPressed(Key))
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
	}

	return CurrentInputType;
}

void AUINavController::VerifyInputType(FString ActionName)
{
	EInputType NewInputType = GetLastInputType(ActionName);

	if (NewInputType != CurrentInputType)
	{
		NotifyInputTypeChange(NewInputType);
	}
}

void AUINavController::NotifyMouseInputType()
{
	EInputType NewInputType = EInputType::Mouse;

	if (NewInputType != CurrentInputType)
	{
		NotifyInputTypeChange(NewInputType);
	}
}

void AUINavController::NotifyInputTypeChange(EInputType NewInputType)
{
	GetWorldTimerManager().ClearTimer(NavChainHandle);

	ActiveWidget->OnInputChanged(CurrentInputType, NewInputType);
	CurrentInputType = NewInputType;
}

void AUINavController::MenuUp()
{
	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	VerifyInputType(TEXT("MenuUp"));

	ActiveWidget->MenuUp();
}

void AUINavController::MenuDown()
{
	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	VerifyInputType(TEXT("MenuDown"));

	ActiveWidget->MenuDown();
}

void AUINavController::MenuLeft()
{
	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	VerifyInputType(TEXT("MenuLeft"));

	ActiveWidget->MenuLeft();
}

void AUINavController::MenuRight()
{
	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	VerifyInputType(TEXT("MenuRight"));

	ActiveWidget->MenuRight();
}

void AUINavController::MenuSelect()
{
	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	VerifyInputType(TEXT("MenuSelect"));

	GetWorldTimerManager().ClearTimer(NavChainHandle);
	ActiveWidget->MenuSelect();
}

void AUINavController::MenuReturn()
{
	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	VerifyInputType(TEXT("MenuReturn"));

	GetWorldTimerManager().ClearTimer(NavChainHandle);
	ActiveWidget->MenuReturn();
}

void AUINavController::MenuUpRelease()
{
	if (Direction != EInputDirection::Up) return;

	GetWorldTimerManager().ClearTimer(NavChainHandle);
}

void AUINavController::MenuDownRelease()
{
	if (Direction != EInputDirection::Down) return;

	GetWorldTimerManager().ClearTimer(NavChainHandle);
}

void AUINavController::MenuLeftRelease()
{
	if (Direction != EInputDirection::Left) return;

	GetWorldTimerManager().ClearTimer(NavChainHandle);
}

void AUINavController::MenuRightRelease()
{
	if (Direction != EInputDirection::Right) return;

	GetWorldTimerManager().ClearTimer(NavChainHandle);
}

void AUINavController::StartMenuUp()
{
	MenuUp();
	Direction = EInputDirection::Up;

	if (!bChainNavigation) return;
	GetWorldTimerManager().SetTimer(NavChainHandle, this, &AUINavController::MenuUp, NavigationChainFrequency, true, InputHeldWaitTime);
}

void AUINavController::StartMenuDown()
{
	MenuDown();
	Direction = EInputDirection::Down;

	if (!bChainNavigation) return;
	GetWorldTimerManager().SetTimer(NavChainHandle, this, &AUINavController::MenuDown, NavigationChainFrequency, true, InputHeldWaitTime);
}

void AUINavController::StartMenuLeft()
{
	MenuLeft();
	Direction = EInputDirection::Left;

	if (!bChainNavigation) return;
	GetWorldTimerManager().SetTimer(NavChainHandle, this, &AUINavController::MenuLeft, NavigationChainFrequency, true, InputHeldWaitTime);
}

void AUINavController::StartMenuRight()
{
	MenuRight();
	Direction = EInputDirection::Right;

	if (!bChainNavigation) return;
	GetWorldTimerManager().SetTimer(NavChainHandle, this, &AUINavController::MenuRight, NavigationChainFrequency, true, InputHeldWaitTime);
}