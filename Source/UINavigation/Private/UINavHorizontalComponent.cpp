// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved


#include "UINavHorizontalComponent.h"
#include "Components/TextBlock.h"

void UUINavHorizontalComponent::NavigateLeft()
{
	OnNavigateLeft();
}

void UUINavHorizontalComponent::NavigateRight()
{
	OnNavigateRight();
}

void UUINavHorizontalComponent::Update()
{
}

void UUINavHorizontalComponent::UpdateTextToIndex(int NewIndex)
{
	OptionIndex = NewIndex;
	Update();
}

void UUINavHorizontalComponent::ChangeText(FText NewText)
{
	NavText->SetText(NewText);
}

void UUINavHorizontalComponent::OnNavigateLeft_Implementation()
{
}

void UUINavHorizontalComponent::OnNavigateRight_Implementation()
{
}
