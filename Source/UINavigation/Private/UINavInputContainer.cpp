// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavInputContainer.h"
#include "UINavWidget.h"
#include "UINavButton.h"
#include "UINavInputBox.h"
#include "UINavController.h"
#include "UINavComponent.h"
#include "UINavInputComponent.h"
#include "UINavBlueprintFunctionLibrary.h"
#include "GameFramework/InputSettings.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "Kismet/GameplayStatics.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

void UUINavInputContainer::SetParentWidget(UUINavWidget * NewParent)
{
	ParentWidget = NewParent;

	bIsFocusable = false;

	if (InputRestrictions.Num() == 0) InputRestrictions.Add(EInputRestriction::None);
	else if (InputRestrictions.Num() > 3) InputRestrictions.SetNum(3);
	InputsPerAction = InputRestrictions.Num();

	CreateInputBoxes();
}

void UUINavInputContainer::ResetKeyMappings()
{
	UUINavBlueprintFunctionLibrary::ResetInputSettings();
	for (UUINavInputBox* InputBox : ParentWidget->UINavInputBoxes) InputBox->ResetKeyMappings();
}

void UUINavInputContainer::CreateInputBoxes()
{
	if (InputBox_BP == nullptr) return;

	FirstButtonIndex = ParentWidget->UINavButtons.Num();

	CreateActionBoxes();
	CreateAxisBoxes();

	LastButtonIndex = ParentWidget->UINavButtons.Num() - 1;

	switch (InputsPerAction)
	{
		case 2:
			TopButtonIndex = FirstButtonIndex + (TargetColumn == ETargetColumn::Right);
			BottomButtonIndex = LastButtonIndex -(TargetColumn != ETargetColumn::Right);
			break;
		case 3:
			TopButtonIndex = FirstButtonIndex + (int)TargetColumn;
			BottomButtonIndex = LastButtonIndex - (2 - (int)TargetColumn);
			break;
	}
}

void UUINavInputContainer::CreateActionBoxes()
{
	AUINavController* PC = Cast<AUINavController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;
	TArray<FName> FoundInputs;

	if (Actions.Num() == 0) return;

	int TempFirstButtonIndex = ParentWidget->UINavButtons.Num();
	int StartingInputComponentIndex = ParentWidget->UINavComponents.Num();

	bool bUseInputNames = ActionNames.Num() > 0;
	int Iterations = bUseInputNames ? ActionNames.Num() : Actions.Num();

	for (int i = 0; i < Iterations; ++i)
	{
		if (!bUseInputNames)
		{
			if (FoundInputs.Contains(Actions[i].ActionName)) continue;
			else FoundInputs.Add(Actions[i].ActionName);
		}

		UUINavInputBox* NewInputBox = CreateWidget<UUINavInputBox>(PC, InputBox_BP);
		if (NewInputBox == nullptr) continue;
		NewInputBox->Container = this;
		NewInputBox->bIsAxis = false;
		NewInputBox->InputsPerAction = InputsPerAction;
		NewInputBox->InputName = bUseInputNames ? ActionNames[i].ToString() : Actions[i].ActionName.ToString();

		ActionPanel->AddChild(NewInputBox);

		ParentWidget->UINavInputBoxes.Add(NewInputBox);
		NumberOfInputs++;

		for (int j = 0; j < InputsPerAction; j++)
		{
			ParentWidget->UINavButtons.Add(nullptr);
			ParentWidget->UINavComponents.Add(nullptr);

			int NewButtonIndex = TempFirstButtonIndex + (bUseInputNames ? i : FoundInputs.Num() - 1) * InputsPerAction + j;
			int NewComponentIndex = StartingInputComponentIndex + (bUseInputNames ? i : FoundInputs.Num() - 1) * InputsPerAction + j;
			ParentWidget->UINavButtons[NewButtonIndex] = NewInputBox->InputButtons[j]->NavButton;
			ParentWidget->UINavComponents[NewComponentIndex] = NewInputBox->InputButtons[j];
			ParentWidget->UINavComponentsIndices.Add(NewButtonIndex);
			if (!ParentWidget->bOverrideButtonIndices)
			{
				NewInputBox->InputButtons[j]->NavButton->ButtonIndex = TempFirstButtonIndex + (bUseInputNames ? i : (FoundInputs.Num() - 1)) * InputsPerAction + j;
			}
			ParentWidget->SetupUINavButtonDelegates(NewInputBox->InputButtons[j]->NavButton);
		}
	} 
}

void UUINavInputContainer::CreateAxisBoxes()
{
	AUINavController* PC = Cast<AUINavController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	TArray<FInputAxisKeyMapping>& Axes = Settings->AxisMappings;
	TArray<FName> FoundInputs;

	if (Axes.Num() == 0) return;

	int TempFirstButtonIndex = ParentWidget->UINavButtons.Num();
	int StartingInputComponentIndex = ParentWidget->UINavComponents.Num();

	bool bUseInputNames = AxisNames.Num() > 0;
	int Iterations = bUseInputNames ? AxisNames.Num() : Axes.Num();

	for (int i = 0; i < Iterations; ++i)
	{
		if (!bUseInputNames)
		{
			if (FoundInputs.Contains(Axes[i].AxisName)) continue;
			else FoundInputs.Add(Axes[i].AxisName);
		}

		UUINavInputBox* NewInputBox = CreateWidget<UUINavInputBox>(PC, InputBox_BP);
		if (NewInputBox == nullptr) continue;
		NewInputBox->Container = this;
		NewInputBox->bIsAxis = true;
		NewInputBox->InputsPerAction = InputsPerAction;
		NewInputBox->InputName = bUseInputNames ? AxisNames[i].ToString() : Axes[i].AxisName.ToString();

		AxisPanel->AddChild(NewInputBox);

		ParentWidget->UINavInputBoxes.Add(NewInputBox);
		NumberOfInputs++;

		for (int j = 0; j < InputsPerAction; j++)
		{
			ParentWidget->UINavButtons.Add(nullptr);
			ParentWidget->UINavComponents.Add(nullptr);

			int NewButtonIndex = TempFirstButtonIndex + (bUseInputNames ? i : FoundInputs.Num() - 1) * InputsPerAction + j;
			int NewComponentIndex = StartingInputComponentIndex + (bUseInputNames ? i : FoundInputs.Num() - 1) * InputsPerAction + j;
			ParentWidget->UINavButtons[NewButtonIndex] = NewInputBox->InputButtons[j]->NavButton;
			ParentWidget->UINavComponents[NewComponentIndex] = NewInputBox->InputButtons[j];
			ParentWidget->UINavComponentsIndices.Add(NewButtonIndex);
			if (!ParentWidget->bOverrideButtonIndices)
			{
				NewInputBox->InputButtons[j]->NavButton->ButtonIndex = TempFirstButtonIndex + (bUseInputNames ? i : (FoundInputs.Num() - 1)) * InputsPerAction + j;
			}
			ParentWidget->SetupUINavButtonDelegates(NewInputBox->InputButtons[j]->NavButton);
		}
	} 
}

bool UUINavInputContainer::IsKeyBeingUsed(FKey CompareKey) const
{
	for (UUINavInputBox* box : ParentWidget->UINavInputBoxes)
	{
		if (box->ContainsKey(CompareKey)) return true;
	}

	return false;
}

bool UUINavInputContainer::RespectsRestriction(FKey CompareKey, int Index)
{
	EInputRestriction Restriction = InputRestrictions[Index];

	switch (Restriction)
	{
		case EInputRestriction::None:
			return true;
			break;
		case EInputRestriction::Keyboard:
			return (!CompareKey.IsMouseButton() && !CompareKey.IsGamepadKey());
			break;
		case EInputRestriction::Mouse:
			return CompareKey.IsMouseButton();
			break;
		case EInputRestriction::Keyboard_Mouse:
			return !CompareKey.IsGamepadKey();
			break;
		case EInputRestriction::Gamepad:
			return CompareKey.IsGamepadKey();
			break;
		default:
			break;
	}

	return false;
}

UTexture2D* UUINavInputContainer::LoadTexture2D(const FString& FullFilePath, bool& IsValid, int32& Width, int32& Height)
{
	IsValid = false;
	UTexture2D* LoadedT2D = nullptr;

	FString ActualPath = FullFilePath + TEXT(".png");

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	//Load From File
	TArray<uint8> RawFileData;
	if (!FFileHelper::LoadFileToArray(RawFileData, *ActualPath)) return nullptr;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//Create T2D!
	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
	{
		const TArray<uint8>* UncompressedBGRA = nullptr;
		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
		{
			LoadedT2D = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);

			//Valid?
			if (!LoadedT2D) return nullptr;
			//~~~~~~~~~~~~~~

			//Out!
			Width = ImageWrapper->GetWidth();
			Height = ImageWrapper->GetHeight();

			//Copy!
			void* TextureData = LoadedT2D->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, UncompressedBGRA->GetData(), UncompressedBGRA->Num());
			LoadedT2D->PlatformData->Mips[0].BulkData.Unlock();

			//Update!
			LoadedT2D->UpdateResource();
		}
	}

	// Success!
	IsValid = true;
	return LoadedT2D;
}

int UUINavInputContainer::GetOffsetFromTargetColumn(bool bTop)
{
	switch (InputsPerAction)
	{
		case 2:
			if (bTop) return (TargetColumn == ETargetColumn::Right);
			else return -(TargetColumn != ETargetColumn::Right);
			break;
		case 3:
			if (bTop) return (int)TargetColumn;
			else return (-2 - (int)TargetColumn);
			break;
	}
	return 0;
}