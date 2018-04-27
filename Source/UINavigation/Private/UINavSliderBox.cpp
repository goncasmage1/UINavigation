// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavSliderBox.h"
#include "ProgressBar.h"
#include "Kismet/KismetMathLibrary.h"

void UUINavSliderBox::NativeConstruct()
{
	Super::BaseConstruct();

	//check(DefaultOptionIndex <= (MaxRange - MinRange) && "DefaultOptionIndex isn't valid");
	if (DefaultOptionIndex > (MaxRange - MinRange))
	{
		DISPLAYERROR(TEXT("DefaultOptionIndex isn't valid"));
	}

	LeftButton->OnClicked.AddDynamic(this, &UUINavSliderBox::NavigateLeft);
	RightButton->OnClicked.AddDynamic(this, &UUINavSliderBox::NavigateRight);
}

void UUINavSliderBox::CheckRightLimit()
{
	int Difference = (MaxRange - MinRange) / Interval;
	if (OptionIndex >= Difference)
	{
		RightButton->SetIsEnabled(false);
	}
}

void UUINavSliderBox::UpdateTextBlock()
{
	int Difference = (MaxRange - MinRange) / Interval;
	if (OptionIndex > (Difference))
	{
		OptionIndex = Difference;
	}

	NavText->SetText(FText::FromString(FString::FromInt(MinRange + OptionIndex*Interval)));

	float Percent = UKismetMathLibrary::InverseLerp(MinRange, MaxRange, MinRange + OptionIndex * Interval);
	SliderBar->SetPercent(Percent);
}

void UUINavSliderBox::NavigateRight()
{
	if (MinRange + OptionIndex*Interval < MaxRange)
	{
		OptionIndex++;
	}

	UpdateTextBlock();

	if (!bDisableButtons) return;

	CheckRightLimit();
	//Enable button if previously disabled
	if (!LeftButton->bIsEnabled) LeftButton->SetIsEnabled(true);
}
