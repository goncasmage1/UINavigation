// Copyright (C) 2018 Gonçalo Marques - All Rights Reserved

#include "UINavButton.h"


UUINavButton::UUINavButton()
{
	IsFocusable = false;
	OnHovered.AddDynamic(this, &UUINavButton::OnHover);
	OnUnhovered.AddDynamic(this, &UUINavButton::OnUnhover);
	OnClicked.AddDynamic(this, &UUINavButton::OnClick);
	OnReleased.AddDynamic(this, &UUINavButton::OnRelease);
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



