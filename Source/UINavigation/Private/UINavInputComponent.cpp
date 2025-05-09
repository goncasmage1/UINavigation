// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavInputComponent.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "UINavBlueprintFunctionLibrary.h"

void UUINavInputComponent::SetIsHold(const FText& HoldText, const bool bIsHold)
{	
	if (!bIsHold)
	{
		if (IsValid(LeftText))
		{
			LeftText->SetText(FText());
		}
		if (IsValid(RightText))
		{
			RightText->SetText(FText());
		}

		if (IsValid(LeftRichText))
		{
			LeftRichText->SetText(FText());
		}
		if (IsValid(RightRichText))
		{
			RightRichText->SetText(FText());
		}

		return;
	}

	FString LeftStr;
	FString RightStr;
	HoldText.ToString().Split(TEXT("{}"), &LeftStr, &RightStr);
	
	if (!LeftStr.IsEmpty())
	{
		if (IsValid(LeftText))
		{
			LeftText->SetText(FText::FromString(LeftStr));
		}
		if (IsValid(LeftRichText))
		{
			LeftRichText->SetText(UUINavBlueprintFunctionLibrary::ApplyStyleRowToText(FText::FromString(LeftStr), NormalStyleRowName));
		}
	}

	if (!RightStr.IsEmpty())
	{
		if (IsValid(RightText))
		{
			RightText->SetText(FText::FromString(RightStr));
		}
		if (IsValid(RightRichText))
		{
			RightRichText->SetText(UUINavBlueprintFunctionLibrary::ApplyStyleRowToText(FText::FromString(RightStr), NormalStyleRowName));
		}
	}
}
