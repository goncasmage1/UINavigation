// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavSlider.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/SpinBox.h"

void UUINavSlider::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = false;

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

	if (NavSpinBox != nullptr)
	{
		NavSpinBox->SetMinSliderValue(MinValue);
		NavSpinBox->SetMinValue(MinValue);
		NavSpinBox->SetMaxSliderValue(MaxValue);
		NavSpinBox->SetMaxValue(MaxValue);
		if (!NavSpinBox->OnValueChanged.IsBound())
			NavSpinBox->OnValueCommitted.AddDynamic(this, &UUINavSlider::HandleOnSpinBoxValueChanged);
	}

	if (!Slider->OnValueChanged.IsBound()) Slider->OnValueChanged.AddDynamic(this, &UUINavSlider::HandleOnSliderValueChanged);
	Slider->StepSize = Interval / (MaxValue - MinValue);

	MaxOption = ((MaxValue - MinValue) / Interval);
	HandleDefaultColor = Slider->SliderHandleColor;
	BarDefaultColor = Slider->SliderBarColor;

	Update();
}

void UUINavSlider::Update()
{
	Slider->SetValue((float)OptionIndex / (float)MaxOption);
	FNumberFormattingOptions FormatOptions;
	FormatOptions.MaximumFractionalDigits = MaxDecimalDigits;
	FormatOptions.MinimumFractionalDigits = MinDecimalDigits;
	float Value = MinValue + Slider->Value * MaxValue;
	FText ValueText = FText::AsNumber(Value, &FormatOptions);
	if (!bUseComma) ValueText = FText::FromString(ValueText.ToString().Replace(TEXT(","),TEXT(".")));
	if (NavText != nullptr) NavText->SetText(ValueText);
	if (NavSpinBox != nullptr) NavSpinBox->SetValue(Value);
}

void UUINavSlider::OnNavigatedTo_Implementation()
{
	Slider->SetSliderHandleColor(HandleHoverColor);
	Slider->SetSliderBarColor(BarHoverColor);
}

void UUINavSlider::OnNavigatedFrom_Implementation()
{
	Slider->SetSliderHandleColor(HandleDefaultColor);
	Slider->SetSliderBarColor(BarDefaultColor);
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

void UUINavSlider::HandleOnSliderValueChanged(float InValue)
{
	OptionIndex = IndexFromPercent(InValue);
	Update();
}

void UUINavSlider::HandleOnSpinBoxValueChanged(float InValue, ETextCommit::Type CommitMethod)
{
	OptionIndex = IndexFromValue(InValue);
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

float UUINavSlider::IndexFromValue(float Value)
{
	return IndexFromPercent((Value - MinValue) / MaxValue);
}
