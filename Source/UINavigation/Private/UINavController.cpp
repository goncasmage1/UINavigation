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

bool AUINavController::IsGamepadConnected()
{
	return FSlateApplication::Get().IsGamepadAttached();
}

void AUINavController::SetActiveWidget(UUINavWidget* NewWidget)
{
	ActiveWidget = NewWidget;
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

	ActiveWidget->MenuSelect();
}

void AUINavController::MenuReturn()
{
	if (ActiveWidget == nullptr ||
		!ActiveWidget->IsInViewport()) return;

	ActiveWidget->MenuReturn();
}


