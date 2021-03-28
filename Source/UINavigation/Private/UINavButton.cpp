// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavButton.h"
#include "Data/Grid.h"

int FGrid::GetDimension() const
{
	switch (GridType)
	{
		case EGridType::Horizontal:
			return DimensionX;
			break;
		case EGridType::Vertical:
			return DimensionY;
			break;
		case EGridType::Grid2D:
			return NumGrid2DButtons;
			break;
	}
	return 0;
}

int FGrid::GetLastButtonIndex() const
{
	return FirstButton->ButtonIndex + GetDimension() - 1;
}

UUINavButton::UUINavButton()
{
	IsFocusable = false;
	OnHovered.AddDynamic(this, &UUINavButton::OnHover);
	OnUnhovered.AddDynamic(this, &UUINavButton::OnUnhover);
	OnClicked.AddDynamic(this, &UUINavButton::OnClick);
	OnPressed.AddDynamic(this, &UUINavButton::OnPress);
	OnReleased.AddDynamic(this, &UUINavButton::OnRelease);
}

bool UUINavButton::IsValid(const bool bIgnoreDisabledUINavButton) const
{
	return (Visibility != ESlateVisibility::Collapsed &&
			Visibility != ESlateVisibility::Hidden &&
			(!bIgnoreDisabledUINavButton || bIsEnabled));
}

void UUINavButton::OnHover()
{
	CustomHover.Broadcast(ButtonIndex);
}

void UUINavButton::OnUnhover()
{
	CustomUnhover.Broadcast(ButtonIndex);
}

void UUINavButton::OnClick()
{
	CustomClick.Broadcast(ButtonIndex);
}

void UUINavButton::OnPress()
{
	CustomPress.Broadcast(ButtonIndex);
}

void UUINavButton::OnRelease()
{
	CustomRelease.Broadcast(ButtonIndex);
}



