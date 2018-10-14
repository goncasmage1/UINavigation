// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavInputBox.h"
#include "UINavController.h"
#include "UINavComponent.h"
#include "UINavSettings.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"

void UUINavInputBox::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;

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
		UUINavComponent* NewInputButton = i == 0 ? InputButton1 : (i == 1 ? InputButton2 : InputButton3);
		InputButtons.Add(NewInputButton);

		if (i < InputsPerAction)
		{
			if (i < TempActions.Num())
			{
				FInputActionKeyMapping NewAction = TempActions[TempActions.Num() - 1 - i];
				Actions.Add(NewAction);

				FString NewActionName = FString();
				if (NewAction.bShift) NewActionName.Append(TEXT("Shift +"));
				if (NewAction.bAlt) NewActionName.Append(TEXT("Alt +"));
				if (NewAction.bCtrl) NewActionName.Append(TEXT("Ctrl +"));
				if (NewAction.bCmd) NewActionName.Append(TEXT("Cmd +"));
				NewActionName.Append(NewAction.Key.GetDisplayName().ToString());
				NewInputButton->NavText->SetText(FText::FromString(*NewActionName));
			}
			else NewInputButton->NavText->SetText(FText::FromName(FName(TEXT("Unbound"))));
		}
		else
		{
			NewInputButton->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UUINavInputBox::ResetKeyMappings()
{
	Actions.Empty();
	BuildKeyMappings();
}

void UUINavInputBox::UpdateActionKey(FInputActionKeyMapping NewAction, int Index)
{
	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	TArray<FInputActionKeyMapping> TempActions;
	Settings->GetActionMappingByName(FName(*ActionName), TempActions);

	if (Index < Actions.Num())
	{
		int Found = 0;

		for (FInputActionKeyMapping& Action : TempActions)
		{
			if (Found == Index)
			{
				Action = NewAction;
				Action.ActionName = FName(*ActionName);
				Actions[Index] = NewAction;
				InputButtons[Index]->NavText->SetText(Action.Key.GetDisplayName());
			}
			Found++;
		}
	}
	else
	{
		FInputActionKeyMapping Action = NewAction;
		Action.ActionName = FName(*ActionName);
		Settings->AddActionMapping(NewAction, true);
		Settings->ActionMappings.Add(Action);
		InputButtons[Index]->NavText->SetText(Action.Key.GetDisplayName());
	}

	Settings->SaveConfig();
	Settings->ForceRebuildKeymaps();
}

void UUINavInputBox::RevertToActionText(int Index)
{ 
	FText OldName = (Index < Actions.Num()) ? Actions[Index].Key.GetDisplayName() : FText::FromName(FName(TEXT("Unbound")));
	InputButtons[Index]->NavText->SetText(OldName);
	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
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




