// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavWidgetComponent.h"

#include "UINavMacros.h"
#include "UINavWidget.h"

UUINavWidgetComponent::UUINavWidgetComponent()
{
}

void UUINavWidgetComponent::BeginPlay()
{
	Super::BeginPlay();
	
	UUINavWidget* NavWidget = Cast<UUINavWidget>(GetWidget());
	if (NavWidget != nullptr)
	{
		if (NavWidget->WidgetComp != nullptr)
		{
			DISPLAYERROR("The player should only control 1 UINavWidgetComponent at a time!");
		}
		else
		{
			NavWidget->WidgetComp = this;
		}
	}
}

