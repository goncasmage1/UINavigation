// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavInputContainer.h"
#include "UINavWidget.h"
#include "UINavButton.h"
#include "UINavInputBox.h"
#include "UINavController.h"
#include "GameFramework/InputSettings.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "DataTable.h"
#include "Kismet/GameplayStatics.h"

void UUINavInputContainer::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (InputsPerAction < 1 || InputsPerAction > 3) DISPLAYERROR("Inputs Per Action must be between 1 and 3");
}

void UUINavInputContainer::SetParentWidget(UUINavWidget * NewParent)
{
	ParentWidget = NewParent;

	CreateInputBoxes();
}

void UUINavInputContainer::CreateInputBoxes()
{
	if (InputBox_BP == nullptr) return;

	AUINavController* PC = Cast<AUINavController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;
	TArray<FKey> ActionKeys;
	TArray<FName> FoundNames;

	//TODO: Optimize action finding in UINavInputBox

	for (int i = 0; i < ActionNames.Num(); ++i)
	{
		ActionKeys.Add(Actions[i].Key);
		UUINavInputBox* NewInputBox = CreateWidget<UUINavInputBox>(PC, InputBox_BP);
		if (NewInputBox == nullptr) continue;

		NewInputBox->InputsPerAction = InputsPerAction;
		NewInputBox->ActionName = ActionNames[i].ToString();
		Panel->AddChild(NewInputBox);

		ParentWidget->UINavInputBoxes.Add(NewInputBox);

		for (int j = 0; j < InputsPerAction; j++)
		{
			ParentWidget->UINavButtons[StartingIndex + i * InputsPerAction + j] = NewInputBox->InputButtons[j]->NavButton;
			if (!ParentWidget->bOverrideButtonIndices)
			{
				NewInputBox->InputButtons[j]->NavButton->ButtonIndex = StartingIndex + i * InputsPerAction + j;
			}
			ParentWidget->SetupUINavButtonDelegates(NewInputBox->InputButtons[j]->NavButton);
		}

	}
}
