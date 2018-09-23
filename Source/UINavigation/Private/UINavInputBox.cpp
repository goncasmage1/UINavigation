// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavInputBox.h"
#include "UINavController.h"
#include "Components/TextBlock.h"

void UUINavInputBox::NativeConstruct()
{
	Super::NativeConstruct();

	const UInputSettings* Settings = GetDefault<UInputSettings>();
	TArray<FInputActionKeyMapping> Actions;

	ActionText->SetText(FText::FromString(ActionName));

	Settings->GetActionMappingByName(FName(*ActionName), Actions);

	for (int i = 0; i < Actions.Num(); i++)
	{
		if (i == Actions.Num() - 1)
		{
			Key1 = Actions[Actions.Num() - 1].Key;
			NavText->SetText(Key1.GetDisplayName());
		}
		else
		{
			Key2 = Actions[Actions.Num() - 2].Key;
			NavText2->SetText(Key2.GetDisplayName());
		}
	}

	if (Actions.Num() == 0)
	{
		DISPLAYERROR(TEXT("UINavInputBox: Couldn't find Action with given name."));
		return;
	}
}

void UUINavInputBox::UpdateActionKey(FKey NewKey, bool SecondKey)
{
	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;

	int Found = 0;
	for (FInputActionKeyMapping& Action : Actions)
	{
		FString NewName = Action.ActionName.ToString();
		if (NewName.Compare(ActionName) != 0) continue;

		if ((Found == 0 && !SecondKey) || (Found == 1 && SecondKey))
		{
			Action.Key = NewKey;
			if (SecondKey) Key2 = NewKey;
			else Key1 = NewKey;

			if (SecondKey) NavText2->SetText(Key2.GetDisplayName());
			else NavText->SetText(Key1.GetDisplayName());
		}
		Found++;
	}

	Settings->SaveConfig();
	Settings->ForceRebuildKeymaps();
}

void UUINavInputBox::NotifySelected(bool SecondKey)
{
	if (SecondKey) NavText2->SetText(FText::FromName(FName("Press Any Key")));
	else NavText->SetText(FText::FromName(FName("Press Any Key")));
}



