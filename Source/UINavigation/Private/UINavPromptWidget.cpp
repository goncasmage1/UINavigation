// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavPromptWidget.h"
#include "UINavBlueprintFunctionLibrary.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Data/PromptData.h"

void UUINavPromptWidget::NativeConstruct()
{
	SetAllNavigationRules(EUINavigationRule::Stop, NAME_None);

	Super::NativeConstruct();

	if (!Title.IsEmpty())
	{
		if (IsValid(TitleText))
		{
			TitleText->SetText(Title);
		}

		if (IsValid(TitleRichText))
		{
			TitleRichText->SetText(UUINavBlueprintFunctionLibrary::ApplyStyleRowToText(Title, TitleStyleRowName));
		}
	}

	if (!Message.IsEmpty())
	{
		if (IsValid(MessageText))
		{
			MessageText->SetText(Message);
		}

		if (IsValid(MessageRichText))
		{
			MessageRichText->SetText(UUINavBlueprintFunctionLibrary::ApplyStyleRowToText(Message, MessageStyleRowName));
		}
	}
}

void UUINavPromptWidget::OnSelect_Implementation(UUINavComponent* Component)
{
	ProcessPromptWidgetSelected(UUINavBlueprintFunctionLibrary::CreateBinaryPromptData(IsAcceptComponent(Component)));
}

void UUINavPromptWidget::OnReturn_Implementation()
{
	ProcessPromptWidgetSelected(UUINavBlueprintFunctionLibrary::CreateBinaryPromptData(false));
}

void UUINavPromptWidget::ProcessPromptWidgetSelected(UPromptDataBase* InPromptData)
{
	if (!IsValid(InPromptData))
	{
		return;
	}

	ReturnToParent();

	Callback.ExecuteIfBound(InPromptData);
}

bool UUINavPromptWidget::IsAcceptComponent(UUINavComponent* Component) const
{
	return (Component == FirstComponent) == FirstComponentIsAccept;
}
