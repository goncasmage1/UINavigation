// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavInputContainer.h"
#include "UINavWidget.h"
#include "UINavButton.h"
#include "UINavInputBox.h"
#include "UINavController.h"
#include "UINavComponent.h"
#include "UINavInputComponent.h"
#include "UINavBlueprintFunctionLibrary.h"
#include "GameFramework/InputSettings.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
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
}

void UUINavInputContainer::CreateActionBoxes()
{
	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;

	if (Actions.Num() == 0) return;

	int TempFirstButtonIndex = ParentWidget->UINavButtons.Num();
	int StartingInputComponentIndex = ParentWidget->UINavComponents.Num();

	int Iterations = ActionNames.Num();

	APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
	for (int i = 0; i < Iterations; ++i)
	{
		UUINavInputBox* NewInputBox = CreateWidget<UUINavInputBox>(PC, InputBox_BP);
		if (NewInputBox == nullptr) continue;
		NewInputBox->Container = this;
		NewInputBox->bIsAxis = false;
		NewInputBox->KeysPerInput = KeysPerInput;
		NewInputBox->InputName = ActionNames[i];

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
	} 
}

void UUINavInputContainer::CreateAxisBoxes()
{

	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	TArray<FInputAxisKeyMapping>& Axes = Settings->AxisMappings;

	if (Axes.Num() == 0) return;

	int TempFirstButtonIndex = ParentWidget->UINavButtons.Num();
	int StartingInputComponentIndex = ParentWidget->UINavComponents.Num();

	int Iterations = AxisNames.Num();

	APlayerController* PC = Cast<APlayerController>(UINavPC->GetOwner());
	for (int i = 0; i < Iterations; ++i)
	{
		UUINavInputBox* NewInputBox = CreateWidget<UUINavInputBox>(PC, InputBox_BP);
		if (NewInputBox == nullptr) continue;
		NewInputBox->Container = this;
		NewInputBox->bIsAxis = true;
		NewInputBox->KeysPerInput = KeysPerInput;
		NewInputBox->InputName = AxisNames[i];

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

FKey UUINavInputContainer::GetAxisKeyFromActionKey(FKey ActionKey)
{
	FString KeyName = ActionKey.GetFName().ToString();
	int KeyIndex = PossibleAxisNames.Find(KeyName);
	if (KeyIndex == INDEX_NONE) return FKey();

	switch (KeyIndex)
	{
		case 0:
			return FKey(FName("Gamepad_LeftTriggerAxis"));
			break;
		case 1:
			return FKey(FName("Gamepad_RightTriggerAxis"));
			break;
		case 2:
		case 3:
			return FKey(FName("Gamepad_LeftY"));
			break;
		case 4:
		case 5:
			return FKey(FName("Gamepad_LeftX"));
			break;
		case 6:
		case 7:
			return FKey(FName("Gamepad_RightY"));
			break;
		case 8:
		case 9:
			return FKey(FName("Gamepad_RightX"));
			break;
		case 10:
		case 11:
			return FKey(FName("MotionController_Left_Thumbstick_Y"));
			break;
		case 12:
		case 13:
			return FKey(FName("MotionController_Left_Thumbstick_X"));
			break;
		case 14:
		case 15:
			return FKey(FName("MotionController_Right_Thumbstick_Y"));
			break;
		case 16:
		case 17:
			return FKey(FName("MotionController_Right_Thumbstick_X"));
			break;
		case 18:
			return FKey(FName("MotionController_Left_TriggerAxis"));
			break;
		case 19:
			return FKey(FName("MotionController_Left_Grip1Axis"));
			break;
		case 20:
			return FKey(FName("MotionController_Left_Grip2Axis"));
			break;
		case 21:
			return FKey(FName("MotionController_Right_TriggerAxis"));
			break;
		case 22:
			return FKey(FName("MotionController_Right_Grip1Axis"));
			break;
		case 23:
			return FKey(FName("MotionController_Right_Grip2Axis"));
			break;
	}

	return FKey();
}
