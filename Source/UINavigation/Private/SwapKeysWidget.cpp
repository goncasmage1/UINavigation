// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#include "SwapKeysWidget.h"
#include "UINavInputBox.h"
#include "Data/RevertRebindReason.h"

void USwapKeysWidget::OnSelect_Implementation(int Index)
{
	NotifySwapResult(Index);
}

void USwapKeysWidget::OnReturn_Implementation()
{
	NotifySwapResult(ReturnSelectedIndex);
}

void USwapKeysWidget::NotifySwapResult(const int Index)
{
	if (CurrentInputBox != nullptr && CollidingInputBox != nullptr)
	{
		if (Index > 0)
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

	ProcessPromptWidgetSelected(Index);
}
