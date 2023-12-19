// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavOptionBox.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "UINavMacros.h"

void UUINavOptionBox::NativeConstruct()
{
	Super::BaseConstruct();

	if (!LeftButton->OnClicked.IsBound())
		LeftButton->OnClicked.AddDynamic(this, &UUINavHorizontalComponent::NavigateLeft);

	if (!RightButton->OnClicked.IsBound())
		RightButton->OnClicked.AddDynamic(this, &UUINavHorizontalComponent::NavigateRight);
}

int UUINavOptionBox::GetMaxOptionIndex() const
{
	if (bUseNumberRange)
	{
		return (MaxRange - MinRange) / Interval;
	}
	else
	{
		return StringOptions.Num() - 1;
	}
}

bool UUINavOptionBox::Update(const bool bNotify /*= true*/)
{
	const bool bChangedIndex = Super::Update(bNotify);

	NavText->SetText(bUseNumberRange ?
		FText::FromString(FString::FromInt(MinRange + OptionIndex*Interval)) :
		StringOptions.IsValidIndex(OptionIndex) ? StringOptions[OptionIndex] : FText());

	NavRichText->SetText(bUseNumberRange ?
		FText::FromString(FString::FromInt(MinRange + OptionIndex * Interval)) :
		StringOptions.IsValidIndex(OptionIndex) ? StringOptions[OptionIndex] : FText());

	return bChangedIndex;
}