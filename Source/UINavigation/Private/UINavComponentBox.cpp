// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavComponentBox.h"
#include "UINavButton.h"
#include "Components/TextBlock.h"

void UUINavComponentBox::NativeConstruct()
{
	BaseConstruct();

	if (OptionIndex > (MaxRange - MinRange))
	{
		DISPLAYERROR(TEXT("Invalid OptionIndex"));
	}

	LeftButton->OnClicked.AddDynamic(this, &UUINavComponentBox::NavigateLeft);
	RightButton->OnClicked.AddDynamic(this, &UUINavComponentBox::NavigateRight);
	LeftButton->IsFocusable = false;
	RightButton->IsFocusable = false;
}

void UUINavComponentBox::BaseConstruct()
{
	Super::NativeConstruct();

	if (MinRange >= MaxRange)
	{
		DISPLAYERROR(TEXT("MinRange has to be smaller that MaxRange"));
	}
	if (Interval <= 0)
	{
		DISPLAYERROR(TEXT("Interval must be at least 1"));
	}

	if (LeftButton == nullptr)
	{
		DISPLAYERROR(TEXT("Couldn't find Button named LeftButton in UINavOptionBox"));
	}
	if (RightButton == nullptr)
	{
		DISPLAYERROR(TEXT("Couldn't find Button named RightButton in UINavOptionBox"));
	}
	if (NavText == nullptr)
	{
		DISPLAYERROR(TEXT("Couldn't find TextBlock named NavText in UINavOptionBox"));
	}

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

void UUINavComponentBox::ChangeText(FText NewText)
{
	NavText->SetText(NewText);
}

void UUINavComponentBox::NavigateLeft()
{
	//Make sure button still has options left to navigate
	if (OptionIndex > 0)
	{
		OptionIndex--;
	}

	UpdateTextBlock();
	

	if (!bDisableButtons)
	{
		OnNavigateLeft();
		return;
	}

	CheckLeftLimit();
	//Enable button if previously disabled
	if (!RightButton->bIsEnabled) RightButton->SetIsEnabled(true);

	OnNavigateLeft();
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
