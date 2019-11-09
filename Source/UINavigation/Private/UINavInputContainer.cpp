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

	CreateInputBoxes();
}

void UUINavInputContainer::OnSetupCompleted_Implementation()
{
}

void UUINavInputContainer::OnInputBoxAdded_Implementation(class UUINavInputBox* NewInputBox)
{
}

void UUINavInputContainer::OnRebindCancelled_Implementation(ERevertRebindReason RevertReason, FKey PressedKey)
{
}

void UUINavInputContainer::ResetKeyMappings()
{
	UUINavBlueprintFunctionLibrary::ResetInputSettings();
	for (UUINavInputBox* InputBox : ParentWidget->UINavInputBoxes) InputBox->ResetKeyMappings();
}

void UUINavInputContainer::CreateInputBoxes()
{
	if (InputBox_BP == nullptr) return;

	FirstButtonIndex = ParentWidget->UINavButtons.Num();

	CreateActionBoxes();
	CreateAxisBoxes();

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

void UUINavInputContainer::CreateActionBoxes()
{
	int TempFirstButtonIndex = ParentWidget->UINavButtons.Num();
	int StartingInputComponentIndex = ParentWidget->UINavComponents.Num();

	TArray<FName> ActionNameKeys;
	ActionNames.GetKeys(ActionNameKeys);
	int Iterations = ActionNameKeys.Num();

	APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
	for (int i = 0; i < Iterations; ++i)
	{
		UUINavInputBox* NewInputBox = CreateWidget<UUINavInputBox>(PC, InputBox_BP);
		if (NewInputBox == nullptr) continue;
		NewInputBox->Container = this;
		NewInputBox->KeysPerInput = KeysPerInput;
		NewInputBox->InputNameTuple = TPair<FName,FText>(ActionNameKeys[i] , ActionNames[ActionNameKeys[i]]);

		ActionPanel->AddChild(NewInputBox);

		ParentWidget->UINavInputBoxes.Add(NewInputBox);
		NumberOfInputs++;

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

		OnInputBoxAdded(NewInputBox);
	} 
}

void UUINavInputContainer::CreateAxisBoxes()
{
	int TempFirstButtonIndex = ParentWidget->UINavButtons.Num();
	int StartingInputComponentIndex = ParentWidget->UINavComponents.Num();

	TArray<FName> AxisNameKeys;
	AxisNames.GetKeys(AxisNameKeys);
	int Iterations = AxisNameKeys.Num();

	APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
	for (int i = 0; i < Iterations; ++i)
	{
		UUINavInputBox* NewInputBox = CreateWidget<UUINavInputBox>(PC, InputBox_BP);
		if (NewInputBox == nullptr) continue;
		NewInputBox->Container = this;
		NewInputBox->KeysPerInput = KeysPerInput;
		NewInputBox->InputNameTuple = TPair<FName, FText>(AxisNameKeys[i], AxisNames[AxisNameKeys[i]]);

		AxisPanel->AddChild(NewInputBox);

		ParentWidget->UINavInputBoxes.Add(NewInputBox);
		NumberOfInputs++;

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

		OnInputBoxAdded(NewInputBox);
	} 
}

bool UUINavInputContainer::IsKeyBeingUsed(FKey CompareKey) const
{
	for (UUINavInputBox* box : ParentWidget->UINavInputBoxes)
	{
		if (box->ContainsKey(CompareKey)) return true;
	}

	return false;
}

bool UUINavInputContainer::RespectsRestriction(FKey CompareKey, int Index)
{
	EInputRestriction Restriction = InputRestrictions[Index];

	switch (Restriction)
	{
		case EInputRestriction::None:
			return true;
			break;
		case EInputRestriction::Keyboard:
			return (!CompareKey.IsMouseButton() && !CompareKey.IsGamepadKey());
			break;
		case EInputRestriction::Mouse:
			return CompareKey.IsMouseButton();
			break;
		case EInputRestriction::Keyboard_Mouse:
			return !CompareKey.IsGamepadKey();
			break;
		case EInputRestriction::Gamepad:
			return CompareKey.IsGamepadKey();
			break;
		default:
			break;
	}

	return false;
}

FKey UUINavInputContainer::GetAxisFromKey(FKey Key)
{
	FKey* AxisKey = KeyToAxisMap.Find(Key);
	if (AxisKey == nullptr) return FKey();

	return *AxisKey;
}

FKey UUINavInputContainer::GetKeyFromAxis(FKey Key, bool bPositive)
{
	FAxis2D_Keys* Axis2DKeys = Axis2DToKeyMap.Find(Key);
	if (Axis2DKeys == nullptr) return FKey();
	
	return bPositive ? Axis2DKeys->PositiveKey : Axis2DKeys->NegativeKey;
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
