// Copyright (C) 2018 Gonçalo Marques - All Rights Reserved

#include "UINavComponentBox.h"
#include "UINavButton.h"
#include "Components/TextBlock.h"

void UUINavComponentBox::NativeConstruct()
{
	BaseConstruct();

	//check(DefaultOptionIndex <= (MaxRange - MinRange) && "DefaultOptionIndex isn't valid");
	if (DefaultOptionIndex > (MaxRange - MinRange))
	{
		DISPLAYERROR(TEXT("DefaultOptionIndex isn't valid"));
	}

	LeftButton->OnClicked.AddDynamic(this, &UUINavComponentBox::NavigateLeft);
	RightButton->OnClicked.AddDynamic(this, &UUINavComponentBox::NavigateRight);
}

void UUINavComponentBox::BaseConstruct()
{
	Super::NativeConstruct();

	//check( && "MinRange has to be smaller that MaxRange");
	if (MinRange >= MaxRange)
	{
		DISPLAYERROR(TEXT("MinRange has to be smaller that MaxRange"));
	}
	//check(Interval > 0 && "Interval must be at least 1");
	if (Interval <= 0)
	{
		DISPLAYERROR(TEXT("Interval must be at least 1"));
	}

	//check(LeftButton != nullptr && "Couldn't find Button named LeftButton in UINavOptionBox");
	if (LeftButton == nullptr)
	{
		DISPLAYERROR(TEXT("Couldn't find Button named LeftButton in UINavOptionBox"));
	}
	//check(RightButton != nullptr && "Couldn't find Button named RightButton in UINavOptionBox");
	if (RightButton == nullptr)
	{
		DISPLAYERROR(TEXT("Couldn't find Button named RightButton in UINavOptionBox"));
	}
	//check(NavText != nullptr && "Couldn't find TextBlock named NavText in UINavOptionBox");
	if (NavText == nullptr)
	{
		DISPLAYERROR(TEXT("Couldn't find TextBlock named NavText in UINavOptionBox"));
	}

	if (OptionIndex == 0) OptionIndex = DefaultOptionIndex;
	UpdateTextBlock();
	CheckLeftLimit();
	CheckRightLimit();
}

void UUINavComponentBox::CheckLeftLimit()
{
	if (OptionIndex == 0)
	{
		LeftButton->SetIsEnabled(false);
	}
}

void UUINavComponentBox::CheckRightLimit()
{

}

void UUINavComponentBox::UpdateTextBlock()
{
}

void UUINavComponentBox::UpdateTextToIndex(int NewIndex)
{
	OptionIndex = NewIndex;
	UpdateTextBlock();

	if (!bDisableButtons) return;
	if (!RightButton->bIsEnabled) RightButton->SetIsEnabled(true);
	if (!LeftButton->bIsEnabled) LeftButton->SetIsEnabled(true);
	CheckLeftLimit();
	CheckRightLimit();
}

void UUINavComponentBox::NavigateLeft()
{
	//Make sure button still has options left to navigate
	if (OptionIndex > 0)
	{
		OptionIndex--;
	}

	UpdateTextBlock();
	OnNavigateLeft();

	if (!bDisableButtons) return;

	CheckLeftLimit();
	//Enable button if previously disabled
	if (!RightButton->bIsEnabled) RightButton->SetIsEnabled(true);
}

void UUINavComponentBox::NavigateRight()
{
	OnNavigateRight();
}

void UUINavComponentBox::OnNavigateLeft_Implementation()
{
}

void UUINavComponentBox::OnNavigateRight_Implementation()
{
}
