// Copyright (C) 2018 Gonçalo Marques - All Rights Reserved

#include "UINavButton.h"


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
			return DimensionX * DimensionY;
			break;
	}
	return 0;
}

UUINavButton::UUINavButton()
{
	IsFocusable = false;
	OnHovered.AddDynamic(this, &UUINavButton::OnHover);
	OnUnhovered.AddDynamic(this, &UUINavButton::OnUnhover);
	OnClicked.AddDynamic(this, &UUINavButton::OnClick);
	OnReleased.AddDynamic(this, &UUINavButton::OnRelease);
}


bool UUINavButton::IsValid()
{
	return (Visibility != ESlateVisibility::Collapsed &&
			Visibility != ESlateVisibility::Hidden &&
			bIsEnabled);
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

void UUINavButton::OnRelease()
{
	CustomRelease.Broadcast(ButtonIndex);
}



