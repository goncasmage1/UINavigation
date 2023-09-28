// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved


#include "UINavHorizontalComponent.h"
#include "Components/TextBlock.h"
#include "UINavWidget.h"

FNavigationReply UUINavHorizontalComponent::NativeOnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply)
{
	FNavigationReply Reply = Super::NativeOnNavigation(MyGeometry, InNavigationEvent, InDefaultReply);

	if (InNavigationEvent.GetNavigationType() == EUINavigation::Left && Reply.GetBoundaryRule() != EUINavigationRule::Stop)
	{
		NavigateLeft();
		return FNavigationReply::Stop();
	}

	if (InNavigationEvent.GetNavigationType() == EUINavigation::Right && Reply.GetBoundaryRule() != EUINavigationRule::Stop)
	{
		NavigateRight();
		return FNavigationReply::Stop();
	}

	return Reply;
}

void UUINavHorizontalComponent::NavigateLeft()
{
	NotifyNavigateLeft();
}

void UUINavHorizontalComponent::NavigateRight()
{
	NotifyNavigateRight();
}

void UUINavHorizontalComponent::NotifyNavigateLeft()
{
	OnNavigateLeft();
	if (IsValid(ParentWidget))
	{
		ParentWidget->PropagateOnHorizCompNavigateLeft(this);
	}
}

void UUINavHorizontalComponent::NotifyNavigateRight()
{
	OnNavigateRight();
	if (IsValid(ParentWidget))
	{
		ParentWidget->PropagateOnHorizCompNavigateRight(this);
	}
}

bool UUINavHorizontalComponent::Update(const bool bNotify /*= true*/)
{
	const int MaxOptionIndex = GetMaxOptionIndex();
	if (MaxOptionIndex < 1) return false;
	
	if (OptionIndex > MaxOptionIndex) OptionIndex = MaxOptionIndex;
	else if (OptionIndex < 0) OptionIndex = 0;

	const bool bChangedIndex = LastOptionIndex != OptionIndex;
	LastOptionIndex = OptionIndex;

	if (bChangedIndex && bNotify)
	{
		NotifyUpdated();
	}

	return bChangedIndex;
}

void UUINavHorizontalComponent::NotifyUpdated()
{
	OnUpdated();
	OnValueChanged.Broadcast();
	OnNativeValueChanged.Broadcast();
	if (IsValid(ParentWidget))
	{
		ParentWidget->PropagateOnHorizCompUpdated(this);
	}
}

bool UUINavHorizontalComponent::SetOptionIndex(int NewIndex)
{
	OptionIndex = NewIndex;
	return Update(false);
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
