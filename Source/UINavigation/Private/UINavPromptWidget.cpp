// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavPromptWidget.h"
#include "UINavBlueprintFunctionLibrary.h"

void UUINavPromptWidget::OnSelect_Implementation(UUINavComponent* Component)
{
	ProcessPromptWidgetSelected(UUINavBlueprintFunctionLibrary::CreateBinaryPromptData(IsAcceptComponent(Component)));
}

void UUINavPromptWidget::OnReturn_Implementation()
{
	ProcessPromptWidgetSelected(UUINavBlueprintFunctionLibrary::CreateBinaryPromptData(false));
}

void UUINavPromptWidget::ProcessPromptWidgetSelected(UPromptDataBase* const InPromptData)
{
	if (!IsValid(InPromptData))
	{
		return;
	}

	ReturnToParent();

	Callback.Execute(InPromptData);
}

bool UUINavPromptWidget::IsAcceptComponent(UUINavComponent* Component) const
{
	return (Component == FirstComponent) == FirstComponentIsAccept;
}
