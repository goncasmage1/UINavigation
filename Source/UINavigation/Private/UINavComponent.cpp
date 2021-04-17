// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavComponent.h"
#include "UINavWidget.h"

UUINavComponent::UUINavComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bIsFocusable = false;
}

void UUINavComponent::NativeConstruct()
{
	check(NavButton != nullptr && "UINavComponent has no associated UINavButton");

	if (NavButton != nullptr)
	{
		NavButton->NavComp = this;
	}

	Super::NativeConstruct();
}

void UUINavComponent::CallCustomInput(const FName ActionName, uint8* Buffer)
{
	UFunction* CustomFunction = FindFunction(ActionName);
	if (CustomFunction != nullptr && CustomFunction->ParmsSize == sizeof(bool))
	{
		ProcessEvent(CustomFunction, Buffer);
	}
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

bool UUINavComponent::IsValid(const bool bIgnoreDisabledUINavButton) const
{
	return (Visibility != ESlateVisibility::Collapsed &&
			Visibility != ESlateVisibility::Hidden &&
			(!bIgnoreDisabledUINavButton || bIsEnabled));
}