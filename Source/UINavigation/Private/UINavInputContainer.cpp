// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavInputContainer.h"
#include "UINavWidget.h"
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

bool UUINavInputContainer::CanUseKey(const UUINavInputBox* InputBox, FKey CompareKey) const
{
	TArray<int> InputGroups = InputBox->InputData.InputGroups;
	if (InputGroups.Num() == 0) InputGroups.Add(-1);

	for (UUINavInputBox* box : ParentWidget->UINavInputBoxes)
	{
		if (InputBox == box) continue;

		if (box->ContainsKey(CompareKey))
		{
			if (InputGroups.Contains(-1) ||
				box->InputData.InputGroups.Contains(-1)) return false;

			TArray<int> CollidingInputGroups = box->InputData.InputGroups;
			if (CollidingInputGroups.Contains(-1)) return false;

			for (int InputGroup : InputGroups)
			{
				if (CollidingInputGroups.Contains(InputGroup)) return false;
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
