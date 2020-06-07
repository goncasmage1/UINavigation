// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavComponent.h"
#include "UINavButton.h"
#include "UINavWidget.h"

UUINavComponent::UUINavComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bIsFocusable = false;
}

void UUINavComponent::NativeConstruct()
{
	check(NavButton != nullptr && "UINavComponent has no associated UINavButton");

	Super::NativeConstruct();
}

void UUINavComponent::OnNavigatedTo_Implementation()
{
}

void UUINavComponent::OnNavigatedFrom_Implementation()
{
}

void UUINavComponent::OnSelected_Implementation()
{
}

void UUINavComponent::OnStartSelected_Implementation()
{
}

void UUINavComponent::OnStopSelected_Implementation()
{
}

bool UUINavComponent::IsValid()
{
	return (Visibility != ESlateVisibility::Collapsed &&
			Visibility != ESlateVisibility::Hidden &&
			bIsEnabled);
}