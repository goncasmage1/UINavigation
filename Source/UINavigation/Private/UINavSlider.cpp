// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavSlider.h"
#include "Slider.h"

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

	OptionCount = (MaxValue - MinValue) / Interval;
}

void UUINavSlider::Update()
{
	Slider->SetValue((float)OptionIndex / (float)(OptionCount - 1));
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
	if (OptionIndex < (OptionCount - 1))
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
	//UE_LOG(LogTemp, Warning, TEXT("%i"), OptionIndex);
	Update();
}

float UUINavSlider::IndexFromPercent(float Value)
{
	float Div = Value / Slider->StepSize;
	float Decimal = Div - (int)Div;
	return Decimal < 0.5 ? (int)Div : (int)Div + 1;
}
