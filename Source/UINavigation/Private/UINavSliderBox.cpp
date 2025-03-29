// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavSliderBox.h"
#include "UINavWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Kismet/KismetMathLibrary.h"
#include "UINavBlueprintFunctionLibrary.h"

void UUINavSliderBox::NativePreConstruct()
{
	Super::BaseConstruct();

	if (!IsDesignTime())
	{
		if (!LeftButton->OnClicked.IsBound())
			LeftButton->OnClicked.AddDynamic(this, &UUINavSliderBox::NavigateLeft);
		if (!RightButton->OnClicked.IsBound())
			RightButton->OnClicked.AddDynamic(this, &UUINavSliderBox::NavigateRight);
	}
}

bool UUINavSliderBox::Update(const bool bNotify /*= true*/)
{
	const bool bChangedIndex = Super::Update(false);
	
	const FText NewText = FText::FromString(FString::FromInt(MinRange + OptionIndex * Interval));
	SetText(NewText);

	if (IsValid(SliderBar))
	{
		const float Percent = UKismetMathLibrary::NormalizeToRange(MinRange + OptionIndex * Interval, MinRange, MaxRange);
		SliderBar->SetPercent(Percent);
	}

	if (bChangedIndex && bNotify)
	{
		NotifyUpdated();
	}

	return bChangedIndex;
}

float UUINavSliderBox::GetSliderPercent() const
{
	return SliderBar->GetPercent();
}
