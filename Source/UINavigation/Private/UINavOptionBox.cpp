// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavOptionBox.h"
#include "UINavButton.h"
#include "Button.h"
#include "TextBlock.h"
#include "WidgetTree.h"


void UUINavOptionBox::NativeConstruct()
{
	Super::NativeConstruct();

	if (bUseNumberRange)
	{
		check(MinRange < MaxRange && "UINavSlider: MinRange has to be smaller that MaxRange");
	}
	else
	{
		check(StringOptions.Num() > 1 && "UINavSlider: StringOptions needs to have at least 2 options");
	}

	check(LeftButton != nullptr && "Couldn't find Button named LeftButton in UINavSlider");
	check(RightButton != nullptr && "Couldn't find Button named RightButton in UINavSlider");
	check(NavText != nullptr && "Couldn't find TextBlock named NavText in UINavSlider");
	if (bUseNumberRange)
	{
		check(DefaultOptionIndex <= (MaxRange - MinRange) && "DefaultOptionIndex not valid");
	}
	else
	{
		check(DefaultOptionIndex <= (StringOptions.Num() - 1) && "DefaultOptionIndex not valid");
	}

	FColor TransparentColor = FColor::White;
	TransparentColor.A = 0;

	NavButton->WidgetStyle.Normal.TintColor = FSlateColor(TransparentColor);
	NavButton->WidgetStyle.Hovered.TintColor = FSlateColor(TransparentColor);
	NavButton->WidgetStyle.Pressed.TintColor = FSlateColor(TransparentColor);
	NavButton->WidgetStyle.Disabled.TintColor = FSlateColor(TransparentColor);
	NavButton->WidgetStyle.NormalPadding = 0;
	NavButton->WidgetStyle.PressedPadding = 0;
	NavButton->SetStyle(NavButton->WidgetStyle);

	LeftButton->OnClicked.AddDynamic(this, &UUINavOptionBox::NavigateLeft);
	RightButton->OnClicked.AddDynamic(this, &UUINavOptionBox::NavigateRight);

	if (OptionIndex == 0) OptionIndex = DefaultOptionIndex;
	UpdateTextBlock();
	CheckLeftLimit();
	CheckRightLimit();
}

void UUINavOptionBox::NavigateLeft()
{
	//Make sure button still has options left to navigate
	if (OptionIndex > 0)
	{
		OptionIndex--;
	}

	UpdateTextBlock();

	if (!bDisableButtons) return;

	CheckLeftLimit();
	//Enable button if previously disabled
	if (!RightButton->bIsEnabled) RightButton->SetIsEnabled(true);
}

void UUINavOptionBox::NavigateRight()
{
	//Make sure button still has options left to navigate
	if (bUseNumberRange)
	{
		if (MinRange + OptionIndex < MaxRange)
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

void UUINavOptionBox::CheckLeftLimit()
{
	if (OptionIndex == 0)
	{
		LeftButton->SetIsEnabled(false);
	}
}

void UUINavOptionBox::CheckRightLimit()
{
	if (bUseNumberRange)
	{
		if (OptionIndex == MaxRange - MinRange)
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
		if (OptionIndex > (MaxRange - MinRange))
		{
			OptionIndex = MaxRange - MinRange;
		}
	}
	else
	{
		if (OptionIndex > (StringOptions.Num() - 1))
		{
			OptionIndex = StringOptions.Num() - 1;
		}
	}

	NavText->SetText(bUseNumberRange ? FText::FromString(FString::FromInt(MinRange + OptionIndex)) : FText::FromName(StringOptions[OptionIndex]));
}

void UUINavOptionBox::UpdateTextWithValue(int NewIndex)
{
	OptionIndex = NewIndex;
	UpdateTextBlock();

	if (!bDisableButtons) return;
	if (!RightButton->bIsEnabled) RightButton->SetIsEnabled(true);
	if (!LeftButton->bIsEnabled) LeftButton->SetIsEnabled(true);
	CheckLeftLimit();
	CheckRightLimit();
}

