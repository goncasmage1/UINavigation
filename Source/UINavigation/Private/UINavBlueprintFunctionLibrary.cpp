// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavBlueprintFunctionLibrary.h"
#include "Sound/SoundClass.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/InputSettings.h"
#include "UINavSettings.h"
#include "UINavComponent.h"
#include "UINavMacros.h"
#include "InputAction.h"
#include "EnhancedInputSubsystems.h"
#if IS_VR_PLATFORM
#include "IXRTrackingSystem.h"
#endif

void UUINavBlueprintFunctionLibrary::SetSoundClassVolume(USoundClass * TargetClass, const float NewVolume)
{
	if (TargetClass == nullptr) return;
	TargetClass->Properties.Volume = NewVolume;
}

float UUINavBlueprintFunctionLibrary::GetSoundClassVolume(USoundClass * TargetClass)
{
	if (TargetClass == nullptr) return -1.f;
	return TargetClass->Properties.Volume;
}

void UUINavBlueprintFunctionLibrary::SetPostProcessSettings(const FString Variable, const FString Value)
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

FString UUINavBlueprintFunctionLibrary::GetPostProcessSettings(const FString Variable)
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

void UUINavBlueprintFunctionLibrary::ResetInputSettings(APlayerController* PC)
{
	if (IsValid(PC))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		if (Subsystem != nullptr)
		{
			const UUINavSettings* UINavSettings = GetDefault<UUINavSettings>();
			for (const TPair<TSoftObjectPtr<UInputMappingContext>, TSoftObjectPtr<UInputMappingContext>>& Entry : UINavSettings->DefaultInputContexts)
			{
				UInputMappingContext* CurrentContext = Entry.Key.LoadSynchronous();
				UInputMappingContext* DefaultContext = Entry.Value.LoadSynchronous();
				
				const TArray<FEnhancedActionKeyMapping>& DefaultMappings = DefaultContext->GetMappings();
				if (DefaultMappings.Num() == 0) continue;

				CurrentContext->UnmapAll();

				for (const FEnhancedActionKeyMapping& DefaultMapping : DefaultMappings)
				{
					FEnhancedActionKeyMapping& NewMapping = CurrentContext->MapKey(DefaultMapping.Action, DefaultMapping.Key);
					NewMapping.Modifiers = DefaultMapping.Modifiers;
					NewMapping.Triggers = DefaultMapping.Triggers;
				}
			}

			FModifyContextOptions ModifyOptions;
			ModifyOptions.bForceImmediately = true;
			Subsystem->RequestRebuildControlMappings(ModifyOptions);
			
			return;
		}
	}
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

bool UUINavBlueprintFunctionLibrary::RespectsRestriction(const FKey Key, const EInputRestriction Restriction)
{
#if IS_VR_PLATFORM
	const FString HMD = GEngine->XRSystem != nullptr ? GEngine->XRSystem->GetSystemName().ToString() : TEXT("");
#else
	const FString HMD = TEXT("");
#endif
	
	switch (Restriction)
	{
	case EInputRestriction::None:
		return true;
	case EInputRestriction::Keyboard:
		return (!Key.IsMouseButton() && !Key.IsGamepadKey());
	case EInputRestriction::Mouse:
		return Key.IsMouseButton();
	case EInputRestriction::Keyboard_Mouse:
		return !Key.IsGamepadKey();
	case EInputRestriction::VR:
		if (HMD == "") return false;
		
		if (HMD == "OculusHMD") {
			return IsKeyInCategory(Key, "Oculus");
		}
		if (HMD == "Morpheus") {
			return IsKeyInCategory(Key, "PSMove");
		}
		return false;
	case EInputRestriction::Gamepad:
		return (Key.IsGamepadKey() && !IsVRKey(Key));
	}

	return false;
}

bool UUINavBlueprintFunctionLibrary::IsGamepadConnected()
{
	return FSlateApplication::Get().IsGamepadAttached();
}

bool UUINavBlueprintFunctionLibrary::IsUINavInputAction(const UInputAction* const InputAction)
{
	const UUINavSettings* const UINavSettings = GetDefault<UUINavSettings>();
	const UUINavEnhancedInputActions* const InputActions = UINavSettings->EnhancedInputActions.LoadSynchronous();
		
	return (InputActions == nullptr ||
			InputAction == InputActions->IA_MenuUp ||
			InputAction == InputActions->IA_MenuDown ||
			InputAction == InputActions->IA_MenuLeft ||
			InputAction == InputActions->IA_MenuRight ||
			InputAction == InputActions->IA_MenuSelect ||
			InputAction == InputActions->IA_MenuReturn ||
			InputAction == InputActions->IA_MenuNext ||
			InputAction == InputActions->IA_MenuPrevious);
}

int UUINavBlueprintFunctionLibrary::GetGridDimension(const FGrid Grid)
{
	switch (Grid.GridType)
	{
		case EGridType::Horizontal:
			return Grid.DimensionX;
		case EGridType::Vertical:
			return Grid.DimensionY;
		case EGridType::Grid2D:
			return Grid.NumGrid2DButtons;
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
	return Component->NavButton->ButtonIndex;
}

UUINavButton* UUINavBlueprintFunctionLibrary::Conv_UINavComponentToUINavButton(UUINavComponent * Component)
{
	return Component->NavButton;
}

int UUINavBlueprintFunctionLibrary::Conv_GridToInt(const FGrid Grid)
{
	return Grid.GridIndex;
}

bool UUINavBlueprintFunctionLibrary::IsVRKey(const FKey Key)
{
	return IsKeyInCategory(Key, "Oculus") || IsKeyInCategory(Key, "Vive") ||
		IsKeyInCategory(Key, "MixedReality") || IsKeyInCategory(Key, "Valve") || IsKeyInCategory(Key, "PSMove");
}

bool UUINavBlueprintFunctionLibrary::IsKeyInCategory(const FKey Key, const FString Category)
{
	return Key.ToString().Contains(Category);
}
