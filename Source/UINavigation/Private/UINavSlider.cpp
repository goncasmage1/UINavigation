// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavSlider.h"
#include "UINavMacros.h"
#include "UINavWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/SpinBox.h"
#include "UINavBlueprintFunctionLibrary.h"
#include "TimerManager.h"
#include "Widgets/Input/SSlider.h"

void UUINavSlider::NativePreConstruct()
{
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
		NavSpinBox->SetClearKeyboardFocusOnCommit(true);

		if (!IsDesignTime())
		{
			if (!NavSpinBox->OnValueCommitted.IsBound()) NavSpinBox->OnValueCommitted.AddUniqueDynamic(this, &UUINavSlider::HandleOnSpinBoxValueCommitted);
			if (!NavSpinBox->OnValueChanged.IsBound()) NavSpinBox->OnValueChanged.AddUniqueDynamic(this, &UUINavSlider::HandleOnSpinBoxValueChanged);
			if (!NavSpinBox->OnBeginSliderMovement.IsBound()) NavSpinBox->OnBeginSliderMovement.AddUniqueDynamic(this, &UUINavSlider::HandleOnSpinBoxMouseCaptureBegin);
		}
	}

	if (!IsDesignTime())
	{
		if (!Slider->OnMouseCaptureBegin.IsBound()) Slider->OnMouseCaptureBegin.AddUniqueDynamic(this, &UUINavSlider::HandleOnSliderMouseCaptureBegin);
		if (!Slider->OnValueChanged.IsBound()) Slider->OnValueChanged.AddUniqueDynamic(this, &UUINavSlider::HandleOnSliderValueChanged);
		if (!Slider->OnMouseCaptureEnd.IsBound()) Slider->OnMouseCaptureEnd.AddUniqueDynamic(this, &UUINavSlider::HandleOnSliderMouseCaptureEnd);
	}

	SanitizeValues();

	HandleDefaultColor = Slider->GetSliderHandleColor();
	BarDefaultColor = Slider->GetSliderBarColor();

	Update();

	Super::NativePreConstruct();
}

bool UUINavSlider::Update(const bool bNotify /*= true*/)
{
	const bool bChangedIndex = Super::Update(false);

	Slider->SetValue(static_cast<float>(OptionIndex) / static_cast<float>(GetMaxOptionIndex()));
	UpdateTextFromPercent(Slider->GetValue());

	if (bChangedIndex && bNotify)
	{
		NotifyUpdated();
	}

	return bChangedIndex;
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

	if (Update())
	{
		Super::NavigateLeft();
	}
}

void UUINavSlider::NavigateRight()
{
	if (OptionIndex < GetMaxOptionIndex())
	{
		OptionIndex++;
	}
	else if (bLoopOptions) OptionIndex = 0;

	if (Update())
	{
		Super::NavigateRight();
	}
}

void UUINavSlider::HandleOnSliderValueChanged(float InValue)
{
	const int32 NewOptionIndex = IndexFromPercent(InValue);
	const float NewValuePercent = static_cast<float>(NewOptionIndex) / static_cast<float>(GetMaxOptionIndex());
	UpdateTextFromPercent(NewValuePercent, /*bUpdateSpinBox*/ !bMovingSpinBox);
}

void UUINavSlider::HandleOnSliderMouseCaptureBegin()
{
	bMovingSlider = true;
}

void UUINavSlider::HandleOnSliderMouseCaptureEnd()
{
	bMovingSlider = false;
	OptionIndex = IndexFromPercent(Slider->GetValue());
	Update();
	// Workaround for a bug in SSlider: on mouse up, it "restores" the cursor to the default cursor,
	// rather than to unset, which prevents bubbling up to UUINavGameViewportClient.
	StaticCastSharedRef<SSlider>(Slider->TakeWidget())->SetCursor(TOptional<EMouseCursor::Type>());
}

void UUINavSlider::HandleOnSpinBoxMouseCaptureBegin()
{
	bMovingSpinBox = true;
}

void UUINavSlider::HandleOnSpinBoxValueChanged(const float InValue)
{
	const int32 NewOptionIndex = IndexFromValue(InValue);
	if (!bMovingSlider)
	{
		Slider->SetValue(static_cast<float>(NewOptionIndex) / static_cast<float>(GetMaxOptionIndex()));
	}
	UpdateTextFromPercent(Slider->GetValue(), /*bUpdateSpinBox*/ false);
}

void UUINavSlider::HandleOnSpinBoxValueCommitted(float InValue, ETextCommit::Type CommitMethod)
{
	bMovingSpinBox = false;

	if (CommitMethod == ETextCommit::Type::OnCleared)
	{
		return;
	}

	OptionIndex = IndexFromValue(InValue);

	if (Update())
	{
		Super::NavigateRight();

		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			Update();
		});
	}
}

void UUINavSlider::SetMinValue(const float NewValue, const bool bNotifyUpdate /*= true*/)
{
	MinValue = NewValue;
	if (IsValid(NavSpinBox))
	{
		NavSpinBox->SetMinSliderValue(MinValue);
		NavSpinBox->SetMinValue(MinValue);
	}
	SanitizeValues();
	Update(bNotifyUpdate);
}

void UUINavSlider::SetMaxValue(const float NewValue, const bool bNotifyUpdate /*= true*/)
{
	MaxValue = NewValue;
	if (IsValid(NavSpinBox))
	{
		NavSpinBox->SetMaxSliderValue(MaxValue);
		NavSpinBox->SetMaxValue(MaxValue);
	}
	SanitizeValues();
	Update(bNotifyUpdate);
}

void UUINavSlider::SetInterval(const float NewInterval, const bool bNotifyUpdate /*= true*/)
{
	Interval = NewInterval;
	SanitizeValues();
	Update(bNotifyUpdate);
}

int UUINavSlider::IndexFromPercent(const float Value)
{
	float Div = Value / Slider->GetStepSize();
	const int MaxOptionIndex = GetMaxOptionIndex();
	if (Div > MaxOptionIndex) Div = MaxOptionIndex;

	const int FlatDiv = FMath::RoundToInt(Div);
	const float Decimal = Div - FlatDiv;
	return Decimal < 0.5 ? FlatDiv : (FlatDiv + 1 <= MaxOptionIndex ? FlatDiv + 1 : MaxOptionIndex);
}

int UUINavSlider::IndexFromValue(const float Value)
{
	return IndexFromPercent(Value <= MinValue ? 0.f : (Value >= MaxValue ? 1.f : (Value - MinValue) / Difference));
}

void UUINavSlider::UpdateTextFromValue(const float Value, const bool bUpdateSpinBox /*= true*/)
{
	const int32 NewOptionIndex = IndexFromValue(Value);
	const float NewValuePercent = static_cast<float>(NewOptionIndex) / static_cast<float>(GetMaxOptionIndex());
	UpdateTextFromPercent(NewValuePercent, bUpdateSpinBox);
}

void UUINavSlider::UpdateTextFromPercent(const float Percent, const bool bUpdateSpinBox /*= true*/)
{
	FNumberFormattingOptions FormatOptions = FNumberFormattingOptions();
	FormatOptions.MaximumFractionalDigits = MaxDecimalDigits;
	FormatOptions.MinimumFractionalDigits = MinDecimalDigits;

	const float NewValue = MinValue + Percent * Difference;
	FText NewValueText = FText::AsNumber(NewValue, &FormatOptions);
	if (!bUseComma) NewValueText = FText::FromString(NewValueText.ToString().Replace(TEXT(","), TEXT(".")));

	SetText(NewValueText);

	if (NavSpinBox != nullptr && bUpdateSpinBox)
	{
		NavSpinBox->SetValue(NewValue);
	}
}

void UUINavSlider::SanitizeValues()
{
	if (Interval == 0.0f)
	{
		Interval = 0.1f;
	}

	if (MaxValue <= MinValue)
	{
		MaxValue = MinValue + Interval;
	}

	Difference = MaxValue - MinValue;
	Slider->SetStepSize(Interval / Difference);
}
