// Fill out your copyright notice in the Description page of Project Settings.


#include "UINavInputDisplay.h"
#include "GameFramework/PlayerController.h"
#include "UINavPCComponent.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UUINavInputDisplay::NativeConstruct()
{
	Super::NativeConstruct();

	APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
	if (!IsValid(PC))
	{
		return;
	}

	UINavPC = PC->FindComponentByClass<UUINavPCComponent>();
	if (!IsValid(UINavPC))
	{
		return;
	}

	UINavPC->InputTypeChangedDelegate.AddDynamic(this, &UUINavInputDisplay::InputTypeChanged);

	UpdateInputVisuals();
}

void UUINavInputDisplay::NativeDestruct()
{
	if (!IsValid(UINavPC))
	{
		return;
	}

	UINavPC->InputTypeChangedDelegate.RemoveDynamic(this, &UUINavInputDisplay::InputTypeChanged);

	Super::NativeDestruct();
}

void UUINavInputDisplay::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsValid(InputText))
	{
		if (bOverride_Font)
		{
			InputText->SetFont(FontOverride);
		}
		else
		{
			FontOverride = InputText->Font;
		}

		if (bOverride_TextColor)
		{
			InputText->SetColorAndOpacity(TextColorOverride);
		}
		else
		{
			TextColorOverride = InputText->ColorAndOpacity;
		}
	}

	if (IsValid(InputImage) && !bMatchIconSize)
	{
		InputImage->SetDesiredSizeOverride(IconSize);
	}
}

void UUINavInputDisplay::UpdateInputVisuals()
{
	if (!IsValid(UINavPC))
	{
		return;
	}

	UTexture2D* NewTexture = UINavPC->GetEnhancedInputIcon(InputAction, Axis, Scale, UINavPC->IsUsingGamepad() ? EInputRestriction::Gamepad : EInputRestriction::Keyboard_Mouse);
	if (NewTexture != nullptr)
	{
		InputImage->SetBrushFromTexture(NewTexture, bMatchIconSize);
		if (!bMatchIconSize)
		{
			InputImage->SetDesiredSizeOverride(IconSize);
		}

		InputText->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		InputText->SetText(UINavPC->GetEnhancedInputText(InputAction, Axis, Scale));
		InputImage->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UUINavInputDisplay::InputTypeChanged(const EInputType NewInputType)
{
	UpdateInputVisuals();
}

void UUINavInputDisplay::SetInputAction(UInputAction* NewAction, const EInputAxis NewAxis, const EAxisType NewScale)
{
	InputAction = NewAction;
	Axis = NewAxis;
	Scale = NewScale;

	UpdateInputVisuals();
}
