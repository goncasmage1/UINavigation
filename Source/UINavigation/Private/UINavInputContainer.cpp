// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavInputContainer.h"
#include "UINavWidget.h"
#include "SwapKeysWidget.h"
#include "UINavPCComponent.h"
#include "UINavButton.h"
#include "UINavInputBox.h"
#include "UINavComponent.h"
#include "UINavInputComponent.h"
#include "UINavBlueprintFunctionLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/DataTable.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "IImageWrapper.h"
#include "EnhancedInputComponent.h"
#include "UINavMacros.h"

#define USING_ENHANCED_INPUT IsValid(UINavPC->GetEnhancedInputComponent())

void UUINavInputContainer::Init(UUINavWidget * NewParent, const int GridIndex)
{
	ParentWidget = NewParent;
	UINavPC = NewParent->UINavPC;

	bIsFocusable = false;

	if (InputRestrictions.Num() == 0) InputRestrictions.Add(EInputRestriction::None);
	else if (InputRestrictions.Num() > 3) InputRestrictions.SetNum(3);
	KeysPerInput = InputRestrictions.Num();

	SetupInputBoxes(GridIndex);
}

void UUINavInputContainer::OnSetupCompleted_Implementation()
{
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
	// TODO: Fix for enhanced input
	if (SwapKeysWidgetClass != nullptr)
	{
		APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
		USwapKeysWidget* SwapKeysWidget = CreateWidget<USwapKeysWidget>(PC, SwapKeysWidgetClass);
		SwapKeysWidget->CollidingInputBox = ParentWidget->UINavInputBoxes[CollidingInputIndex];
		SwapKeysWidget->CurrentInputBox = ParentWidget->UINavInputBoxes[CurrentInputIndex];
		SwapKeysWidget->InputCollisionData = InputCollisionData;
		ParentWidget->GoToBuiltWidget(SwapKeysWidget, false, false, SpawnKeysWidgetZOrder);
		return true;
	}
	return false;
}

void UUINavInputContainer::ResetKeyMappings()
{
	UUINavBlueprintFunctionLibrary::ResetInputSettings(Cast<APlayerController>(UINavPC->GetOwner()));
	for (UUINavInputBox* InputBox : ParentWidget->UINavInputBoxes) InputBox->ResetKeyWidgets();
}

void UUINavInputContainer::SetupInputBoxes(const int GridIndex)
{
	if (InputBox_BP == nullptr) return;

	if (USING_ENHANCED_INPUT)
	{
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
	}
	else
	{
		NumberOfInputs = InputNames.Num();
	}
	
	FirstButtonIndex = ParentWidget->UINavButtons.Num();

	CreateInputBoxes(GridIndex);

	LastButtonIndex = ParentWidget->UINavButtons.Num() != FirstButtonIndex ? ParentWidget->UINavButtons.Num() - 1 : FirstButtonIndex;

	switch (KeysPerInput)
	{
		case 2:
			TopButtonIndex = FirstButtonIndex + (TargetColumn == ETargetColumn::Right);
			BottomButtonIndex = LastButtonIndex -(TargetColumn != ETargetColumn::Right);
			break;
		case 3:
			TopButtonIndex = FirstButtonIndex + static_cast<int>(TargetColumn);
			BottomButtonIndex = LastButtonIndex - (2 - static_cast<int>(TargetColumn));
			break;
	}

	OnSetupCompleted();
}

void UUINavInputContainer::CreateInputBoxes(const int GridIndex)
{
	if (InputBox_BP == nullptr) return;

	const int TempFirstButtonIndex = ParentWidget->UINavButtons.Num();

	APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
	for (int i = 0; i < NumberOfInputs; ++i)
	{
		UUINavInputBox* NewInputBox = CreateWidget<UUINavInputBox>(PC, InputBox_BP);
		NewInputBox->Container = this;
		NewInputBox->KeysPerInput = KeysPerInput;
		if (USING_ENHANCED_INPUT)
		{
			int Index = i;
			for (const TPair<UInputMappingContext*, FInputContainerEnhancedActionDataArray>&Context : EnhancedInputs)
			{
				if (Index >= Context.Value.Actions.Num())
				{
					Index -= Context.Value.Actions.Num();
				}
				else
				{
					NewInputBox->InputContext = Context.Key;
					NewInputBox->InputActionData = Context.Value.Actions[Index];
					NewInputBox->EnhancedInputGroups = Context.Value.InputGroups;
					break;
				}
			}
		}
		else
		{
			NewInputBox->InputName = InputNames[i];

			FInputRebindData InputRebindData;
			bool bSuccess = false;
			UINavPC->GetInputRebindData(InputNames[i], InputRebindData, bSuccess);
			if (bSuccess) NewInputBox->InputData = InputRebindData;
		}

		ParentWidget->UINavInputBoxes.Add(NewInputBox);
	}

	for (int i = 0; i < NumberOfInputs; ++i)
	{
		UUINavInputBox* const InputBox = ParentWidget->UINavInputBoxes[i];
		InputBox->CreateKeyWidgets();

		for (int j = 0; j < KeysPerInput; j++)
		{
			ParentWidget->UINavButtons.Add(nullptr);

			const int NewButtonIndex = TempFirstButtonIndex + i * KeysPerInput + j;
			ParentWidget->UINavButtons[NewButtonIndex] = InputBox->InputButtons[j]->NavButton;
			InputBox->InputButtons[j]->NavButton->ButtonIndex = NewButtonIndex;
			if (GridIndex != -1)
			{
				InputBox->InputButtons[j]->NavButton->GridIndex = GridIndex;
				InputBox->InputButtons[j]->NavButton->IndexInGrid = (i * KeysPerInput) + j;
			}
			ParentWidget->SetupUINavButtonDelegates(InputBox->InputButtons[j]->NavButton);
		}

		OnAddInputBox(InputBox);
	}
}

ERevertRebindReason UUINavInputContainer::CanRegisterKey(const UUINavInputBox * InputBox, const FKey NewKey, const int Index, int& OutCollidingActionIndex, int& OutCollidingKeyIndex)
{
	if (!NewKey.IsValid()) return ERevertRebindReason::BlacklistedKey;
	if (KeyWhitelist.Num() > 0 && !KeyWhitelist.Contains(NewKey)) return ERevertRebindReason::NonWhitelistedKey;
	if (KeyBlacklist.Contains(NewKey)) return ERevertRebindReason::BlacklistedKey;
	if (!RespectsRestriction(NewKey, Index)) return ERevertRebindReason::RestrictionMismatch;
	if (InputBox->ContainsKey(NewKey) != INDEX_NONE) return ERevertRebindReason::UsedBySameInput;
	if (!CanUseKey(InputBox, NewKey, OutCollidingActionIndex, OutCollidingKeyIndex)) return ERevertRebindReason::UsedBySameInputGroup;

	return ERevertRebindReason::None;
}

bool UUINavInputContainer::CanUseKey(const UUINavInputBox* InputBox, const FKey CompareKey, int& OutCollidingActionIndex, int& OutCollidingKeyIndex) const
{
	TArray<int> InputGroups = USING_ENHANCED_INPUT ? InputBox->EnhancedInputGroups : InputBox->InputData.InputGroups;
	if (InputGroups.Num() == 0) InputGroups.Add(-1);

	for (int i = 0; i < ParentWidget->UINavInputBoxes.Num(); ++i)
	{
		if (InputBox == ParentWidget->UINavInputBoxes[i]) continue;

		const int KeyIndex = ParentWidget->UINavInputBoxes[i]->ContainsKey(CompareKey);
		if (KeyIndex != INDEX_NONE)
		{
			TArray<int> CollidingInputGroups = USING_ENHANCED_INPUT ? ParentWidget->UINavInputBoxes[i]->EnhancedInputGroups : ParentWidget->UINavInputBoxes[i]->InputData.InputGroups;

			if (InputGroups.Contains(-1) ||
				CollidingInputGroups.Contains(-1))
			{
				OutCollidingActionIndex = i;
				OutCollidingKeyIndex = KeyIndex;
				return false;
			}

			for (int InputGroup : InputGroups)
			{
				if (CollidingInputGroups.Contains(InputGroup))
				{
					OutCollidingActionIndex = i;
					OutCollidingKeyIndex = KeyIndex;
					return false;
				}
			}
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
	for (UUINavInputBox* InputBox : ParentWidget->UINavInputBoxes)
	{
		if (InputBox->InputName.IsEqual(InputName) &&
			InputBox->AxisType == AxisType)
		{
			InputBox->ResetKeyWidgets();
			break;
		}
	}
}

UUINavInputBox* UUINavInputContainer::GetOppositeInputBox(const FInputContainerEnhancedActionData& ActionData)
{
	for (UUINavInputBox* InputBox : ParentWidget->UINavInputBoxes)
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
	for (UUINavInputBox* InputBox : ParentWidget->UINavInputBoxes)
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

void UUINavInputContainer::GetAxisPropertiesFromMapping(const FEnhancedActionKeyMapping& ActionMapping,
	bool& bOutPositive, EInputAxis& OutAxis) const
{
	TArray<UInputModifier*> Modifiers(ActionMapping.Modifiers);
	Modifiers.Append(ActionMapping.Action->Modifiers);
	bOutPositive = true;
	if (!UINavPC->IsAxis2D(ActionMapping.Key))
	{
		OutAxis = EInputAxis::X;
	}
	
	for (const UInputModifier* Modifier : Modifiers)
	{
		const UInputModifierNegate* Negate = Cast<UInputModifierNegate>(Modifier);
		if (Negate != nullptr)
		{
			bOutPositive = !bOutPositive;
			continue;
		}

		// TODO: Add support for Scalar input modifier
		
		const UInputModifierSwizzleAxis* Swizzle = Cast<UInputModifierSwizzleAxis>(Modifier);
		if (Swizzle != nullptr && !UINavPC->IsAxis2D(ActionMapping.Key))
		{
			switch(Swizzle->Order)
			{
			case EInputAxisSwizzle::YXZ:
			case EInputAxisSwizzle::YZX:
				OutAxis = EInputAxis::Y;
				break;
			case EInputAxisSwizzle::ZXY:
			case EInputAxisSwizzle::ZYX:
				OutAxis = EInputAxis::Z;
				break;
			}
			continue;
		}
	}
}

FKey UUINavInputContainer::GetAxisFromKey(FKey Key)
{
	FKey* AxisKey = KeyToAxisMap.Find(Key);
	return AxisKey == nullptr ? Key : *AxisKey;
}

int UUINavInputContainer::GetOffsetFromTargetColumn(const bool bTop) const
{
	switch (KeysPerInput)
	{
		case 2:
			if (bTop) return (TargetColumn == ETargetColumn::Right);
			else return -(TargetColumn != ETargetColumn::Right);
		case 3:
			if (bTop) return static_cast<int>(TargetColumn);
			else return (-2 - static_cast<int>(TargetColumn));
	}
	return 0;
}

void UUINavInputContainer::GetInputRebindData(const int InputIndex, FInputRebindData& RebindData) const
{
	if (ParentWidget->UINavInputBoxes.IsValidIndex(InputIndex))
	{
		RebindData = ParentWidget->UINavInputBoxes[InputIndex]->InputData;
	}
}

void UUINavInputContainer::GetEnhancedInputRebindData(const int InputIndex, FInputRebindData& RebindData) const
{
	if (ParentWidget->UINavInputBoxes.IsValidIndex(InputIndex))
	{
		RebindData.InputText = ParentWidget->UINavInputBoxes[InputIndex]->InputText->GetText();
		RebindData.InputGroups = ParentWidget->UINavInputBoxes[InputIndex]->EnhancedInputGroups;
	}
}
