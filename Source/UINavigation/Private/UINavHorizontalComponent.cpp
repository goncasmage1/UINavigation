// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved


#include "UINavHorizontalComponent.h"
#include "Components/TextBlock.h"
#include "UINavWidget.h"

void UUINavHorizontalComponent::NavigateLeft()
{
	OnNavigateLeft();
	ParentWidget->OnHorizCompUpdated(ComponentIndex);
}

void UUINavHorizontalComponent::NavigateRight()
{
	OnNavigateRight();
	ParentWidget->OnHorizCompUpdated(ComponentIndex);
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
