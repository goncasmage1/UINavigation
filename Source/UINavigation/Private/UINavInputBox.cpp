// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavInputBox.h"
#include "UINavController.h"
#include "UINavInputComponent.h"
#include "UINavInputContainer.h"
#include "UINavSettings.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Engine/DataTable.h"
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

	ActionText->SetText(FText::FromString(ActionName));

	Settings->GetActionMappingByName(FName(*ActionName), TempActions);

	AUINavController* PC = Cast<AUINavController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	if (TempActions.Num() == 0)
	{
		DISPLAYERROR(TEXT("Couldn't find Action with given name."));
		return;
	}

	for (int i = 0; i < 3; i++)
	{
		UUINavInputComponent* NewInputButton = i == 0 ? InputButton1 : (i == 1 ? InputButton2 : InputButton3);
		InputButtons.Add(NewInputButton);

		if (i < InputsPerAction)
		{
			if (i < TempActions.Num())
			{
				FInputActionKeyMapping NewAction = TempActions[TempActions.Num() - 1 - i];
				Keys.Add(NewAction.Key);

				if (UpdateKeyIconForKey(i))
				{
					bUsingKeyImage[i] = true;
					NewInputButton->InputImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
					NewInputButton->NavText->SetVisibility(ESlateVisibility::Collapsed);
				}
				NewInputButton->NavText->SetText(GetKeyName(i));
			}
			else
			{
				NewInputButton->NavText->SetText(FText::FromName(FName(TEXT("Unbound"))));
				NewInputButton->InputImage->SetVisibility(ESlateVisibility::Collapsed);
				NewInputButton->NavText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
				Keys.Add(FKey());
			}
		}
		else
		{
			NewInputButton->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UUINavInputBox::ResetKeyMappings()
{
	Keys.Empty();
	bUsingKeyImage = { false, false, false };
	BuildKeyMappings();
}

void UUINavInputBox::UpdateActionKey(FInputActionKeyMapping NewAction, int Index)
{
	if (!ShouldRegisterKey(NewAction.Key, Index))
	{
		RevertToActionText(Index);
		return;
	}

	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;

	int Found = 0;
	bool bFound = false;
	for (int i = 0; i < Actions.Num(); i++)
	{
		if (Actions[i].ActionName.IsEqual(FName(*ActionName)))
		{
			if (Found == Index && Container->RespectsRestriction(NewAction.Key, Index))
			{
				Actions[i].Key = NewAction.Key;
				Keys[Index] = NewAction.Key;
				InputButtons[Index]->NavText->SetText(GetKeyName(Index));
				bFound = true;
				break;
			}
			Found++;
		}
	}
	if (!bFound)
	{
		FInputActionKeyMapping Action = NewAction;
		Action.ActionName = FName(*ActionName);
		Settings->AddActionMapping(Action, true);
		Keys[Index] = NewAction.Key;
		InputButtons[Index]->NavText->SetText(GetKeyName(Index));
	}

	CheckKeyIcon(Index);

	Settings->SaveConfig();
	Settings->ForceRebuildKeymaps();
}

bool UUINavInputBox::ShouldRegisterKey(FKey NewKey, int Index) const
{
	if (Container->IsKeyBeingUsed(NewKey)) return false;
	else return Container->RespectsRestriction(NewKey, Index);
}

bool UUINavInputBox::UpdateKeyIconForKey(int Index)
{
	FKey Key = Keys[Index];
	FInputIconMapping* KeyIcon = nullptr;

	if (Key.IsGamepadKey())
	{
		if (Container->GamepadKeyIconData != nullptr && Container->GamepadKeyIconData->GetRowMap().Contains(Key.GetFName()))
		{
			KeyIcon = (FInputIconMapping*)Container->GamepadKeyIconData->GetRowMap()[Key.GetFName()];
		}
	}
	else
	{
		if (Container->KeyboardMouseKeyIconData != nullptr && Container->KeyboardMouseKeyIconData->GetRowMap().Contains(Key.GetFName()))
		{
			KeyIcon = (FInputIconMapping*)Container->KeyboardMouseKeyIconData->GetRowMap()[Key.GetFName()];
		}
	}
	if (KeyIcon == nullptr) return false;

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
		if (Container->GamepadKeyNameData != nullptr && Container->GamepadKeyNameData->GetRowMap().Contains(Key.GetFName()))
		{
			KeyName = (FInputNameMapping*)Container->GamepadKeyNameData->GetRowMap()[Key.GetFName()];
			return FText::FromString(KeyName->InputName);
		}
	}
	else
	{
		if (Container->KeyboardMouseKeyNameData != nullptr && Container->KeyboardMouseKeyNameData->GetRowMap().Contains(Key.GetFName()))
		{
			KeyName = (FInputNameMapping*)Container->KeyboardMouseKeyNameData->GetRowMap()[Key.GetFName()];
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
	if (Index < InputsPerAction && !(Keys[Index].GetFName().IsEqual(FName("None"))))
	{
		OldName = GetKeyName(Index);
		CheckKeyIcon(Index);
	}
	else
	{
		OldName = FText::FromName(FName(TEXT("Unbound")));
	}

	InputButtons[Index]->NavText->SetText(OldName);
}

void UUINavInputBox::NotifySelected(int Index)
{
	FName NewName = FName("Press Any Key");

	InputButtons[Index]->NavText->SetText(FText::FromName(NewName));

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
