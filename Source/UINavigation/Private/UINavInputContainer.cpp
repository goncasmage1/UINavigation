// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavInputContainer.h"
#include "UINavInputBox.h"
#include "UINavController.h"
#include "GameFramework/InputSettings.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PanelWidget.h"
#include "Blueprint/WidgetTree.h"

void UUINavInputContainer::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (InputBox_BP == nullptr) return;

	AUINavController* PC = Cast<AUINavController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;
	TArray<FKey> ActionKeys;
	TArray<FName> FoundNames;

	for (FInputActionKeyMapping& Action : Actions)
	{
		FName NewName = Action.ActionName;

		if (FoundNames.Find(NewName) != INDEX_NONE) continue;
		FoundNames.Add(NewName);

		ActionKeys.Add(Action.Key);

		UUINavInputBox* NewInputBox = CreateWidget<UUINavInputBox>(PC, InputBox_BP);
		NewInputBox->ActionName = NewName.ToString();
		NewInputBox->AddToViewport();
		Panel->AddChild(NewInputBox);
	}
}
