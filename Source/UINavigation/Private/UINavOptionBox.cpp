// Copyright (C) 2018 Gonçalo Marques - All Rights Reserved

#include "UINavOptionBox.h"
#include "Blueprint/WidgetTree.h"


void UUINavOptionBox::NativeConstruct()
{
	Super::BaseConstruct();

	if (!bUseNumberRange)
	{
		if (StringOptions.Num() <= 1)
		{
			DISPLAYERROR(TEXT("StringOptions needs to have at least 2 options"));
		}
		if (OptionIndex > (StringOptions.Num() - 1))
		{
			DISPLAYERROR(TEXT("Invalid OptionIndex"));
		}
	}
	else
	{
		if (OptionIndex > (MaxRange - MinRange))
		{
			DISPLAYERROR(TEXT("Invalid OptionIndex"));
		}
	}

	LeftButton->OnClicked.AddDynamic(this, &UUINavOptionBox::NavigateLeft);
	RightButton->OnClicked.AddDynamic(this, &UUINavOptionBox::NavigateRight);
}

void UUINavOptionBox::NavigateRight()
{
	Super::NavigateRight();

	//Make sure button still has options left to navigate
	if (bUseNumberRange)
	{
		if (MinRange + OptionIndex*Interval < MaxRange)
		{
			OptionIndex++;
		}
	}
	else
	{
		if (OptionIndex < StringOptions.Num() - 1)
		{
			OptionIndex++;
		}
	}

	UpdateTextBlock();

	if (!bDisableButtons) return;

	CheckRightLimit();
	//Enable button if previously disabled
	if (!LeftButton->bIsEnabled) LeftButton->SetIsEnabled(true);
}

void UUINavOptionBox::CheckRightLimit()
{
	if (bUseNumberRange)
	{
		int Difference = (MaxRange - MinRange) / Interval;
		if (OptionIndex >= Difference)
		{
			RightButton->SetIsEnabled(false);
		}
	}
	else
	{
		if (OptionIndex == StringOptions.Num() - 1)
		{
			RightButton->SetIsEnabled(false);
		}
	}
}

void UUINavOptionBox::UpdateTextBlock()
{
	//Correct OptionIndex to fit appropriate range
	if (bUseNumberRange)
	{
		int Difference = (MaxRange - MinRange) / Interval;
		if (OptionIndex > (Difference))
		{
			OptionIndex = Difference;
		}
	}
	else
	{
		if (OptionIndex > (StringOptions.Num() - 1))
		{
			OptionIndex = StringOptions.Num() - 1;
		}
	}

	if (OptionIndex >= StringOptions.Num()) return;

	NavText->SetText(bUseNumberRange ? 
		FText::FromString(FString::FromInt(MinRange + OptionIndex*Interval)) : 
		FText::FromName(StringOptions[OptionIndex]));
}

