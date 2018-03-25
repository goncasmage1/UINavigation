// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavCustomSlider.h"


UUINavCustomSlider::UUINavCustomSlider()
{
	OnMouseCaptureBegin.AddDynamic(this, &UUINavCustomSlider::OnCaptureBegin);
	OnValueChanged.AddDynamic(this, &UUINavCustomSlider::OnValueUpdate);
}

void UUINavCustomSlider::OnCaptureBegin()
{
	AttemptedValue = DefaultValue;
	SnapToValue();
}

void UUINavCustomSlider::OnValueUpdate(float NewValue)
{
	AttemptedValue = NewValue;
	SnapToValue();
}

void UUINavCustomSlider::SnapToValue()
{
	int Count = 0;
	float Division = 1.f / (Steps);
	float NewValue = 0.f;
	NewValue += Division;

	while (NewValue < 1.f)
	{
		if (AttemptedValue < NewValue)
		{
			break;
		}
		NewValue += Division;
		Count++;
	}

	StepIndex = Count;
	SetValue(1.f / (Steps - 1)*StepIndex);
}

void UUINavCustomSlider::IncreaseStepIndex()
{
	if (StepIndex < Steps)
	{
		StepIndex++;
		SetValue(1.f / (Steps - 1)*StepIndex);
	}
}

void UUINavCustomSlider::DecreaseStepIndex()
{
	if (StepIndex > 0)
	{
		StepIndex--;
		SetValue(1.f / (Steps - 1)*StepIndex);
	}
}
