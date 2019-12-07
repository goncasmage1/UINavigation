// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavOptionBox.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UUINavOptionBox::NativeConstruct()
{
	Super::BaseConstruct();

	if (bUseNumberRange)
	{
		if (OptionIndex > (MaxRange - MinRange))
		{
			DISPLAYERROR(TEXT("Invalid OptionIndex"));
		}
	}
	else
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

	if (!LeftButton->OnClicked.IsBound())
		LeftButton->OnClicked.AddDynamic(this, &UUINavOptionBox::NavigateLeft);
	if (!RightButton->OnClicked.IsBound())
		RightButton->OnClicked.AddDynamic(this, &UUINavOptionBox::NavigateRight);
}


int UUINavOptionBox::GetLastOptionIndex()
{
	if (bUseNumberRange) return Super::GetLastOptionIndex();
	else return StringOptions.Num() -1;
}

void UUINavOptionBox::NavigateRight()
{
	//Make sure button still has options left to navigate
	if (bUseNumberRange)
	{
		if (MinRange + OptionIndex*Interval < MaxRange) OptionIndex++;
		else
		{
			if (bLoopOptions) OptionIndex = 0;
			else return;
		}
	}
	else
	{
		if (OptionIndex < StringOptions.Num() - 1) OptionIndex++;
		else
		{
			if (bLoopOptions) OptionIndex = 0;
			else return;
		}
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

void UUINavOptionBox::CheckRightLimit()
{
	if (bLoopOptions) return;
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

void UUINavOptionBox::Update()
{
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
		if (StringOptions.Num() == 0) return;

		if (OptionIndex < 0) OptionIndex = 0;
		if (OptionIndex >= StringOptions.Num()) OptionIndex = StringOptions.Num() - 1;
	}

	NavText->SetText(bUseNumberRange ? 
		FText::FromString(FString::FromInt(MinRange + OptionIndex*Interval)) : 
		FText::FromName(StringOptions[OptionIndex]));

	Super::Update();
}

