// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved


#include "UINavHorizontalComponent.h"
#include "Components/TextBlock.h"
#include "UINavWidget.h"

FNavigationReply UUINavHorizontalComponent::NativeOnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply)
{
	FNavigationReply Reply = Super::NativeOnNavigation(MyGeometry, InNavigationEvent, InDefaultReply);

	if (InNavigationEvent.GetNavigationType() == EUINavigation::Left)
	{
		NavigateLeft();
		return FNavigationReply::Stop();
	}

	if (InNavigationEvent.GetNavigationType() == EUINavigation::Right)
	{
		NavigateRight();
		return FNavigationReply::Stop();
	}

	return Reply;
}

void UUINavHorizontalComponent::NavigateLeft()
{
	OnNavigateLeft();
	OnUpdated();
	OnValueChanged.Broadcast();
	OnNativeValueChanged.Broadcast();
	ParentWidget->PropagateOnHorizCompNavigateLeft(this);
	ParentWidget->PropagateOnHorizCompUpdated(this);
}

void UUINavHorizontalComponent::NavigateRight()
{
	OnNavigateRight();
	OnUpdated();
	OnValueChanged.Broadcast();
	OnNativeValueChanged.Broadcast();
	ParentWidget->PropagateOnHorizCompNavigateRight(this);
	ParentWidget->PropagateOnHorizCompUpdated(this);
}

void UUINavHorizontalComponent::Update()
{
	const int MaxOptionIndex = GetMaxOptionIndex();
	if (MaxOptionIndex < 1) return;
	
	if (OptionIndex > MaxOptionIndex) OptionIndex = MaxOptionIndex;
	else if (OptionIndex < 0) OptionIndex = 0;

	LastOptionIndex = OptionIndex;
}

void UUINavHorizontalComponent::SetOptionIndex(int NewIndex)
{
	OptionIndex = NewIndex;
	Update();
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
