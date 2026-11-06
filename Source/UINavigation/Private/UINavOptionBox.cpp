// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavOptionBox.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "UINavWidget.h"
#include "UINavMacros.h"
#include "UINavBlueprintFunctionLibrary.h"

void UUINavOptionBox::NativePreConstruct()
{
	Super::BaseConstruct();

	if (!IsDesignTime() && LeftButton != nullptr && RightButton != nullptr)
	{
		LeftButton->OnClicked.AddUniqueDynamic(this, &UUINavHorizontalComponent::NavigateLeft);
		RightButton->OnClicked.AddUniqueDynamic(this, &UUINavHorizontalComponent::NavigateRight);
	}
}

int UUINavOptionBox::GetMaxOptionIndex() const
{
	if (bUseNumberRange)
	{
		return (MaxRange - MinRange) / Interval;
	}
	else
	{
		return FMath::Max(StringOptions.Num() - 1, 0);
	}
}

bool UUINavOptionBox::Update(const bool bNotify /*= true*/)
{
	const bool bChangedIndex = Super::Update(false);

	const FText NewText = bUseNumberRange ?
		FText::FromString(FString::FromInt(MinRange + OptionIndex * Interval)) :
		StringOptions.IsValidIndex(OptionIndex) ? StringOptions[OptionIndex] : FText();

	SetText(NewText);

	if (bChangedIndex && bNotify)
	{
		NotifyUpdated();
	}

	return bChangedIndex;
}

