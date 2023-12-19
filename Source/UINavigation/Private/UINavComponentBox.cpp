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

	if (NavRichText == nullptr) DISPLAYERROR(TEXT("Couldn't find RichTextBlock named NavRichText in UINavOptionBox"));

	Update();

	if (!bDisableButtons || bLoopOptions) return;

	CheckLeftLimit();
	CheckRightLimit();
}

void UUINavComponentBox::CheckLeftLimit()
{
	if (bLoopOptions) return;

	LeftButton->SetIsEnabled(OptionIndex > 0);
}

void UUINavComponentBox::CheckRightLimit()
{
	if (bLoopOptions) return;

	RightButton->SetIsEnabled(OptionIndex < GetMaxOptionIndex());
}

bool UUINavComponentBox::SetOptionIndex(const int NewIndex)
{
	const bool bShouldNotify = Super::SetOptionIndex(NewIndex);

	if (!bLoopOptions && bDisableButtons)
	{
		CheckLeftLimit();
		CheckRightLimit();
	}

	if (bShouldNotify)
	{
		NotifyUpdated();
	}

	return bShouldNotify;
}

void UUINavComponentBox::NavigateLeft()
{
	//Make sure button still has options left to navigate
	if (OptionIndex > 0) OptionIndex--;
	else if (bLoopOptions) OptionIndex = GetMaxOptionIndex();
	else return;

	const bool bShouldNotify = Update(false);

	if (!bLoopOptions && bDisableButtons)
	{
		CheckLeftLimit();
		CheckRightLimit();
	}

	if (bShouldNotify)
	{
		NotifyUpdated();
		Super::NavigateLeft();
	}
}

void UUINavComponentBox::NavigateRight()
{
	if ((OptionIndex + 1) <= GetMaxOptionIndex()) OptionIndex++;
	else
	{
		if (bLoopOptions) OptionIndex = 0;
		else return;
	}

	const bool bShouldNotify = Update(false);

	if (!bLoopOptions && bDisableButtons)
	{
		CheckLeftLimit();
		CheckRightLimit();
	}

	if (bShouldNotify)
	{
		NotifyUpdated();
		Super::NavigateRight();
	}
}