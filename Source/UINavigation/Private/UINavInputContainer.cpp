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

	if (InputRestrictions.Num() == 0) InputRestrictions.Add(EInputRestriction::None);
	else if (InputRestrictions.Num() > 3) InputRestrictions.SetNum(3);

	if (IsValid(UINavPC) && !UINavPC->GetCurrentPlatformData().bCanUseKeyboardMouse)
	{
		for (int32 i = InputRestrictions.Num() - 1; i >= 0; --i)
		{
			const EInputRestriction InputRestriction = InputRestrictions[i];
			if (InputRestriction == EInputRestriction::Keyboard ||
				InputRestriction == EInputRestriction::Mouse ||
				InputRestriction == EInputRestriction::Keyboard_Mouse)
			{
				InputRestrictions.RemoveAt(i);
			}
		}
	}

	KeysPerInput = InputRestrictions.Num();

	DecidedCallback.BindUFunction(this, FName("SwapKeysDecided"));

	SetupInputBoxes();

	Super::NativeConstruct();
}

FReply UUINavInputContainer::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	const FReply Reply = Super::NativeOnFocusReceived(InGeometry, InFocusEvent);

	if (InputBoxes.Num() > 0)
	{
		InputBoxes[0]->InputButton1->SetFocus();
	}

	return Reply;
}

void UUINavInputContainer::OnAddInputBox_Implementation(class UUINavInputBox* NewInputBox)
{
	if (InputBoxesPanel != nullptr)
	{
		InputBoxesPanel->AddChild(NewInputBox);
	}
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

UUINavInputBox* UUINavInputContainer::GetInputBoxAtIndex(const int Index) const
{
	if (Index == -1 && InputBoxes.Num() > 0)
	{
		return InputBoxes.Last();
	}

	if (!InputBoxes.IsValidIndex(Index))
	{
		return nullptr;
	}

	return InputBoxes[Index];
}

void UUINavInputContainer::SetupInputBoxes()
{
	if (InputBox_BP == nullptr) return;

	InputBoxes.Reset();

	NumberOfInputs = 0;
	for (const TPair<UInputMappingContext*, FInputContainerEnhancedActionDataArray>& Context : EnhancedInputs)
	{
		NumberOfInputs += Context.Value.Actions.Num();
	}

	if (NumberOfInputs == 0)
	{
		DISPLAYERROR(TEXT("Input Container has no Enhanced Input data!"));
		return;
	}
	
	CreateInputBoxes();
}

void UUINavInputContainer::CreateInputBoxes()
{
	if (InputBox_BP == nullptr || UINavPC == nullptr) return;

	for (int i = 0; i < NumberOfInputs; ++i)
	{
		UUINavInputBox* NewInputBox = CreateWidget<UUINavInputBox>(this, InputBox_BP);
		InputBoxes.Add(NewInputBox);
		NewInputBox->Container = this;
		NewInputBox->KeysPerInput = KeysPerInput;

		int Index = i;
		for (const TPair<UInputMappingContext*, FInputContainerEnhancedActionDataArray>& Context : EnhancedInputs)
		{
			if (Index >= Context.Value.Actions.Num())
			{
				Index -= Context.Value.Actions.Num();
			}
			else
			{
				NewInputBox->InputContext = Context.Key;
				NewInputBox->InputActionData = Context.Value.Actions[Index];
				NewInputBox->EnhancedInputGroups = Context.Value.Actions[Index].InputGroupsOverride.Num() > 0 ? Context.Value.Actions[Index].InputGroupsOverride : Context.Value.InputGroups;
				break;
			}
		}
	}

	for (int i = 0; i < NumberOfInputs; ++i)
	{
		UUINavInputBox* const InputBox = InputBoxes[i];
		InputBox->CreateKeyWidgets();
		OnAddInputBox(InputBox);
	}
}

ERevertRebindReason UUINavInputContainer::CanRegisterKey(UUINavInputBox * InputBox, const FKey NewKey, const bool bIsHold, const int Index, int& OutCollidingActionIndex, int& OutCollidingKeyIndex)
{
	if (!NewKey.IsValid()) return ERevertRebindReason::BlacklistedKey;
	if (KeyWhitelist.Num() > 0 && !KeyWhitelist.Contains(NewKey)) return ERevertRebindReason::NonWhitelistedKey;
	if (KeyBlacklist.Contains(NewKey)) return ERevertRebindReason::BlacklistedKey;
	if (!RespectsRestriction(NewKey, Index)) return ERevertRebindReason::RestrictionMismatch;

	const int ExistingKeyIndex = InputBox->ContainsKey(NewKey);
	if (ExistingKeyIndex != INDEX_NONE && InputBox->bIsHoldInput[ExistingKeyIndex]) return ERevertRebindReason::UsedBySameInput;
	if (!CanUseKey(InputBox, NewKey, bIsHold, OutCollidingActionIndex, OutCollidingKeyIndex)) return ERevertRebindReason::UsedBySameInputGroup;

	return ERevertRebindReason::None;
}

bool UUINavInputContainer::CanUseKey(UUINavInputBox* InputBox, const FKey CompareKey, const bool bIsHold, int& OutCollidingActionIndex, int& OutCollidingKeyIndex) const
{
	if (InputBox->EnhancedInputGroups.Num() == 0) InputBox->EnhancedInputGroups.Add(-1);

	for (int i = 0; i < InputBoxes.Num(); ++i)
	{
		if (InputBox == InputBoxes[i]) continue;

		const int KeyIndex = InputBoxes[i]->ContainsKey(CompareKey);
		if (KeyIndex != INDEX_NONE)
		{
			bool bIsCollidingInputBox = false;
			if (InputBox->EnhancedInputGroups.Contains(-1) ||
				InputBoxes[i]->EnhancedInputGroups.Contains(-1))
			{
				bIsCollidingInputBox = true;
			}

			if (!bIsCollidingInputBox)
			{
				for (int InputGroup : InputBox->EnhancedInputGroups)
				{
					if (InputBoxes[i]->EnhancedInputGroups.Contains(InputGroup))
					{
						bIsCollidingInputBox = true;
						break;
					}
				}
			}

			if (bIsHold != InputBoxes[i]->bIsHoldInput[KeyIndex])
			{
				return true;
			}

			if (bIsCollidingInputBox)
			{
				OutCollidingActionIndex = i;
				OutCollidingKeyIndex = KeyIndex;
				return false;
			}

			return true;
		}
	}

	return true;
}

bool UUINavInputContainer::RespectsRestriction(const FKey CompareKey, const int Index)
{
	const EInputRestriction Restriction = InputRestrictions[Index];

	return UUINavBlueprintFunctionLibrary::RespectsRestriction(CompareKey, Restriction);
}

void UUINavInputContainer::ResetInputBox(const FName InputName, const EAxisType AxisType)
{
	for (UUINavInputBox* InputBox : InputBoxes)
	{
		if (InputBox->InputName.IsEqual(InputName) &&
			InputBox->AxisType == AxisType)
		{
			InputBox->ResetKeyWidgets();
			break;
		}
	}
}

void UUINavInputContainer::SwapKeysDecided(const UPromptDataBase* const PromptData)
{
	const UPromptDataSwapKeys* const SwapKeysPromptData = Cast<UPromptDataSwapKeys>(PromptData);
	if (SwapKeysPromptData->CurrentInputBox != nullptr && SwapKeysPromptData->CollidingInputBox != nullptr)
	{
		if (SwapKeysPromptData->bShouldSwap)
		{
			const bool bWasCurrentInputBoxHold = SwapKeysPromptData->CurrentInputBox->bIsHoldInput[SwapKeysPromptData->InputCollisionData.CurrentKeyIndex];
			const bool bWasCollidingInputBoxHold = SwapKeysPromptData->CollidingInputBox->bIsHoldInput[SwapKeysPromptData->InputCollisionData.CollidingKeyIndex];
			FEnhancedActionKeyMapping* CurrentActionMapping = SwapKeysPromptData->CurrentInputBox->GetActionMapping(SwapKeysPromptData->InputCollisionData.CurrentKeyIndex);
			FEnhancedActionKeyMapping* CollidingActionMapping = SwapKeysPromptData->CollidingInputBox->GetActionMapping(SwapKeysPromptData->InputCollisionData.CollidingKeyIndex);
			const TObjectPtr<UInputTrigger> CurrentInputBoxTrigger = CollidingActionMapping != nullptr && !CollidingActionMapping->Triggers.IsEmpty() && bWasCurrentInputBoxHold && !bWasCollidingInputBoxHold ? DuplicateObject<UInputTrigger>(CollidingActionMapping->Triggers[0], this) : nullptr;
			const TObjectPtr<UInputTrigger> CollidingInputBoxTrigger = CurrentActionMapping != nullptr && !CurrentActionMapping->Triggers.IsEmpty() && !bWasCurrentInputBoxHold && bWasCollidingInputBoxHold ? DuplicateObject<UInputTrigger>(CurrentActionMapping->Triggers[0], this) : nullptr;

			int32 ModifiedActionMappingIndex = SwapKeysPromptData->CurrentInputBox->FinishUpdateNewKey(bWasCollidingInputBoxHold, -1, CurrentInputBoxTrigger);
			SwapKeysPromptData->CollidingInputBox->UpdateInputKey(SwapKeysPromptData->InputCollisionData.CurrentInputKey,
				bWasCurrentInputBoxHold,
				SwapKeysPromptData->InputCollisionData.CollidingKeyIndex,
				true,
				ModifiedActionMappingIndex,
				CollidingInputBoxTrigger);
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

UUINavInputBox* UUINavInputContainer::GetOppositeInputBox(const FInputContainerEnhancedActionData& ActionData)
{
	for (UUINavInputBox* InputBox : InputBoxes)
	{
		if (InputBox->InputActionData.Action == ActionData.Action &&
			InputBox->InputActionData.Axis == ActionData.Axis &&
			((InputBox->InputActionData.AxisScale == EAxisType::Positive && ActionData.AxisScale == EAxisType::Negative) ||
			(InputBox->InputActionData.AxisScale == EAxisType::Negative && ActionData.AxisScale == EAxisType::Positive)))
		{
			return InputBox;
		}
	}

	return nullptr;
}

UUINavInputBox* UUINavInputContainer::GetOppositeInputBox(const FName& InputName, const EAxisType AxisType)
{
	for (UUINavInputBox* InputBox : InputBoxes)
	{
		if (InputBox->InputName == InputName &&
			((InputBox->AxisType == EAxisType::Positive && AxisType == EAxisType::Negative) ||
			(InputBox->AxisType == EAxisType::Negative && AxisType == EAxisType::Positive)))
		{
			return InputBox;
		}
	}

	return nullptr;
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
