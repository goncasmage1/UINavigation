// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavSliderBox.h"
#include "UINavWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Kismet/KismetMathLibrary.h"

void UUINavSliderBox::NativeConstruct()
{
	Super::BaseConstruct();

	if (!LeftButton->OnClicked.IsBound())
		LeftButton->OnClicked.AddDynamic(this, &UUINavSliderBox::NavigateLeft);
	if (!RightButton->OnClicked.IsBound())
		RightButton->OnClicked.AddDynamic(this, &UUINavSliderBox::NavigateRight);
}

bool UUINavSliderBox::Update(const bool bNotify /*= true*/)
{
	const bool bChangedIndex = Super::Update(bNotify);

	if (NavText != nullptr)
		NavText->SetText(FText::FromString(FString::FromInt(MinRange + OptionIndex * Interval)));

	if (NavRichText != nullptr)
		NavRichText->SetText(FText::FromString(FString::FromInt(MinRange + OptionIndex * Interval)));

	const float Percent = UKismetMathLibrary::NormalizeToRange(MinRange + OptionIndex * Interval, MinRange, MaxRange);
	SliderBar->SetPercent(Percent);

	return bChangedIndex;
}

float UUINavSliderBox::GetSliderPercent() const
{
	return SliderBar->GetPercent();
}