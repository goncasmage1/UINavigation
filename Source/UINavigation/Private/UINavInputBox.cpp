// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavInputBox.h"
#include "UINavController.h"
#include "Components/TextBlock.h"

void UUINavInputBox::NativeConstruct()
{
	Super::NativeConstruct();

	const UInputSettings* Settings = GetDefault<UInputSettings>();
	const TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;

	ActionText->SetText(FText::FromString(ActionName));

	int Found = 0;
	for (FInputActionKeyMapping Action : Actions)
	{
		FString NewName = Action.ActionName.ToString();
		if (NewName.Compare(ActionName) != 0) continue;
		
		if (Found > 1) continue;
		
		if (Found > 0)
		{
			Key2 = Action.Key;
			NavText2->SetText(Key2.GetDisplayName());
		}
		else
		{
			Key1 = Action.Key;
			NavText->SetText(Key1.GetDisplayName());
		}
		Found++;
	}
	if (Found == 0)
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

		Found++;

		if ((Found == 1 && !SecondKey) || (Found == 2 && SecondKey))
		{
			Action.Key = NewKey;
			if (SecondKey) Key2 = NewKey;
			else Key1 = NewKey;

			if (SecondKey) NavText2->SetText(Key2.GetDisplayName());
			else NavText->SetText(Key1.GetDisplayName());
		}
	}

	Settings->SaveKeyMappings();

	/*for (TObjectIterator<UPlayerInput> It; It; ++It)
	{
		It->ForceRebuildingKeyMaps(true);
	}*/
}

void UUINavInputBox::NotifySelected(bool SecondKey)
{
	if (SecondKey) NavText2->SetText(FText::FromName(FName("Press Any Key")));
	else NavText->SetText(FText::FromName(FName("Press Any Key")));
}



