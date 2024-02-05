// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved


#include "UINavInputDisplay.h"
#include "GameFramework/PlayerController.h"
#include "UINavPCComponent.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "UINavSettings.h"
#include "UINavBlueprintFunctionLibrary.h"

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

	UINavPC->UpdateInputIconsDelegate.AddDynamic(this, &UUINavInputDisplay::UpdateInputVisuals);

	UpdateInputVisuals();
}

void UUINavInputDisplay::NativeDestruct()
{
	if (!IsValid(UINavPC))
	{
		return;
	}

	UINavPC->UpdateInputIconsDelegate.RemoveDynamic(this, &UUINavInputDisplay::UpdateInputVisuals);

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

	if (IsValid(InputRichText))
	{
		InputRichText->SetText(StyleRowName.IsEmpty() ? InputRichText->GetText() : UUINavBlueprintFunctionLibrary::ApplyStyleRowToText(InputRichText->GetText(), StyleRowName));
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

	if (IsValid(InputText)) InputText->SetVisibility(ESlateVisibility::Collapsed);
	if (IsValid(InputRichText)) InputRichText->SetVisibility(ESlateVisibility::Collapsed);
	InputImage->SetVisibility(ESlateVisibility::Collapsed);

	TSoftObjectPtr<UTexture2D> NewSoftTexture = GetDefault<UUINavSettings>()->bLoadInputIconsAsync ?
		UINavPC->GetSoftEnhancedInputIcon(InputAction, Axis, Scale, UINavPC->IsUsingGamepad() ? EInputRestriction::Gamepad : EInputRestriction::Keyboard_Mouse) :
		UINavPC->GetEnhancedInputIcon(InputAction, Axis, Scale, UINavPC->IsUsingGamepad() ? EInputRestriction::Gamepad : EInputRestriction::Keyboard_Mouse);
	if (!NewSoftTexture.IsNull() && DisplayType != EInputDisplayType::Text)
	{
		InputImage->SetBrushFromSoftTexture(NewSoftTexture, bMatchIconSize);
		if (!bMatchIconSize)
		{
			InputImage->SetDesiredSizeOverride(IconSize);
		}

		InputImage->SetVisibility(ESlateVisibility::Visible);
	}
	if (NewSoftTexture.IsNull() || DisplayType != EInputDisplayType::Icon)
	{
		const FText InputRawText = UINavPC->GetEnhancedInputText(InputAction, Axis, Scale, UINavPC->IsUsingGamepad() ? EInputRestriction::Gamepad : EInputRestriction::Keyboard_Mouse);
		if (IsValid(InputText))
		{
			InputText->SetText(InputRawText);
			InputText->SetVisibility(ESlateVisibility::Visible);
		}

		if (IsValid(InputRichText))
		{
			InputRichText->SetText(StyleRowName.IsEmpty() ? InputRawText : UUINavBlueprintFunctionLibrary::ApplyStyleRowToText(InputRawText, StyleRowName));
			InputRichText->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UUINavInputDisplay::SetInputAction(UInputAction* NewAction, const EInputAxis NewAxis, const EAxisType NewScale)
{
	InputAction = NewAction;
	Axis = NewAxis;
	Scale = NewScale;

	UpdateInputVisuals();
}
