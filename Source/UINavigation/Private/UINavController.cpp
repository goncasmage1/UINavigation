// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavController.h"
#include "UINavWidget.h"


void AUINavController::SetupInputComponent()
{
	Super::SetupInputComponent();
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

void AUINavController::VerifyInputTypeChange(FKey Key)
{
	EInputType NewInputType = GetKeyInputType(Key);

	if (NewInputType != CurrentInputType)
	{
		NotifyInputTypeChange(NewInputType);
	}
}

void AUINavController::NotifyKeyPressed(FKey PressedKey)
{
	FindActionByKey(PressedKey, true);
}

void AUINavController::NotifyKeyReleased(FKey ReleasedKey)
{
	FindActionByKey(ReleasedKey, false);
}

void AUINavController::FindActionByKey(FKey PressedKey, bool bPressed)
{
	TArray<FString> Actions;
	KeyMap.GenerateKeyArray(Actions);
	for (FString action : Actions)
	{
		for (FKey key : KeyMap[action])
		{
			if (key == PressedKey)
			{
				VerifyInputTypeChange(PressedKey);
				ExecuteActionByName(action, bPressed);
				return;
			}
		}
	}
}

void AUINavController::ExecuteActionByName(FString Action, bool bPressed)
{
	if (Action.Equals("MenuUp"))
	{
		if (bPressed)
		{
			StartMenuUp();
		}
		else
		{
			MenuUpRelease();
		}
	}
	else if (Action.Equals("MenuDown"))
	{
		if (bPressed)
		{
			StartMenuDown();
		}
		else
		{
			MenuDownRelease();
		}
	}
	else if (Action.Equals("MenuLeft"))
	{
		if (bPressed)
		{
			StartMenuLeft();
		}
		else
		{
			MenuLeftRelease();
		}
	}
	else if (Action.Equals("MenuRight"))
	{
		if (bPressed)
		{
			StartMenuRight();
		}
		else
		{
			MenuRightRelease();
		}
	}
	else if (Action.Equals("MenuSelect"))
	{
		if (bPressed)
		{
			MenuSelect();
		}
	}
	else if (Action.Equals("MenuReturn"))
	{
		if (bPressed)
		{
			MenuReturn();
		}
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

bool AUINavController::IsGamepadConnected()
{
	return FSlateApplication::Get().IsGamepadAttached();
}

void AUINavController::MenuUp()
{
	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	ActiveWidget->MenuUp();
}

void AUINavController::MenuDown()
{
	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	ActiveWidget->MenuDown();
}

void AUINavController::MenuLeft()
{
	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	ActiveWidget->MenuLeft();
}

void AUINavController::MenuRight()
{
	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	ActiveWidget->MenuRight();
}

void AUINavController::MenuSelect()
{
	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	GetWorldTimerManager().ClearTimer(NavChainHandle);
	ActiveWidget->MenuSelect();
}

void AUINavController::MenuReturn()
{
	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

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