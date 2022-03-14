// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved


#include "UINavHorizontalComponent.h"
#include "Components/TextBlock.h"
#include "UINavWidget.h"

void UUINavHorizontalComponent::NavigateLeft()
{
	OnNavigateLeft();
	OnUpdated();
	ParentWidget->OnHorizCompNavigateLeft(NavButton->ButtonIndex);
	ParentWidget->OnHorizCompUpdated(NavButton->ButtonIndex);
}

void UUINavHorizontalComponent::NavigateRight()
{
	OnNavigateRight();
	OnUpdated();
	ParentWidget->OnHorizCompNavigateRight(NavButton->ButtonIndex);
	ParentWidget->OnHorizCompUpdated(NavButton->ButtonIndex);
}

void UUINavHorizontalComponent::Update()
{
	const int MaxOptionIndex = GetMaxOptionIndex();
	if (MaxOptionIndex < 1) return;
	
	if (OptionIndex > MaxOptionIndex) OptionIndex = MaxOptionIndex;
	else if (OptionIndex < 0) OptionIndex = 0;

	LastOptionIndex = OptionIndex;
}

void UUINavHorizontalComponent::UpdateTextToIndex(int NewIndex)
{
	OptionIndex = NewIndex;
	Update();
}

void UUINavHorizontalComponent::ChangeText(const FText NewText)
{
	if (NavText != nullptr) NavText->SetText(NewText);
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
