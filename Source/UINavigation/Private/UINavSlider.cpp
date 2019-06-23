// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavSlider.h"
#include "Slider.h"
#include "TextBlock.h"

void UUINavSlider::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = false;

	if (!Slider->OnValueChanged.IsBound()) Slider->OnValueChanged.AddDynamic(this, &UUINavSlider::HandleOnValueChanged);
	Slider->StepSize = Interval / (MaxValue - MinValue);

	if (MinValue >= MaxValue)
	{
		DISPLAYERROR(TEXT("MaxValue should be greater than MinValue"));
	}
	if (Interval > (MaxValue - MinValue))
	{
		DISPLAYERROR(TEXT("Interval shouldn't be greater than difference between MaxValue and MinValue"));
	}
	if (MinDecimalDigits > MaxDecimalDigits)
	{
		DISPLAYERROR(TEXT("MinDecimalDigits shouldn't be greater than MaxDecimalDigits"));
	}

	MaxOption = (MaxValue - MinValue) / Interval - 1;
}

void UUINavSlider::Update()
{
	Slider->SetValue((float)OptionIndex / (float)MaxOption);
	FNumberFormattingOptions FormatOptions;
	FormatOptions.MaximumFractionalDigits = MaxDecimalDigits;
	FormatOptions.MinimumFractionalDigits = MinDecimalDigits;
	FText NumberText = FText::AsNumber(Slider->Value, &FormatOptions);
	if (!bUseComma) NumberText = FText::FromString(NumberText.ToString().Replace(TEXT(","),TEXT(".")));
	if (NavText != nullptr) NavText->SetText(NumberText);
}

void UUINavSlider::NavigateLeft()
{
	if (OptionIndex > 0)
	{
		OptionIndex--;
	}

	Update();

	OnNavigateLeft();
}

void UUINavSlider::NavigateRight()
{
	if (OptionIndex < MaxOption)
	{
		OptionIndex++;
	}
	else
	{
		OnNavigateRight();
		return;
	}

	Update();

	OnNavigateRight();
}

void UUINavSlider::HandleOnValueChanged(float InValue)
{
	OptionIndex = IndexFromPercent(InValue);
	Update();
}

float UUINavSlider::IndexFromPercent(float Value)
{
	float Div = Value / Slider->StepSize;
	if (Div > MaxOption) Div = MaxOption;

	int FlatDiv = (int)Div;
	float Decimal = Div - FlatDiv;
	return Decimal < 0.5 ? FlatDiv : (FlatDiv + 1 <= MaxOption ? FlatDiv + 1 : MaxOption);
}
