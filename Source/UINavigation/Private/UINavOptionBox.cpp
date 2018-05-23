// Copyright (C) 2018 Gonçalo Marques - All Rights Reserved

#include "UINavOptionBox.h"
#include "TextBlock.h"
#include "WidgetTree.h"


void UUINavOptionBox::NativeConstruct()
{
	Super::BaseConstruct();

	if (!bUseNumberRange)
	{
		//check(StringOptions.Num() > 1 && "StringOptions needs to have at least 2 options");
		if (StringOptions.Num() <= 1)
		{
			DISPLAYERROR(TEXT("StringOptions needs to have at least 2 options"));
		}
		//check(DefaultOptionIndex <= (StringOptions.Num() - 1) && "DefaultOptionIndex isn't valid");
		if (DefaultOptionIndex > (StringOptions.Num() - 1))
		{
			DISPLAYERROR(TEXT("DefaultOptionIndex isn't valid"));
		}
	}
	else
	{
		//check(DefaultOptionIndex <= (MaxRange - MinRange) && "DefaultOptionIndex isn't valid");
		if (DefaultOptionIndex > (MaxRange - MinRange))
		{
			DISPLAYERROR(TEXT("DefaultOptionIndex isn't valid"));
		}
	}

	LeftButton->OnClicked.AddDynamic(this, &UUINavOptionBox::NavigateLeft);
	RightButton->OnClicked.AddDynamic(this, &UUINavOptionBox::NavigateRight);
}

void UUINavOptionBox::NavigateRight()
{
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

	NavText->SetText(bUseNumberRange ? 
		FText::FromString(FString::FromInt(MinRange + OptionIndex*Interval)) : 
		FText::FromName(StringOptions[OptionIndex]));
}

void UUINavOptionBox::SetOptionText(FText NewText)
{
	NavText->SetText(NewText);
}


