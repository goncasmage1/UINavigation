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

void UUINavInputBox::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = false;

	BuildKeyMappings();
}

void UUINavInputBox::BuildKeyMappings()
{
	const UInputSettings* Settings = GetDefault<UInputSettings>();
	TArray<FInputActionKeyMapping> TempActions;
	TArray<FInputAxisKeyMapping> TempAxes;

	InputButtons = {
		InputButton1,
		InputButton2,
		InputButton3
	};

	FString LastNameChar = InputNameTuple.Key.ToString().Right(1);
	if (LastNameChar.Equals(TEXT("+")))
	{
		AxisType = EAxisType::Positive;
		FString NewName = InputNameTuple.Key.ToString();
		NewName.RemoveAt(NewName.Len() - 1);
		InputNameTuple.Key = FName(*NewName);
	}
	else if (LastNameChar.Equals(TEXT("-")))
	{
		AxisType = EAxisType::Negative;
		FString NewName = InputNameTuple.Key.ToString();
		NewName.RemoveAt(NewName.Len() - 1);
		InputNameTuple.Key = FName(*NewName);
	}

	if (InputNameTuple.Value.ToString().IsEmpty())
	{
		InputNameTuple.Value = FText::FromName(InputNameTuple.Key);
	}
	InputText->SetText(InputNameTuple.Value);

	if (IS_AXIS) Settings->GetAxisMappingByName(InputNameTuple.Key, TempAxes);
	else Settings->GetActionMappingByName(InputNameTuple.Key, TempActions);

	if ((IS_AXIS && TempAxes.Num() == 0) || (!IS_AXIS && TempActions.Num() == 0))
	{
		FString Message = TEXT("Couldn't find Input with name ");
		Message.Append(*InputNameTuple.Key.ToString());
		DISPLAYERROR(Message);
		return;
	}

	for (int j = 0; j < 3; j++)
	{
		UUINavInputComponent* NewInputButton = InputButtons[j];

		if (j < KeysPerInput)
		{
			for (int i = 0; i < 3; i++)
			{
				if (j + 1 <= Keys.Num()) break;

				if (i < KeysPerInput)
				{
					if ((IS_AXIS && i < TempAxes.Num()) || (!IS_AXIS && i < TempActions.Num()))
					{
						FKey NewKey = IS_AXIS ? TempAxes[i].Key : TempActions[i].Key;

						if (Keys.Contains(NewKey) || !ShouldRegisterKey(NewKey, j)) continue;

						Keys.Add(NewKey);

						if (UpdateKeyIconForKey(j))
						{
							bUsingKeyImage[j] = true;
							NewInputButton->InputImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
							NewInputButton->NavText->SetVisibility(ESlateVisibility::Collapsed);
						}
						NewInputButton->NavText->SetText(GetKeyName(j));
					}
				}
				else if (Keys.Num() >= KeysPerInput)
				{
					NewInputButton->SetVisibility(Container->bCollapseInputBox ? ESlateVisibility::Collapsed : ESlateVisibility::Hidden);
				}
			}
		}
		else
		{
			NewInputButton->SetVisibility(Container->bCollapseInputBox ? ESlateVisibility::Collapsed : ESlateVisibility::Hidden);
		}

		if (Keys.Num() - 1 < j)
		{
			NewInputButton->NavText->SetText(FText::FromName(Container->EmptyKeyName));
			NewInputButton->InputImage->SetVisibility(ESlateVisibility::Collapsed);
			NewInputButton->NavText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			Keys.Add(FKey());
		}
	}
}

void UUINavInputBox::ResetKeyMappings()
{
	Keys.Empty();
	bUsingKeyImage = { false, false, false };
	InputButtons.Empty();
	BuildKeyMappings();

	if (Container->UINavPC != nullptr && Container->UINavPC->KeyMap.Contains(InputNameTuple.Key.ToString()))
	{
		Container->UINavPC->KeyMap.Add(InputNameTuple.Key.ToString(), Keys);
	}
}

void UUINavInputBox::UpdateInputKey(FKey NewKey, int Index)
{
	if (!ShouldRegisterKey(NewKey, Index))
	{
		RevertToActionText(Index);
		return;
	}

	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;
	TArray<FInputAxisKeyMapping>& Axes = Settings->AxisMappings;

	int Found = 0;
	bool bFound = false;
	int Iterations = IS_AXIS ? Axes.Num() : Actions.Num();
	for (int i = 0; i < Iterations; i++)
	{
		if ((IS_AXIS && Axes[i].AxisName.IsEqual(InputNameTuple.Key)) || (!IS_AXIS && Actions[i].ActionName.IsEqual(InputNameTuple.Key)))
		{
			if (Found == Index && Container->RespectsRestriction(NewKey, Index))
			{
				if (IS_AXIS) Axes[i].Key = NewKey;
				else Actions[i].Key = NewKey;
				Keys[Index] = NewKey;
				InputButtons[Index]->NavText->SetText(GetKeyName(Index));
				bFound = true;
				break;
			}
			Found++;
		}
	}
	if (!bFound)
	{
		if (IS_AXIS)
		{
			FInputAxisKeyMapping Axis;
			Axis.Key = NewKey;
			Axis.AxisName = InputNameTuple.Key;
			Settings->AddAxisMapping(Axis, true);
			Keys[Index] = NewKey;
			InputButtons[Index]->NavText->SetText(GetKeyName(Index));
		}
		else
		{
			FInputActionKeyMapping Action = FInputActionKeyMapping();
			Action.ActionName = InputNameTuple.Key;
			Action.Key = NewKey;
			Settings->AddActionMapping(Action, true);
			Keys[Index] = NewKey;
			InputButtons[Index]->NavText->SetText(GetKeyName(Index));
		}
	}

	if (Container->UINavPC != nullptr)
	{
		Container->UINavPC->PressedActions.Empty();
		if (Container->UINavPC->KeyMap.Contains(InputNameTuple.Key.ToString()))
		{
			Container->UINavPC->KeyMap.Add(InputNameTuple.Key.ToString(), Keys);
		}
		Container->UINavPC->UnbindMouseWorkaround();
	}

	CheckKeyIcon(Index);

	Settings->SaveConfig();
	Settings->ForceRebuildKeymaps();
}

bool UUINavInputBox::ShouldRegisterKey(FKey NewKey, int Index) const
{
	ERevertRebindReason RevertReason = ERevertRebindReason::UsedBySameActionGroup;
	if (Container->IsKeyBeingUsed(NewKey))
	{
	}
	else if (Container->Blacklist.Contains(NewKey))
	{
		RevertReason = ERevertRebindReason::BlacklistedKey;
	}
	else if (Container->RespectsRestriction(NewKey, Index))
	{
		RevertReason = ERevertRebindReason::RestrictionMismatch;
	}
	else
	{
		return true;
	}
	
	Container->OnRebindCancelled(RevertReason, NewKey);
	return false;
}

bool UUINavInputBox::UpdateKeyIconForKey(int Index)
{
	FKey Key = Keys[Index];
	FInputIconMapping* KeyIcon = nullptr;

	if (Key.IsGamepadKey())
	{
		if (Container->UINavPC->GamepadKeyIconData != nullptr && Container->UINavPC->GamepadKeyIconData->GetRowMap().Contains(Key.GetFName()))
		{
			KeyIcon = (FInputIconMapping*)Container->UINavPC->GamepadKeyIconData->GetRowMap()[Key.GetFName()];
		}
	}
	else
	{
		if (Container->UINavPC->KeyboardMouseKeyIconData != nullptr && Container->UINavPC->KeyboardMouseKeyIconData->GetRowMap().Contains(Key.GetFName()))
		{
			KeyIcon = (FInputIconMapping*)Container->UINavPC->KeyboardMouseKeyIconData->GetRowMap()[Key.GetFName()];
		}
	}
	if (KeyIcon == nullptr || KeyIcon->InputIcon == nullptr) return false;

	UTexture2D* NewTexture = KeyIcon->InputIcon.LoadSynchronous();
	if (NewTexture != nullptr)
	{
		InputButtons[Index]->InputImage->SetBrushFromTexture(NewTexture);
		return true;
	}
	return false;
}

FText UUINavInputBox::GetKeyName(int Index)
{
	FKey Key = Keys[Index];
	FInputNameMapping* KeyName = nullptr;

	if (Key.IsGamepadKey())
	{
		if (Container->UINavPC->GamepadKeyNameData != nullptr && Container->UINavPC->GamepadKeyNameData->GetRowMap().Contains(Key.GetFName()))
		{
			KeyName = (FInputNameMapping*)Container->UINavPC->GamepadKeyNameData->GetRowMap()[Key.GetFName()];
			return FText::FromString(KeyName->InputName);
		}
	}
	else
	{
		if (Container->UINavPC->KeyboardMouseKeyNameData != nullptr && Container->UINavPC->KeyboardMouseKeyNameData->GetRowMap().Contains(Key.GetFName()))
		{
			KeyName = (FInputNameMapping*)Container->UINavPC->KeyboardMouseKeyNameData->GetRowMap()[Key.GetFName()];
			return FText::FromString(KeyName->InputName);
		}
	}
	return Key.GetDisplayName();
}

void UUINavInputBox::CheckKeyIcon(int Index)
{
	if (UpdateKeyIconForKey(Index) != bUsingKeyImage[Index]) bUsingKeyImage[Index] = !bUsingKeyImage[Index];

	if (bUsingKeyImage[Index])
	{
		InputButtons[Index]->InputImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		InputButtons[Index]->NavText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UUINavInputBox::RevertToActionText(int Index)
{
	FText OldName;
	if (Index < KeysPerInput && !(Keys[Index].GetFName().IsEqual(FName("None"))))
	{
		OldName = GetKeyName(Index);
		CheckKeyIcon(Index);
	}
	else
	{
		OldName = FText::FromName(Container->EmptyKeyName);
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
	FName NewName = FName("Press Any Key");

	InputButtons[Index]->NavText->SetText(FText::FromName(NewName));

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

