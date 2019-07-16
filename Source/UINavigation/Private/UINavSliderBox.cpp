// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavSliderBox.h"
#include "Components/ProgressBar.h"
#include "Kismet/KismetMathLibrary.h"

void UUINavSliderBox::NativeConstruct()
{
	Super::BaseConstruct();

	//check(DefaultOptionIndex <= (MaxRange - MinRange) && "DefaultOptionIndex isn't valid");
	if (OptionIndex > (MaxRange - MinRange))
	{
		DISPLAYERROR(TEXT("Invalid OptionIndex"));
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

void UUINavSliderBox::Update()
{
	int Difference = (MaxRange - MinRange) / Interval;
	if (OptionIndex > (Difference))
	{
		OptionIndex = Difference;
	}
	
	if (NavText != nullptr)
		NavText->SetText(FText::FromString(FString::FromInt(MinRange + OptionIndex*Interval)));

	float Percent = UKismetMathLibrary::NormalizeToRange(MinRange + OptionIndex * Interval, MinRange, MaxRange);
	SliderBar->SetPercent(Percent);
}

void UUINavSliderBox::NavigateRight()
{
	if (MinRange + OptionIndex*Interval < MaxRange) OptionIndex++;
	else
	{
		if (bLoopOptions) OptionIndex = 0;
		else return;
	}

	Update();

	if (!bDisableButtons || bLoopOptions)
	{
		Super::NavigateRight();
		return;
	}

	CheckRightLimit();
	//Enable button if previously disabled
	if (!LeftButton->bIsEnabled) LeftButton->SetIsEnabled(true);

	Super::NavigateRight();
}

float UUINavSliderBox::GetSliderPercent() const
{
	return SliderBar->Percent;
}
