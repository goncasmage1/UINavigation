// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavPromptWidget.h"
#include "UINavBlueprintFunctionLibrary.h"

void UUINavPromptWidget::OnSelect_Implementation(UUINavComponent* Component)
{
}

void UUINavPromptWidget::OnReturn_Implementation()
{
	ProcessPromptWidgetSelected(UUINavBlueprintFunctionLibrary::CreateBinaryPromptData(FirstComponentIsAccept));
}

void UUINavPromptWidget::ProcessPromptWidgetSelected(UPromptDataBase* const InPromptData)
{
	if (!IsValid(InPromptData))
	{
		return;
	}

	if (ParentWidget != nullptr)
	{
		ParentWidget->PromptWidgetClass = GetClass();
		ParentWidget->PromptData = InPromptData;
	}
	ReturnToParent();
}