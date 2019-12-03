// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#include "SwapKeysWidget.h"
#include "UINavInputBox.h"
#include "Data/RevertRebindReason.h"

void USwapKeysWidget::OnSelect_Implementation(int Index)
{
	NotifySwapResult(Index > 0);
}

void USwapKeysWidget::OnReturn_Implementation()
{
	NotifySwapResult(false);
}

void USwapKeysWidget::NotifySwapResult(bool bAccept)
{
	if (CurrentInputBox != nullptr && CollidingInputBox != nullptr)
	{
		if (bAccept)
		{
			CurrentInputBox->FinishUpdateInputKey();
			CollidingInputBox->UpdateInputKey(InputCollisionData.CurrentInputKey,
											  InputCollisionData.CollidingKeyIndex,
											  true);
		}
		else
		{
			CurrentInputBox->CancelUpdateInputKey(ERevertRebindReason::SwapRejected);
		}
	}

	ReturnToParent();
}
