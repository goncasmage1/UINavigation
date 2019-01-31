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
	TArray<FInputAxisKeyMapping> TempAxes;

	InputText->SetText(FText::FromString(InputName));

	if (bIsAxis) Settings->GetAxisMappingByName(FName(*InputName), TempAxes);
	else Settings->GetActionMappingByName(FName(*InputName), TempActions);

	AUINavController* PC = Cast<AUINavController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	if ((bIsAxis && TempAxes.Num() == 0) || !bIsAxis && TempActions.Num() == 0)
	{
		DISPLAYERROR(TEXT("Couldn't find Input with given name."));
		return;
	}

	for (int i = 0; i < 3; i++)
	{
		UUINavInputComponent* NewInputButton = i == 0 ? InputButton1 : (i == 1 ? InputButton2 : InputButton3);
		InputButtons.Add(NewInputButton);

		if (i < InputsPerAction)
		{
			if ((bIsAxis && i < TempAxes.Num()) || (!bIsAxis && i < TempActions.Num()))
			{
				Keys.Add(bIsAxis ? TempAxes[TempAxes.Num() - 1 - i].Key : TempActions[TempActions.Num() - 1 - i].Key);

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
				NewInputButton->NavText->SetText(FText::FromName(Container->EmptyKeyName));
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

void UUINavInputBox::UpdateInputKey(FInputActionKeyMapping NewAction, int Index)
{
	if (!ShouldRegisterKey(NewAction.Key, Index))
	{
		RevertToActionText(Index);
		return;
	}

	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;
	TArray<FInputAxisKeyMapping>& Axes = Settings->AxisMappings;

	int Found = 0;
	bool bFound = false;
	for (int i = 0; i < Actions.Num(); i++)
	{
		if ((bIsAxis && Axes[i].AxisName.IsEqual(FName(*InputName))) || (!bIsAxis && Actions[i].ActionName.IsEqual(FName(*InputName))))
		{
			if (Found == Index && Container->RespectsRestriction(NewAction.Key, Index))
			{
				if (bIsAxis) Axes[i].Key = NewAction.Key;
				else Actions[i].Key = NewAction.Key;
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
		if (bIsAxis)
		{
			FInputAxisKeyMapping Axis;
			Axis.Key = NewAction.Key;
			Axis.AxisName = FName(*InputName);
			Settings->AddAxisMapping(Axis, true);
			Keys[Index] = NewAction.Key;
			InputButtons[Index]->NavText->SetText(GetKeyName(Index));
		}
		else
		{
			FInputActionKeyMapping Action = NewAction;
			Action.ActionName = FName(*InputName);
			Settings->AddActionMapping(Action, true);
			Keys[Index] = NewAction.Key;
			InputButtons[Index]->NavText->SetText(GetKeyName(Index));
		}
	}

	AUINavController* UINavPC = Cast<AUINavController>(GetOwningPlayer());
	if (UINavPC != nullptr) UINavPC->PressedActions.Empty();

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
		if (Container->GamepadKeyIconData != nullptr && Container->GamepadKeyIconData->RowMap.Contains(Key.GetFName()))
		{
			KeyIcon = (FInputIconMapping*)Container->GamepadKeyIconData->RowMap[Key.GetFName()];
		}
	}
	else
	{
		if (Container->KeyboardMouseKeyIconData != nullptr && Container->KeyboardMouseKeyIconData->RowMap.Contains(Key.GetFName()))
		{
			KeyIcon = (FInputIconMapping*)Container->KeyboardMouseKeyIconData->RowMap[Key.GetFName()];
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
		if (Container->GamepadKeyNameData != nullptr && Container->GamepadKeyNameData->RowMap.Contains(Key.GetFName()))
		{
			KeyName = (FInputNameMapping*)Container->GamepadKeyNameData->RowMap[Key.GetFName()];
			return FText::FromString(KeyName->InputName);
		}
	}
	else
	{
		if (Container->KeyboardMouseKeyNameData != nullptr && Container->KeyboardMouseKeyNameData->RowMap.Contains(Key.GetFName()))
		{
			KeyName = (FInputNameMapping*)Container->KeyboardMouseKeyNameData->RowMap[Key.GetFName()];
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
		OldName = FText::FromName(Container->EmptyKeyName);
	}

	InputButtons[Index]->NavText->SetText(OldName);

	AUINavController* UINavPC = Cast<AUINavController>(GetOwningPlayer());
	if (UINavPC != nullptr) UINavPC->PressedActions.Empty();
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

