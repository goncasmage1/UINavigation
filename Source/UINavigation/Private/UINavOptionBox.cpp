// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavOptionBox.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UINavMacros.h"

void UUINavOptionBox::NativeConstruct()
{
	Super::BaseConstruct();

	if (!LeftButton->OnClicked.IsBound())
		LeftButton->OnClicked.AddDynamic(this, &UUINavOptionBox::NavigateLeft);
	
	if (!RightButton->OnClicked.IsBound())
		RightButton->OnClicked.AddDynamic(this, &UUINavOptionBox::NavigateRight);
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

void UUINavOptionBox::Update()
{
	Super::Update();

	NavText->SetText(bUseNumberRange ? 
		FText::FromString(FString::FromInt(MinRange + OptionIndex*Interval)) : 
		StringOptions.IsValidIndex(OptionIndex) ? StringOptions[OptionIndex] : FText());
}

