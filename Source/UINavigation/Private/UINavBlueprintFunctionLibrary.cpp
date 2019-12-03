// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavBlueprintFunctionLibrary.h"
#include "Sound/SoundClass.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/InputSettings.h"
#include "UINavSettings.h"
#include "UINavComponent.h"

void UUINavBlueprintFunctionLibrary::SetSoundClassVolume(USoundClass * TargetClass, float NewVolume)
{
	if (TargetClass == nullptr) return;
	TargetClass->Properties.Volume = NewVolume;
}

float UUINavBlueprintFunctionLibrary::GetSoundClassVolume(USoundClass * TargetClass)
{
	if (TargetClass == nullptr) return -1.f;
	return TargetClass->Properties.Volume;
}

void UUINavBlueprintFunctionLibrary::SetPostProcessSettings(FString Variable, FString Value)
{
	if (!GConfig)return;
	FString PostProcess = TEXT("PostProcessQuality@");
	PostProcess.Append(FString::FromInt(GEngine->GameUserSettings->GetPostProcessingQuality()));
	GConfig->SetString(
		*PostProcess,
		*Variable,
		*Value,
		GScalabilityIni
	);
}

FString UUINavBlueprintFunctionLibrary::GetPostProcessSettings(FString Variable)
{
	if (!GConfig) return FString();

	FString ValueReceived;
	FString PostProcess = TEXT("PostProcessQuality@");
	PostProcess.Append(FString::FromInt(GEngine->GameUserSettings->GetPostProcessingQuality()));
	GConfig->GetString(
		*PostProcess,
		*Variable,
		ValueReceived,
		GScalabilityIni
	);
	return ValueReceived;
}

void UUINavBlueprintFunctionLibrary::ResetInputSettings()
{
	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	UUINavSettings *MySettings = GetMutableDefault<UUINavSettings>();

	//Remove old mappings
	TArray<FInputActionKeyMapping> OldActionMappings = Settings->GetActionMappings();
	for (FInputActionKeyMapping Mapping : OldActionMappings)
	{
		Settings->RemoveActionMapping(Mapping, false);
	}
	TArray<FInputAxisKeyMapping> OldAxisMappings = Settings->GetAxisMappings();
	for (FInputAxisKeyMapping Mapping : OldAxisMappings)
	{
		Settings->RemoveAxisMapping(Mapping, false);
	}

	//Add new ones
	for (FInputActionKeyMapping Mapping : MySettings->ActionMappings)
	{
		Settings->AddActionMapping(Mapping, false);
	}
	for (FInputAxisKeyMapping Mapping : MySettings->AxisMappings)
	{
		Settings->AddAxisMapping(Mapping, false);
	}

	Settings->SaveConfig();
	Settings->ForceRebuildKeymaps();
}

bool UUINavBlueprintFunctionLibrary::RespectsRestriction(FKey Key, EInputRestriction Restriction)
{
	switch (Restriction)
	{
		case EInputRestriction::None:
			return true;
			break;
		case EInputRestriction::Keyboard:
			return (!Key.IsMouseButton() && !Key.IsGamepadKey());
			break;
		case EInputRestriction::Mouse:
			return Key.IsMouseButton();
			break;
		case EInputRestriction::Keyboard_Mouse:
			return !Key.IsGamepadKey();
			break;
		case EInputRestriction::Gamepad:
			return Key.IsGamepadKey();
			break;
	}

	return false;
}

bool UUINavBlueprintFunctionLibrary::IsGamepadConnected()
{
	return FSlateApplication::Get().IsGamepadAttached();
}

int UUINavBlueprintFunctionLibrary::GetGridDimension(const FGrid Grid)
{
	switch (Grid.GridType)
	{
		case EGridType::Horizontal:
			return Grid.DimensionX;
			break;
		case EGridType::Vertical:
			return Grid.DimensionY;
			break;
		case EGridType::Grid2D:
			return Grid.NumGrid2DButtons;
			break;
	}
	return 0;
}

bool UUINavBlueprintFunctionLibrary::IsButtonValid(UUINavButton * Button)
{
	if (Button == nullptr) return false;
	return Button->IsValid();
}

int UUINavBlueprintFunctionLibrary::Conv_UINavButtonToInt(UUINavButton * Button)
{
	return Button->ButtonIndex;
}

int UUINavBlueprintFunctionLibrary::Conv_UINavComponentToInt(UUINavComponent * Component)
{
	return Component->ComponentIndex;
}

UUINavButton* UUINavBlueprintFunctionLibrary::Conv_UINavComponentToUINavButton(UUINavComponent * Component)
{
	return Component->NavButton;
}

int UUINavBlueprintFunctionLibrary::Conv_GridToInt(FGrid Grid)
{
	return Grid.GridIndex;
}
