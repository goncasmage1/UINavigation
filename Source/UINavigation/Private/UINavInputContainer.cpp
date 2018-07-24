// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavInputContainer.h"
#include "UINavWidget.h"
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

	for (int i = 0; i < Actions.Num(); ++i)
	{
		if (ActionIndexes.Find(i) == INDEX_NONE) continue;

		FName NewName = Actions[i].ActionName;
		ActionKeys.Add(Actions[i].Key);

		UUINavInputBox* NewInputBox = CreateWidget<UUINavInputBox>(PC, InputBox_BP);
		if (NewInputBox == nullptr) continue;

		ParentWidget->UINavButtons[StartingIndex + i * 2] = NewInputBox->NavButton;
		ParentWidget->UINavButtons[StartingIndex + i * 2 + 1] = NewInputBox->NavButton2;
		ParentWidget->UINavInputBoxes.Add(NewInputBox);
		NewInputBox->ActionName = NewName.ToString();
		NewInputBox->AddToViewport();
		Panel->AddChild(NewInputBox);
	}
}

void UUINavInputContainer::SetParentWidget(UUINavWidget * NewParent)
{
	ParentWidget = NewParent;

	CreateInputBoxes();
}
