// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavController.h"
#include "UINavWidget.h"


void AUINavController::SetupInputComponent()
{
	Super::SetupInputComponent();

	FInputActionBinding& Action1 = InputComponent->BindAction("MenuUp", IE_Pressed, this, &AUINavController::MenuUp);
	Action1.bExecuteWhenPaused = true;
	Action1.bConsumeInput = false;
	FInputActionBinding& Action2 = InputComponent->BindAction("MenuDown", IE_Pressed, this, &AUINavController::MenuDown);
	Action2.bExecuteWhenPaused = true;
	Action2.bConsumeInput = false;
	FInputActionBinding& Action3 = InputComponent->BindAction("MenuLeft", IE_Pressed, this, &AUINavController::MenuLeft);
	Action3.bExecuteWhenPaused = true;
	Action3.bConsumeInput = false;
	FInputActionBinding& Action4 = InputComponent->BindAction("MenuRight", IE_Pressed, this, &AUINavController::MenuRight);
	Action4.bExecuteWhenPaused = true;
	Action4.bConsumeInput = false;
	FInputActionBinding& Action5 = InputComponent->BindAction("MenuSelect", IE_Pressed, this, &AUINavController::MenuSelect);
	Action5.bExecuteWhenPaused = true;
	Action5.bConsumeInput = false;
	FInputActionBinding& Action6 = InputComponent->BindAction("MenuReturn", IE_Pressed, this, &AUINavController::MenuReturn);
	Action6.bExecuteWhenPaused = true;
	Action6.bConsumeInput = false;
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

	return EInputType::None;
}

void AUINavController::VerifyInputType(FString ActionName)
{
	EInputType NewInputType = GetLastInputType(ActionName);

	if (NewInputType != CurrentInputType)
	{
		ActiveWidget->OnInputChanged(CurrentInputType, NewInputType);
		CurrentInputType = NewInputType;
	}
}

void AUINavController::NotifyMouseInputType()
{
	EInputType NewInputType = EInputType::Mouse;

	if (NewInputType != CurrentInputType)
	{
		ActiveWidget->OnInputChanged(CurrentInputType, NewInputType);
		CurrentInputType = NewInputType;
	}
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

	ActiveWidget->MenuSelect();
}

void AUINavController::MenuReturn()
{
	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	VerifyInputType(TEXT("MenuReturn"));

	ActiveWidget->MenuReturn();
}


