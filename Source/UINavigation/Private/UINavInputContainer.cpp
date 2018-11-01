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
#include "Kismet/GameplayStatics.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

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

void UUINavInputContainer::ResetKeyMappings()
{
	for (UUINavInputBox* InputBox : ParentWidget->UINavInputBoxes) InputBox->ResetKeyMappings();
}

void UUINavInputContainer::CreateInputBoxes()
{
	if (InputBox_BP == nullptr) return;

	Panel->ClearChildren();

	AUINavController* PC = Cast<AUINavController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;
	TArray<FKey> ActionKeys;
	TArray<FName> FoundActions;

	//TODO: Optimize action finding in UINavInputBox

	bool bUseActionNames = ActionNames.Num() > 0;
	int Iterations = bUseActionNames ? ActionNames.Num() : Actions.Num();
	{
		ParentWidget->InputBoxStartIndex = ParentWidget->UINavButtons.Num();
		for (int i = 0; i < Iterations; ++i)
		{
			if (!bUseActionNames)
			{
				if (FoundActions.Contains(Actions[i].ActionName)) continue;
				else FoundActions.Add(Actions[i].ActionName);
			}

			for (int k = 0; k < InputsPerAction; k++) ParentWidget->UINavButtons.Add(nullptr);
			ActionKeys.Add(Actions[i].Key);
			UUINavInputBox* NewInputBox = CreateWidget<UUINavInputBox>(PC, InputBox_BP);
			if (NewInputBox == nullptr) continue;

			NewInputBox->Container = this;
			NewInputBox->InputsPerAction = InputsPerAction;
			NewInputBox->ActionName = bUseActionNames ? ActionNames[i].ToString() : Actions[i].ActionName.ToString();
			Panel->AddChild(NewInputBox);

			ParentWidget->UINavInputBoxes.Add(NewInputBox);

			for (int j = 0; j < InputsPerAction; j++)
			{
				ParentWidget->UINavButtons[StartingIndex + (bUseActionNames ? i : FoundActions.Num() - 1) * InputsPerAction + j] = NewInputBox->InputButtons[j]->NavButton;
				if (!ParentWidget->bOverrideButtonIndices)
				{
					NewInputBox->InputButtons[j]->NavButton->ButtonIndex = StartingIndex + i * InputsPerAction + j;
				}
				ParentWidget->SetupUINavButtonDelegates(NewInputBox->InputButtons[j]->NavButton);
			}
		}
		ParentWidget->InputBoxEndIndex = ParentWidget->UINavButtons.Num() - 1;
	}
}

UTexture2D* UUINavInputContainer::LoadTexture2D(const FString& FullFilePath, bool& IsValid, int32& Width, int32& Height)
{
	IsValid = false;
	UTexture2D* LoadedT2D = NULL;

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	//Load From File
	TArray<uint8> RawFileData;
	if (!FFileHelper::LoadFileToArray(RawFileData, *FullFilePath)) return NULL;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//Create T2D!
	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
	{
		const TArray<uint8>* UncompressedBGRA = NULL;
		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
		{
			LoadedT2D = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);

			//Valid?
			if (!LoadedT2D) return NULL;
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