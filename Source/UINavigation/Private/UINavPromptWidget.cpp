// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#include "UINavPromptWidget.h"

void UUINavPromptWidget::OnSelect_Implementation(int Index)
{
	ProcessPromptWidgetSelected(Index);
}

void UUINavPromptWidget::OnReturn_Implementation()
{
	ProcessPromptWidgetSelected(ReturnSelectedIndex);
}

void UUINavPromptWidget::ProcessPromptWidgetSelected(int Index)
{
	if (ParentWidget != nullptr)
	{
		ParentWidget->PromptWidgetClass = GetClass();
		ParentWidget->PromptSelectedIndex = Index;
	}
	ReturnToParent();
}