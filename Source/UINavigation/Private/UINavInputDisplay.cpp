// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved


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
			FontOverride = InputText->GetFont();
		}

		if (bOverride_TextColor)
		{
			InputText->SetColorAndOpacity(TextColorOverride);
		}
		else
		{
			TextColorOverride = InputText->GetColorAndOpacity();
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

	InputText->SetVisibility(ESlateVisibility::Collapsed);
	InputImage->SetVisibility(ESlateVisibility::Collapsed);
	UTexture2D* NewTexture = UINavPC->GetEnhancedInputIcon(InputAction, Axis, Scale, UINavPC->IsUsingGamepad() ? EInputRestriction::Gamepad : EInputRestriction::Keyboard_Mouse);
	if (NewTexture != nullptr && DisplayType != EInputDisplayType::Text)
	{
		InputImage->SetBrushFromTexture(NewTexture, bMatchIconSize);
		if (!bMatchIconSize)
		{
			InputImage->SetDesiredSizeOverride(IconSize);
		}

		InputImage->SetVisibility(ESlateVisibility::Visible);
	}
	if (NewTexture == nullptr || DisplayType != EInputDisplayType::Icon)
	{
		InputText->SetText(UINavPC->GetEnhancedInputText(InputAction, Axis, Scale, UINavPC->IsUsingGamepad() ? EInputRestriction::Gamepad : EInputRestriction::Keyboard_Mouse));

		InputText->SetVisibility(ESlateVisibility::Visible);
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
