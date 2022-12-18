// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavSlider.h"
#include "UINavMacros.h"
#include "UINavWidget.h"
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
		NavSpinBox->ClearKeyboardFocusOnCommit = true;
		if (!NavSpinBox->OnValueChanged.IsBound())
			NavSpinBox->OnValueCommitted.AddDynamic(this, &UUINavSlider::HandleOnSpinBoxValueChanged);
	}

	if (!Slider->OnValueChanged.IsBound()) Slider->OnValueChanged.AddDynamic(this, &UUINavSlider::HandleOnSliderValueChanged);
	if (!Slider->OnMouseCaptureBegin.IsBound()) Slider->OnMouseCaptureBegin.AddDynamic(this, &UUINavSlider::HandleOnMouseCaptureBegin);
	if (!Slider->OnMouseCaptureEnd.IsBound()) Slider->OnMouseCaptureEnd.AddDynamic(this, &UUINavSlider::HandleOnMouseCaptureEnd);

	Difference = MaxValue - MinValue;
	Slider->IsFocusable = false;
	Slider->StepSize = Interval / Difference;

	HandleDefaultColor = Slider->SliderHandleColor;
	BarDefaultColor = Slider->SliderBarColor;

	Update();
}

void UUINavSlider::Update()
{
	Super::Update();

	Slider->SetValue(static_cast<float>(OptionIndex) / static_cast<float>(GetMaxOptionIndex()));
	FNumberFormattingOptions FormatOptions = FNumberFormattingOptions();
	FormatOptions.MaximumFractionalDigits = MaxDecimalDigits;
	FormatOptions.MinimumFractionalDigits = MinDecimalDigits;

	const float Value = MinValue + Slider->Value * Difference;
	FText ValueText = FText::AsNumber(Value, &FormatOptions);
	if (!bUseComma) ValueText = FText::FromString(ValueText.ToString().Replace(TEXT(","),TEXT(".")));

	if (NavText != nullptr) NavText->SetText(ValueText);
	if (NavSpinBox != nullptr)
	{
		NavSpinBox->SetValue(Value);
	}
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

void UUINavSlider::SetValueClamped(const float Value)
{
	OptionIndex = IndexFromValue(Value);
	Update();
}

float UUINavSlider::GetSliderValue() const
{
	return (Slider->GetValue());
}

void UUINavSlider::NavigateLeft()
{
	if (OptionIndex > 0)
	{
		OptionIndex--;
	}
	else if (bLoopOptions) OptionIndex = GetMaxOptionIndex();

	Update();

	OnNavigateLeft();
}

void UUINavSlider::NavigateRight()
{
	if (OptionIndex < GetMaxOptionIndex())
	{
		OptionIndex++;
	}
	else if (bLoopOptions) OptionIndex = 0;

	Update();

	OnNavigateRight();
}

void UUINavSlider::HandleOnSliderValueChanged(float InValue)
{
	OptionIndex = IndexFromPercent(InValue);
	Update();
}

void UUINavSlider::HandleOnMouseCaptureBegin()
{
	LastOptionIndex = OptionIndex;
}

void UUINavSlider::HandleOnMouseCaptureEnd()
{
	if (OptionIndex != LastOptionIndex &&
		ParentWidget != nullptr)
	{
		OnUpdated();
		ParentWidget->OnHorizCompUpdated(NavButton->ButtonIndex);
	}
}

void UUINavSlider::HandleOnSpinBoxValueChanged(float InValue, ETextCommit::Type CommitMethod)
{
	OptionIndex = IndexFromValue(InValue);
	Update();
}

float UUINavSlider::IndexFromPercent(const float Value)
{
	float Div = Value / Slider->StepSize;
	const int MaxOptionIndex = GetMaxOptionIndex();
	if (Div > MaxOptionIndex) Div = MaxOptionIndex;

	const int FlatDiv = FMath::RoundToInt(Div);
	const float Decimal = Div - FlatDiv;
	return Decimal < 0.5 ? FlatDiv : (FlatDiv + 1 <= MaxOptionIndex ? FlatDiv + 1 : MaxOptionIndex);
}

float UUINavSlider::IndexFromValue(const float Value)
{
	return IndexFromPercent(Value <= MinValue ? 0.f : (Value >= MaxValue ? 1.f : (Value - MinValue) / Difference));
}
