// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavComponentBox.h"
#include "UINavMacros.h"
#include "Components/TextBlock.h"

void UUINavComponentBox::NativeConstruct()
{
	BaseConstruct();

	if (!LeftButton->OnClicked.IsBound())
		LeftButton->OnClicked.AddDynamic(this, &UUINavComponentBox::NavigateLeft);	
	if (!RightButton->OnClicked.IsBound())
		RightButton->OnClicked.AddDynamic(this, &UUINavComponentBox::NavigateRight);
}

void UUINavComponentBox::BaseConstruct()
{
	Super::NativeConstruct();

	if (MinRange >= MaxRange) DISPLAYERROR(TEXT("MinRange has to be smaller that MaxRange"));
	if (Interval <= 0) DISPLAYERROR(TEXT("Interval must be at least 1"));

	if (LeftButton == nullptr) DISPLAYERROR(TEXT("Couldn't find Button named LeftButton in UINavOptionBox"));

	if (RightButton == nullptr) DISPLAYERROR(TEXT("Couldn't find Button named RightButton in UINavOptionBox"));

	if (NavText == nullptr) DISPLAYERROR(TEXT("Couldn't find TextBlock named NavText in UINavOptionBox"));

	Update();

	if (!bDisableButtons || bLoopOptions) return;

	CheckLeftLimit();
	CheckRightLimit();
}

void UUINavComponentBox::CheckLeftLimit()
{
	if (bLoopOptions) return;
	if (OptionIndex <= 0) LeftButton->SetIsEnabled(false);
}

void UUINavComponentBox::CheckRightLimit()
{
	if (bLoopOptions) return;
	if (OptionIndex >= GetMaxOptionIndex()) RightButton->SetIsEnabled(false);
}

void UUINavComponentBox::SetOptionIndex(const int NewIndex)
{
	Super::SetOptionIndex(NewIndex);

	if (!bDisableButtons || bLoopOptions) return;
	if (!RightButton->GetIsEnabled()) RightButton->SetIsEnabled(true);
	if (!LeftButton->GetIsEnabled()) LeftButton->SetIsEnabled(true);
	CheckLeftLimit();
	CheckRightLimit();
}

void UUINavComponentBox::NavigateLeft()
{
	//Make sure button still has options left to navigate
	if (OptionIndex > 0) OptionIndex--;
	else if (bLoopOptions) OptionIndex = GetMaxOptionIndex();
	else return;

	if (Update())
	{
		NotifyNavigateLeft();
	}

	if (bLoopOptions || !bDisableButtons)
	{
		return;
	}

	CheckLeftLimit();
	//Enable button if previously disabled
	if (!RightButton->GetIsEnabled() && GetMaxOptionIndex() > 0) RightButton->SetIsEnabled(true);
}

void UUINavComponentBox::NavigateRight()
{
	if ((OptionIndex + 1) <= GetMaxOptionIndex()) OptionIndex++;
	else
	{
		if (bLoopOptions) OptionIndex = 0;
		else return;
	}

	if (Update())
	{
		NotifyNavigateRight();
	}

	if (bLoopOptions || !bDisableButtons)
	{
		return;
	}

	CheckRightLimit();
	//Enable button if previously disabled
	if (!LeftButton->GetIsEnabled() && GetMaxOptionIndex() > 0) LeftButton->SetIsEnabled(true);
}
