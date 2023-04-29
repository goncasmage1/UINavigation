// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "SwapKeysWidget.h"
#include "UINavInputBox.h"
#include "Data/RevertRebindReason.h"
#include "UINavBlueprintFunctionLibrary.h"

void USwapKeysWidget::OnSelect_Implementation(UUINavComponent* Component)
{
	NotifySwapResult(UUINavBlueprintFunctionLibrary::CreateBinaryPromptData((Component == FirstComponent) == FirstComponentIsAccept));
}

void USwapKeysWidget::OnReturn_Implementation()
{
	NotifySwapResult(UUINavBlueprintFunctionLibrary::CreateBinaryPromptData(FirstComponentIsAccept));
}

void USwapKeysWidget::NotifySwapResult(UPromptDataBinary* const InPromptData)
{
	if (CurrentInputBox != nullptr && CollidingInputBox != nullptr)
	{
		if (InPromptData->bAccept)
		{
			CurrentInputBox->FinishUpdateNewKey();
			CollidingInputBox->UpdateInputKey(InputCollisionData.CurrentInputKey,
											  InputCollisionData.CollidingKeyIndex,
											  true);
		}
		else
		{
			CurrentInputBox->CancelUpdateInputKey(ERevertRebindReason::SwapRejected);
		}
	}

	ProcessPromptWidgetSelected(InPromptData);
}
