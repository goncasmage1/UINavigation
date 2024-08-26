// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavInputContainer.h"
#include "UINavWidget.h"
#include "SwapKeysWidget.h"
#include "UINavPCComponent.h"
#include "UINavInputBox.h"
#include "UINavComponent.h"
#include "UINavInputComponent.h"
#include "UINavBlueprintFunctionLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/DataTable.h"
#include "GameFramework/PlayerController.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "IImageWrapper.h"
#include "EnhancedInputComponent.h"
#include "UINavMacros.h"
#include "Internationalization/Internationalization.h"
#include "HAL/Platform.h"
#include "Delegates/Delegate.h"
#include "Data/PromptDataSwapKeys.h"
#include "Data/PlatformConfigData.h"

void UUINavInputContainer::NativeConstruct()
{
	ParentWidget = UUINavWidget::GetOuterObject<UUINavWidget>(this);
	UINavPC = Cast<APlayerController>(GetOwningPlayer())->FindComponentByClass<UUINavPCComponent>();

	SetIsFocusable(false);
	
	DecidedCallback.BindUFunction(this, FName("SwapKeysDecided"));

	WidgetTree->ForWidgetAndChildren(InputBoxesPanel, [this](UWidget* Widget)
	{
		if (UUINavInputBox* InputBox = Cast<UUINavInputBox>(Widget))
		{
			InputBox->Container = this;
			InputBox->CreateKeyWidgets();
			this->InputBoxes.Add(InputBox);
		}
	});

	Super::NativeConstruct();
}

FReply UUINavInputContainer::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	const FReply Reply = Super::NativeOnFocusReceived(InGeometry, InFocusEvent);

	if (InputBoxes.Num() > 0)
	{
		InputBoxes[0]->InputButton->SetFocus();
	}

	return Reply;
}

void UUINavInputContainer::OnKeyRebinded_Implementation(FName InputName, FKey OldKey, FKey NewKey)
{
}

void UUINavInputContainer::OnRebindCancelled_Implementation(ERevertRebindReason RevertReason, FKey PressedKey)
{
}

bool UUINavInputContainer::RequestKeySwap(const FInputCollisionData& InputCollisionData, const int CurrentInputIndex, const int CollidingInputIndex) const
{
	if (SwapKeysWidgetClass != nullptr)
	{
		APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
		USwapKeysWidget* SwapKeysWidget = CreateWidget<USwapKeysWidget>(PC, SwapKeysWidgetClass);
		SwapKeysWidget->Title = SwapKeysTitleText;
		FFormatNamedArguments MessageArgs;
		MessageArgs.Add(TEXT("CollidingKey"), UINavPC->GetKeyText(InputCollisionData.PressedKey));
		MessageArgs.Add(TEXT("CollidingAction"), InputCollisionData.CollidingInputText);
		MessageArgs.Add(TEXT("OtherKey"), UINavPC->GetKeyText(InputCollisionData.CurrentInputKey));
		SwapKeysWidget->Message = FText::Format(SwapKeysMessageText, MessageArgs);
		SwapKeysWidget->CollidingInputBox = InputBoxes[CollidingInputIndex];
		SwapKeysWidget->CurrentInputBox = InputBoxes[CurrentInputIndex];
		SwapKeysWidget->InputCollisionData = InputCollisionData;
		SwapKeysWidget->SetCallback(DecidedCallback);
		UINavPC->GoToBuiltWidget(SwapKeysWidget, false, false, SpawnKeysWidgetZOrder);
		return true;
	}
	return false;
}

void UUINavInputContainer::ResetKeyMappings()
{
	UUINavBlueprintFunctionLibrary::ResetInputSettings(Cast<APlayerController>(UINavPC->GetOwner()));
	for (UUINavInputBox* InputBox : InputBoxes) InputBox->ResetKeyWidgets();
}

ERevertRebindReason UUINavInputContainer::CanRegisterKey(UUINavInputBox * InputBox, const FKey NewKey, int& OutCollidingActionIndex)
{
	if (!NewKey.IsValid()) return ERevertRebindReason::BlacklistedKey;
	if (KeyWhitelist.Num() > 0 && !KeyWhitelist.Contains(NewKey)) return ERevertRebindReason::NonWhitelistedKey;
	if (KeyBlacklist.Contains(NewKey)) return ERevertRebindReason::BlacklistedKey;
	if (!UUINavBlueprintFunctionLibrary::RespectsRestriction(NewKey, InputBox->InputRestriction)) return ERevertRebindReason::RestrictionMismatch;
	if (InputBox->ContainsKey(NewKey)) return ERevertRebindReason::UsedBySameInput;
	if (!CanUseKey(InputBox, NewKey, OutCollidingActionIndex)) return ERevertRebindReason::UsedBySameInputGroup;

	return ERevertRebindReason::None;
}

bool UUINavInputContainer::CanUseKey(UUINavInputBox* InputBox, const FKey CompareKey, int& OutCollidingActionIndex) const
{
	if (InputBox->EnhancedInputGroups.Num() == 0) InputBox->EnhancedInputGroups.Add(-1);

	for (int i = 0; i < InputBoxes.Num(); ++i)
	{
		if (InputBox == InputBoxes[i]) continue;
		
		if (InputBoxes[i]->ContainsKey(CompareKey))
		{
			if (InputBox->EnhancedInputGroups.Contains(-1) ||
				InputBoxes[i]->EnhancedInputGroups.Contains(-1))
			{
				OutCollidingActionIndex = i;
				return false;
			}

			for (int InputGroup : InputBox->EnhancedInputGroups)
			{
				if (InputBoxes[i]->EnhancedInputGroups.Contains(InputGroup))
				{
					OutCollidingActionIndex = i;
					return false;
				}
			}
		}
	}

	return true;
}

void UUINavInputContainer::SwapKeysDecided(const UPromptDataBase* const PromptData)
{
	const UPromptDataSwapKeys* const SwapKeysPromptData = Cast<UPromptDataSwapKeys>(PromptData);
	if (SwapKeysPromptData->CurrentInputBox != nullptr && SwapKeysPromptData->CollidingInputBox != nullptr)
	{
		if (SwapKeysPromptData->bShouldSwap)
		{
			SwapKeysPromptData->CurrentInputBox->FinishUpdateNewKey();
			SwapKeysPromptData->CollidingInputBox->UpdateInputKey(SwapKeysPromptData->InputCollisionData.CurrentInputKey,
				true);
		}
		else
		{
			SwapKeysPromptData->CurrentInputBox->CancelUpdateInputKey(ERevertRebindReason::SwapRejected);
		}
	}
}

UUINavInputBox* UUINavInputContainer::GetInputBoxInDirection(UUINavInputBox* InputBox, const EUINavigation Direction)
{
	if (!IsValid(InputBox))
	{
		return nullptr;
	}

	int Index;
	if (!InputBoxes.Find(InputBox, Index))
	{
		return nullptr;
	}

	switch (Direction)
	{
		case EUINavigation::Up:
			--Index;
			break;
		case EUINavigation::Down:
			++Index;
			break;
		default:
			return nullptr;
	}

	return InputBoxes.IsValidIndex(Index) ? InputBoxes[Index] : nullptr;
}

void UUINavInputContainer::GetInputRebindData(const int InputIndex, FInputRebindData& RebindData) const
{
	if (InputBoxes.IsValidIndex(InputIndex))
	{
		RebindData = InputBoxes[InputIndex]->InputData;
	}
}

void UUINavInputContainer::GetEnhancedInputRebindData(const int InputIndex, FInputRebindData& RebindData) const
{
	if (InputBoxes.IsValidIndex(InputIndex))
	{
		RebindData.InputText = InputBoxes[InputIndex]->GetCurrentText();
		RebindData.InputGroups = InputBoxes[InputIndex]->EnhancedInputGroups;
	}
}
