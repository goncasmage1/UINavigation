// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavInputBox.h"
#include "UINavInputComponent.h"
#include "UINavInputContainer.h"
#include "UINavSettings.h"
#include "UINavPCComponent.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Data/RevertRebindReason.h"
#include "Engine/DataTable.h"
#include "GameFramework/InputSettings.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

#define IS_RIGHT_SCALE(Axis) ((Axis.Scale > 0.0f && IS_POSITIVE_AXIS) || (Axis.Scale < 0.0f && IS_NEGATIVE_AXIS))
#define GET_REVERSE_AXIS (AxisType == EAxisType::Positive ? EAxisType::Negative : EAxisType::Positive)

void UUINavInputBox::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = false;
}

void UUINavInputBox::CreateKeyWidgets()
{
	const UInputSettings* Settings = GetDefault<UInputSettings>();
	TArray<FInputActionKeyMapping> Actions;
	TArray<FInputAxisKeyMapping> Axes;

	InputButtons = {
		InputButton1,
		InputButton2,
		InputButton3
	};

	ProcessInputName();

	if (IS_AXIS) Settings->GetAxisMappingByName(InputName, Axes);
	else Settings->GetActionMappingByName(InputName, Actions);

	if ((IS_AXIS && Axes.Num() == 0) || 
		(!IS_AXIS && Actions.Num() == 0))
	{
		FString Message = TEXT("Couldn't find Input with name ");
		Message.Append(*InputName.ToString());
		DISPLAYERROR(Message);
		return;
	}

	for (int j = 0; j < 3; j++)
	{
		UUINavInputComponent* NewInputButton = InputButtons[j];

		if (j < KeysPerInput)
		{
			int Iterations = IS_AXIS ? Axes.Num() : Actions.Num();
			FKey PotentialAxisKey;
			for (int i = Iterations - 1; i >= 0; --i)
			{
				if ((IS_AXIS && !PotentialAxisKey.IsValid() && IS_RIGHT_SCALE(Axes[i])) ||
					(!IS_AXIS))
				{
					FKey NewKey = IS_AXIS ? GetKeyFromAxis(Axes[i].Key) : Actions[i].Key;
					if (!Container->RespectsRestriction(NewKey, j))
					{
						continue;
					}

					if (TrySetupNewKey(NewKey, j, NewInputButton))
					{
						break;
					}
				}
				else if (IS_AXIS &&
						 !PotentialAxisKey.IsValid() &&
						 Container->RespectsRestriction(Axes[i].Key, j))
				{
					FKey NewPotentialKey = Container->UINavPC->GetKeyFromAxis(Axes[i].Key, AxisType == EAxisType::Positive);
					if (!Keys.Contains(NewPotentialKey))
					{
						PotentialAxisKey = NewPotentialKey;
					}
				}
			}
			if (PotentialAxisKey.IsValid())
			{
				TrySetupNewKey(PotentialAxisKey, j, NewInputButton);
			}
		}
		else
		{
			NewInputButton->SetVisibility(Container->bCollapseInputBoxes ? ESlateVisibility::Collapsed : ESlateVisibility::Hidden);
		}

		if (Keys.Num() - 1 < j)
		{
			NewInputButton->NavText->SetText(Container->EmptyKeyText);
			NewInputButton->InputImage->SetVisibility(ESlateVisibility::Collapsed);
			NewInputButton->NavText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			Keys.Add(FKey());
		}
	}
}

bool UUINavInputBox::TrySetupNewKey(FKey NewKey, int KeyIndex, UUINavInputComponent* NewInputButton)
{
	if (!NewKey.IsValid() || Keys.IsValidIndex(KeyIndex) || Keys.Contains(NewKey)) return false;

	Keys.Add(NewKey);

	if (UpdateKeyIconForKey(KeyIndex))
	{
		bUsingKeyImage[KeyIndex] = true;
		NewInputButton->InputImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		NewInputButton->NavText->SetVisibility(ESlateVisibility::Collapsed);
	}
	NewInputButton->NavText->SetText(GetKeyText(KeyIndex));

	return true;
}

void UUINavInputBox::ResetKeyWidgets()
{
	Keys.Empty();
	bUsingKeyImage = { false, false, false };
	InputButtons.Empty();
	CreateKeyWidgets();

	if (Container->UINavPC != nullptr && Container->UINavPC->KeyMap.Contains(InputName.ToString()))
	{
		Container->UINavPC->KeyMap.Add(InputName.ToString(), Keys);
	}
}

void UUINavInputBox::UpdateInputKey(FKey NewKey, int Index)
{
	ERevertRebindReason RevertReason = Container->CanRegisterKey(this, NewKey, Index);
	if (RevertReason != ERevertRebindReason::None)
	{
		Container->OnRebindCancelled(RevertReason, NewKey);
		RevertToKeyText(Index);
		return;
	}

	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;
	TArray<FInputAxisKeyMapping>& Axes = Settings->AxisMappings;

	FKey NewAxisKey;
	bool bFound = false;
	bool bRemoved2DAxis = false;
	int Iterations = IS_AXIS ? Axes.Num() : Actions.Num();
	for (int i = Iterations - 1; i >= 0; --i)
	{
		if ((IS_AXIS && Axes[i].AxisName.IsEqual(InputName)) || 
			(!IS_AXIS && Actions[i].ActionName.IsEqual(InputName)))
		{
			FKey InputKey = IS_AXIS ? GetKeyFromAxis(Axes[i].Key) : Actions[i].Key;
			if (InputKey == Keys[Index] &&
				Container->RespectsRestriction(NewKey, Index))
			{
				if (IS_AXIS)
				{
					if (InputKey != Axes[i].Key &&
						!IS_RIGHT_SCALE(Axes[i]))
					{
						NewAxisKey = NewKey;
						break;
					}

					//Remove indirect axis in opposite scale input
					if (Container->UINavPC->Is2DAxis(Axes[i].Key))
					{
						bRemoved2DAxis = true;
					}

					Axes[i].Key = NewKey;
				}
				else Actions[i].Key = NewKey;
				Keys[Index] = NewKey;
				InputButtons[Index]->NavText->SetText(GetKeyText(Index));
				bFound = true;
				break;
			}
		}
	}
	if (!bFound)
	{
		if (IS_AXIS)
		{
			FKey AxisKey = NewAxisKey.IsValid() ? NewAxisKey : NewKey;
			FInputAxisKeyMapping Axis;
			Axis.Key = AxisKey;
			Axis.AxisName = InputName;
			Axis.Scale = AxisType == EAxisType::Positive ? 1.0f : -1.0f;
			Settings->AddAxisMapping(Axis, true);
			Keys[Index] = AxisKey;
			InputButtons[Index]->NavText->SetText(GetKeyText(Index));
		}
		else
		{
			FInputActionKeyMapping Action = FInputActionKeyMapping();
			Action.ActionName = InputName;
			Action.Key = NewKey;
			Settings->AddActionMapping(Action, true);
			Keys[Index] = NewKey;
			InputButtons[Index]->NavText->SetText(GetKeyText(Index));
		}
	}

	if (Container->UINavPC != nullptr)
	{
		Container->UINavPC->PressedActions.Empty();
		if (Container->UINavPC->KeyMap.Contains(InputName.ToString()))
		{
			Container->UINavPC->KeyMap.Add(InputName.ToString(), Keys);
		}
		Container->UINavPC->UnbindMouseWorkaround();
	}

	UpdateKeyDisplay(Index);

	Settings->SaveConfig();
	Settings->ForceRebuildKeymaps();

	if (bRemoved2DAxis)
	{
		Container->ResetInputBox(InputName, GET_REVERSE_AXIS);
	}
}

FKey UUINavInputBox::GetKeyFromAxis(FKey AxisKey)
{
	FKey NewKey = Container->UINavPC->GetKeyFromAxis(AxisKey, AxisType == EAxisType::Positive);
	if (!NewKey.IsValid())
	{
		NewKey = AxisKey;
	}
	return NewKey;
}

void UUINavInputBox::ProcessInputName()
{
	FString InputNameString = InputName.ToString();
	FString LastNameChar = InputNameString.Right(1);
	if (LastNameChar.Equals(TEXT("+")))
	{
		AxisType = EAxisType::Positive;
		InputNameString.RemoveAt(InputNameString.Len() - 1);
		InputName = FName(*InputNameString);
	}
	else if (LastNameChar.Equals(TEXT("-")))
	{
		AxisType = EAxisType::Negative;
		InputNameString.RemoveAt(InputNameString.Len() - 1);
		InputName = FName(*InputNameString);
	}

	if (InputData.InputText.IsEmpty())
	{
		InputData.InputText = FText::FromName(InputName);
	}
	InputText->SetText(InputData.InputText);
}

bool UUINavInputBox::UpdateKeyIconForKey(int Index)
{
	UTexture2D* NewTexture = Container->UINavPC->GetKeyIcon(Keys[Index]);
	if (NewTexture != nullptr)
	{
		InputButtons[Index]->InputImage->SetBrushFromTexture(NewTexture);
		return true;
	}
	return false;
}

FText UUINavInputBox::GetKeyText(int Index)
{
	FKey Key = Keys[Index];
	return Container->UINavPC->GetKeyText(Key);
}

void UUINavInputBox::UpdateKeyDisplay(int Index)
{
	bUsingKeyImage[Index] = UpdateKeyIconForKey(Index);
	if (bUsingKeyImage[Index])
	{
		InputButtons[Index]->InputImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		InputButtons[Index]->NavText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UUINavInputBox::RevertToKeyText(int Index)
{
	FText OldName;
	if (Index < KeysPerInput && !(Keys[Index].GetFName().IsEqual(FName("None"))))
	{
		OldName = GetKeyText(Index);
		UpdateKeyDisplay(Index);
	}
	else
	{
		OldName = Container->EmptyKeyText;
	}

	InputButtons[Index]->NavText->SetText(OldName);

	if (Container->UINavPC != nullptr)
	{
		Container->UINavPC->PressedActions.Empty();
		Container->UINavPC->UnbindMouseWorkaround();
	}
}

void UUINavInputBox::NotifySelected(int Index)
{
	InputButtons[Index]->NavText->SetText(Container->PressKeyText);

	if (Container->UINavPC != nullptr) Container->UINavPC->BindMouseWorkaround();

	if (bUsingKeyImage[Index])
	{
		InputButtons[Index]->InputImage->SetVisibility(ESlateVisibility::Collapsed);
		InputButtons[Index]->NavText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

bool UUINavInputBox::ContainsKey(FKey CompareKey) const
{
	return Keys.Contains(CompareKey);
}

