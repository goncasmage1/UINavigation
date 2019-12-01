// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#include "UINavInputContainer.h"
#include "UINavWidget.h"
#include "SwapKeysWidget.h"
#include "UINavPCComponent.h"
#include "UINavButton.h"
#include "UINavInputBox.h"
#include "UINavComponent.h"
#include "UINavInputComponent.h"
#include "UINavBlueprintFunctionLibrary.h"
#include "GameFramework/InputSettings.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/DataTable.h"
#include "Components/PanelWidget.h"
#include "Kismet/GameplayStatics.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

void UUINavInputContainer::Init(UUINavWidget * NewParent)
{
	ParentWidget = NewParent;
	UINavPC = NewParent->UINavPC;

	bIsFocusable = false;

	if (InputRestrictions.Num() == 0) InputRestrictions.Add(EInputRestriction::None);
	else if (InputRestrictions.Num() > 3) InputRestrictions.SetNum(3);
	KeysPerInput = InputRestrictions.Num();

	SetupInputBoxes();
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

void UUINavInputContainer::OnRebindCancelled_Implementation(ERevertRebindReason RevertReason, FKey PressedKey)
{
}

bool UUINavInputContainer::RequestKeySwap(FInputCollisionData InputCollisionData, int CurrentInputIndex, int CollidingInputIndex)
{
	if (SwapKeysWidgetClass != nullptr)
	{
		APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
		USwapKeysWidget* SwapKeysWidget = CreateWidget<USwapKeysWidget>(PC, SwapKeysWidgetClass);
		SwapKeysWidget->CollidingInputBox = ParentWidget->UINavInputBoxes[CollidingInputIndex];
		SwapKeysWidget->CurrentInputBox = ParentWidget->UINavInputBoxes[CurrentInputIndex];
		SwapKeysWidget->InputCollisionData = InputCollisionData;
		ParentWidget->GoToBuiltWidget(SwapKeysWidget, false, false);
		return true;
	}
	return false;
}

void UUINavInputContainer::ResetKeyMappings()
{
	UUINavBlueprintFunctionLibrary::ResetInputSettings();
	for (UUINavInputBox* InputBox : ParentWidget->UINavInputBoxes) InputBox->ResetKeyWidgets();
}

void UUINavInputContainer::SetupInputBoxes()
{
	if (InputBox_BP == nullptr) return;

	NumberOfInputs = InputNames.Num();
	FirstButtonIndex = ParentWidget->UINavButtons.Num();

	CreateInputBoxes();

	LastButtonIndex = ParentWidget->UINavButtons.Num() != FirstButtonIndex ? ParentWidget->UINavButtons.Num() - 1 : FirstButtonIndex;

	switch (KeysPerInput)
	{
		case 2:
			TopButtonIndex = FirstButtonIndex + (TargetColumn == ETargetColumn::Right);
			BottomButtonIndex = LastButtonIndex -(TargetColumn != ETargetColumn::Right);
			break;
		case 3:
			TopButtonIndex = FirstButtonIndex + (int)TargetColumn;
			BottomButtonIndex = LastButtonIndex - (2 - (int)TargetColumn);
			break;
	}

	OnSetupCompleted();
}

void UUINavInputContainer::CreateInputBoxes()
{
	if (InputBox_BP == nullptr) return;

	int TempFirstButtonIndex = ParentWidget->UINavButtons.Num();
	int StartingInputComponentIndex = ParentWidget->UINavComponents.Num();

	int Iterations = InputNames.Num();

	APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
	for (int i = 0; i < Iterations; ++i)
	{
		UUINavInputBox* NewInputBox = CreateWidget<UUINavInputBox>(PC, InputBox_BP);
		NewInputBox->Container = this;
		NewInputBox->KeysPerInput = KeysPerInput;
		NewInputBox->InputName = InputNames[i];

		FInputRebindData InputRebindData;
		bool bSuccess = false;
		UINavPC->GetInputRebindData(InputNames[i], InputRebindData, bSuccess);
		if (bSuccess) NewInputBox->InputData = InputRebindData;

		OnAddInputBox(NewInputBox);
		NewInputBox->CreateKeyWidgets();

		ParentWidget->UINavInputBoxes.Add(NewInputBox);

		for (int j = 0; j < KeysPerInput; j++)
		{
			ParentWidget->UINavButtons.Add(nullptr);
			ParentWidget->UINavComponents.Add(nullptr);

			int NewButtonIndex = TempFirstButtonIndex + i * KeysPerInput + j;
			int NewComponentIndex = StartingInputComponentIndex + i * KeysPerInput + j;
			ParentWidget->UINavButtons[NewButtonIndex] = NewInputBox->InputButtons[j]->NavButton;
			ParentWidget->UINavComponents[NewComponentIndex] = NewInputBox->InputButtons[j];
			NewInputBox->InputButtons[j]->NavButton->ButtonIndex = NewButtonIndex;
			NewInputBox->InputButtons[j]->ComponentIndex = NewButtonIndex;
			ParentWidget->SetupUINavButtonDelegates(NewInputBox->InputButtons[j]->NavButton);
		}
	}
}

ERevertRebindReason UUINavInputContainer::CanRegisterKey(const UUINavInputBox * InputBox, FKey NewKey, int Index, int& CollidingActionIndex, int& CollidingKeyIndex)
{
	if (KeyBlacklist.Contains(NewKey) || !NewKey.IsValid())
	{
		return ERevertRebindReason::BlacklistedKey;
	}
	else if (!RespectsRestriction(NewKey, Index))
	{
		return ERevertRebindReason::RestrictionMismatch;
	}
	else if (!CanUseKey(InputBox, NewKey, CollidingActionIndex, CollidingKeyIndex))
	{
		return ERevertRebindReason::UsedBySameInputGroup;
	}

	return ERevertRebindReason::None;
}

bool UUINavInputContainer::CanUseKey(const UUINavInputBox* InputBox, FKey CompareKey, int& CollidingActionIndex, int& CollidingKeyIndex) const
{
	TArray<int> InputGroups = InputBox->InputData.InputGroups;
	if (InputGroups.Num() == 0) InputGroups.Add(-1);

	for (int i = 0; i < ParentWidget->UINavInputBoxes.Num(); ++i)
	{
		if (InputBox == ParentWidget->UINavInputBoxes[i]) continue;

		int KeyIndex = ParentWidget->UINavInputBoxes[i]->ContainsKey(CompareKey);
		if (KeyIndex != INDEX_NONE)
		{
			TArray<int> CollidingInputGroups = ParentWidget->UINavInputBoxes[i]->InputData.InputGroups;

			if (InputGroups.Contains(-1) ||
				CollidingInputGroups.Contains(-1))
			{
				CollidingActionIndex = i;
				CollidingKeyIndex = KeyIndex;
				return false;
			}

			for (int InputGroup : InputGroups)
			{
				if (CollidingInputGroups.Contains(InputGroup))
				{
					CollidingActionIndex = i;
					CollidingKeyIndex = KeyIndex;
					return false;
				}
			}
		}
	}

	return true;
}

bool UUINavInputContainer::RespectsRestriction(FKey CompareKey, int Index)
{
	EInputRestriction Restriction = InputRestrictions[Index];

	return UUINavBlueprintFunctionLibrary::RespectsRestriction(CompareKey, Restriction);
}

void UUINavInputContainer::ResetInputBox(FName InputName, EAxisType AxisType)
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

FKey UUINavInputContainer::GetAxisFromKey(FKey Key)
{
	FKey* AxisKey = KeyToAxisMap.Find(Key);
	return AxisKey == nullptr ? Key : *AxisKey;
}

int UUINavInputContainer::GetOffsetFromTargetColumn(bool bTop)
{
	switch (KeysPerInput)
	{
		case 2:
			if (bTop) return (TargetColumn == ETargetColumn::Right);
			else return -(TargetColumn != ETargetColumn::Right);
			break;
		case 3:
			if (bTop) return (int)TargetColumn;
			else return (-2 - (int)TargetColumn);
			break;
	}
	return 0;
}

void UUINavInputContainer::GetInputRebindData(int InputIndex, FInputRebindData& RebindData)
{
	if (InputIndex >= 0 && InputIndex < ParentWidget->UINavInputBoxes.Num())
	{
		RebindData = ParentWidget->UINavInputBoxes[InputIndex]->InputData;
	}
}
