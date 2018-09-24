// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavInputBox.h"
#include "UINavController.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"

void UUINavInputBox::NativeConstruct()
{
	Super::NativeConstruct();

	const UInputSettings* Settings = GetDefault<UInputSettings>();
	TArray<FInputActionKeyMapping> Actions;

	ActionText->SetText(FText::FromString(ActionName));

	Settings->GetActionMappingByName(FName(*ActionName), Actions);

	AUINavController* PC = Cast<AUINavController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	if (Actions.Num() == 0)
	{
		DISPLAYERROR(TEXT("Couldn't find Action with given name."));
		return;
	}

	for (int i = 0; i < InputsPerAction; i++)
	{
		UUINavComponent* NewInputButton = CreateWidget<UUINavComponent>(PC, InputButton_BP);

		if (i < Actions.Num())
		{
			FKey NewKey = Actions[i].Key;
			Keys.Add(NewKey);
			NewInputButton->NavText->SetText(NewKey.GetDisplayName());
		}
		else NewInputButton->NavText->SetText(FText::FromName(FName(TEXT("Unbound"))));

		InputButtons.Add(NewInputButton);
		NewInputButton->AddToViewport();
		HorizontalBox->AddChild(NewInputButton);
	}
}

void UUINavInputBox::UpdateActionKey(FKey NewKey, int Index)
{
	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;

	int Found = 0;
	for (FInputActionKeyMapping& Action : Actions)
	{
		FString NewName = Action.ActionName.ToString();
		if (NewName.Compare(ActionName) != 0) continue;

		if (Found == Index)
		{
			Action.Key = NewKey;
			Keys[Index] = NewKey;
			InputButtons[Index]->NavText->SetText(NewKey.GetDisplayName());
		}
		Found++;
	}

	Settings->SaveConfig();
	Settings->ForceRebuildKeymaps();
}

void UUINavInputBox::NotifySelected(int Index)
{
	FName NewName = FName("Press Any Key");

	InputButtons[Index]->NavText->SetText(FText::FromName(NewName));
}

void UUINavInputBox::NotifyUnbound(int Index)
{
	FName NewName = FName("Unbound");

	InputButtons[Index]->NavText->SetText(FText::FromName(NewName));
}




