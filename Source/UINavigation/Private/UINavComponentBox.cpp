// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavComponentBox.h"
#include "UINavMacros.h"
#include "UINavButton.h"
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
	else LeftButton->IsFocusable = false;

	if (RightButton == nullptr) DISPLAYERROR(TEXT("Couldn't find Button named RightButton in UINavOptionBox"));
	else RightButton->IsFocusable = false;

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

void UUINavComponentBox::UpdateTextToIndex(const int NewIndex)
{
	Super::UpdateTextToIndex(NewIndex);

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

	FinishNavigateLeft(LastOptionIndex != OptionIndex);
}

void UUINavComponentBox::NavigateRight()
{
	if ((OptionIndex + 1) <= GetMaxOptionIndex()) OptionIndex++;
	else
	{
		if (bLoopOptions) OptionIndex = 0;
		else return;
	}

	FinishNavigateRight(LastOptionIndex != OptionIndex);
}

void UUINavComponentBox::FinishNavigateLeft(const bool bOptionChanged)
{
	Update();

	if (bLoopOptions)
	{
		Super::NavigateLeft();
		return;
	}
	else if (!bDisableButtons)
	{
		if (bOptionChanged)
		{
			Super::NavigateLeft();
		}
		return;
	}

	CheckLeftLimit();
	//Enable button if previously disabled
	if (!RightButton->GetIsEnabled() && GetMaxOptionIndex() > 0) RightButton->SetIsEnabled(true);

	if (bOptionChanged)
	{
		Super::NavigateLeft();
	}
}

void UUINavComponentBox::FinishNavigateRight(const bool bOptionChanged)
{
	Update();

	if (bLoopOptions)
	{
		Super::NavigateRight();
		return;
	}
	else if (!bDisableButtons)
	{
		if (bOptionChanged)
		{
			Super::NavigateRight();
		}
		return;
	}

	CheckRightLimit();
	//Enable button if previously disabled
	if (!LeftButton->GetIsEnabled() && GetMaxOptionIndex() > 0) LeftButton->SetIsEnabled(true);

	if (bOptionChanged)
	{
		Super::NavigateRight();
	}
}
