// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved


#include "UINavHorizontalComponent.h"
#include "Components/TextBlock.h"
#include "UINavWidget.h"

void UUINavHorizontalComponent::NavigateLeft()
{
	OnNavigateLeft();
	OnUpdated();
	ParentWidget->OnHorizCompNavigateLeft(ComponentIndex);
	ParentWidget->OnHorizCompUpdated(ComponentIndex);
}

void UUINavHorizontalComponent::NavigateRight()
{
	OnNavigateRight();
	OnUpdated();
	ParentWidget->OnHorizCompNavigateRight(ComponentIndex);
	ParentWidget->OnHorizCompUpdated(ComponentIndex);
}

void UUINavHorizontalComponent::Update()
{
	int MaxOptionIndex = GetMaxOptionIndex();
	if (OptionIndex > MaxOptionIndex) OptionIndex = MaxOptionIndex;
	else if (OptionIndex < 0) OptionIndex = 0;

	LastOptionIndex = OptionIndex;
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

void UUINavHorizontalComponent::OnUpdated_Implementation()
{
}
