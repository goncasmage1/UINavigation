// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "SwapKeysWidget.h"
#include "UINavInputBox.h"
#include "UINavPCComponent.h"
#include "Data/RevertRebindReason.h"
#include "UINavBlueprintFunctionLibrary.h"
#include "Data/PromptDataSwapKeys.h"

void USwapKeysWidget::OnSelect_Implementation(UUINavComponent* Component)
{
	NotifySwapResult((Component == FirstComponent) == FirstComponentIsAccept);
}

void USwapKeysWidget::OnReturn_Implementation()
{
	NotifySwapResult(FirstComponentIsAccept);
}

void USwapKeysWidget::NotifySwapResult(const bool bSwap)
{
	UPromptDataSwapKeys* SwapKeysPromptData = NewObject<UPromptDataSwapKeys>();
	if (!IsValid(SwapKeysPromptData))
	{
		return;
	}

	SwapKeysPromptData->bShouldSwap = bSwap;
	SwapKeysPromptData->InputCollisionData = InputCollisionData;
	SwapKeysPromptData->CurrentInputBox = CurrentInputBox;
	SwapKeysPromptData->CollidingInputBox = CollidingInputBox;

	ProcessPromptWidgetSelected(SwapKeysPromptData);
}
