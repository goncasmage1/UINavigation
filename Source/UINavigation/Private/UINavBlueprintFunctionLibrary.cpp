// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavBlueprintFunctionLibrary.h"
#include "UINavPCComponent.h"
#include "Sound/SoundClass.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/InputSettings.h"
#include "UINavSettings.h"
#include "UINavDefaultInputSettings.h"
#include "UINavSavedInputSettings.h"
#include "UINavComponent.h"
#include "UINavMacros.h"
#include "Data/PromptData.h"
#include "InputAction.h"
#include "EnhancedInputSubsystems.h"
#include "Data/UINavEnhancedActionKeyMapping.h"
#include "Components/PanelWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Framework/Application/SlateApplication.h"
#if IS_VR_PLATFORM
#include "IXRTrackingSystem.h"
#endif
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "Misc/ConfigCacheIni.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"

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
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
		if (PC->InputComponent->IsA<UEnhancedInputComponent>() && Subsystem != nullptr)
		{
			const UUINavDefaultInputSettings* DefaultUINavInputSettings = GetDefault<UUINavDefaultInputSettings>();
			for (const TPair<TSoftObjectPtr<UInputMappingContext>, FInputMappingArray>& Entry : DefaultUINavInputSettings->DefaultEnhancedInputMappings)
			{
				UInputMappingContext* InputContext = Entry.Key.LoadSynchronous();

				if (InputContext == nullptr)
				{
					continue;
				}
				
				const FInputMappingArray& DefaultMappings = Entry.Value;
				if (DefaultMappings.InputMappings.Num() == 0) continue;

				InputContext->UnmapAll();

				for (const FUINavEnhancedActionKeyMapping& DefaultInputMapping : DefaultMappings.InputMappings)
				{
					FEnhancedActionKeyMapping& NewMapping = InputContext->MapKey(DefaultInputMapping.Action.LoadSynchronous(), DefaultInputMapping.Key);

					TArray<UInputModifier*> InputModifiers;
					for (const TSoftObjectPtr<UInputModifier>& Modifier : DefaultInputMapping.Modifiers)
					{
						InputModifiers.Add(Modifier.LoadSynchronous());
					}
					NewMapping.Modifiers = InputModifiers;

					TArray<UInputTrigger*> InputTriggers;
					for (const TSoftObjectPtr<UInputTrigger>& Trigger : DefaultInputMapping.Triggers)
					{
						InputTriggers.Add(Trigger.LoadSynchronous());
					}
					NewMapping.Triggers = InputTriggers;
				}
			}

			UUINavSavedInputSettings* SavedInputSettings = GetMutableDefault<UUINavSavedInputSettings>();
			SavedInputSettings->SavedEnhancedInputMappings.Reset();
			SavedInputSettings->SaveConfig();

			UUINavPCComponent* UINavPC = PC->FindComponentByClass<UUINavPCComponent>();
			if (IsValid(UINavPC))
			{
				UINavPC->RefreshNavigationKeys();
			}

			Subsystem->RequestRebuildControlMappings();
		}
	}
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

FText UUINavBlueprintFunctionLibrary::ApplyStyleRowToText(const FText& Text, const FString& StyleRowName)
{
	const FString TextString = Text.ToString();
	
	TArray<FString> StringArray;
	TextString.ParseIntoArray(StringArray, TEXT("\n"));
	
	FString FinalString;
	for (int32 i = 0; i < StringArray.Num(); ++i)
	{
		if (i > 0) FinalString += TEXT("\n");
		FinalString += TEXT("<") + StyleRowName + TEXT(">") + StringArray[i].TrimEnd() + TEXT("</>");
	}
	
	return FText::FromString(FinalString);
}

FText UUINavBlueprintFunctionLibrary::GetRawTextFromRichText(const FText& RichText)
{
	FString RichTextStr = RichText.ToString();

	int32 StartIndex;
	if (!RichTextStr.FindChar(TCHAR('>'), StartIndex))
	{
		return RichText;
	}
	RichTextStr = RichTextStr.RightChop(StartIndex + 1);

	int32 EndIndex;
	if (!RichTextStr.FindChar(TCHAR('<'), EndIndex))
	{
		return RichText;
	}
	RichTextStr = RichTextStr.Left(EndIndex);
	return FText::FromString(*RichTextStr);
}

bool UUINavBlueprintFunctionLibrary::IsGamepadConnected()
{
	return FSlateApplication::Get().IsGamepadAttached();
}

bool UUINavBlueprintFunctionLibrary::IsUINavInputAction(const UInputAction* const InputAction)
{
	const UUINavSettings* const UINavSettings = GetDefault<UUINavSettings>();
	const UUINavEnhancedInputActions* const InputActions = UINavSettings->EnhancedInputActions.LoadSynchronous();

	if (InputActions == nullptr) return false;
		
	return (InputAction == InputActions->IA_MenuUp ||
			InputAction == InputActions->IA_MenuDown ||
			InputAction == InputActions->IA_MenuLeft ||
			InputAction == InputActions->IA_MenuRight ||
			InputAction == InputActions->IA_MenuSelect ||
			InputAction == InputActions->IA_MenuReturn ||
			InputAction == InputActions->IA_MenuNext ||
			InputAction == InputActions->IA_MenuPrevious);
}

UPromptDataBinary* UUINavBlueprintFunctionLibrary::CreateBinaryPromptData(const bool bAccept)
{
	UPromptDataBinary* PromptData = NewObject<UPromptDataBinary>();
	if (!IsValid(PromptData))
	{
		return nullptr;
	}
	PromptData->bAccept = bAccept;

	return PromptData;
}

UWidget* UUINavBlueprintFunctionLibrary::GetPanelWidgetChild(const UWidget* const Widget, const int ChildIndex)
{
	const UPanelWidget* const PanelWidget = Cast<UPanelWidget>(Widget);
	if (!IsValid(PanelWidget))
	{
		return nullptr;
	}

	return PanelWidget->GetChildAt(ChildIndex);
}

UWidget* UUINavBlueprintFunctionLibrary::GetUniformGridChild(const UWidget* const Widget, const int Column, const int Row)
{
	const UUniformGridPanel* const GridPanelWidget = Cast<UUniformGridPanel>(Widget);
	if (!IsValid(GridPanelWidget))
	{
		return nullptr;
	}

	for (int i = 0; i < GridPanelWidget->GetChildrenCount(); ++i)
	{
		UWidget* const Child = GridPanelWidget->GetChildAt(i);
		if (!IsValid(Child))
		{
			continue;
		}

		const UUniformGridSlot* const GridSlot = Cast<UUniformGridSlot>(Child->Slot);
		if (!IsValid(GridSlot))
		{
			continue;
		}

		if (GridSlot->GetColumn() == Column && GridSlot->GetRow() == Row)
		{
			return Child;
		}
	}

	return nullptr;
}

UWidget* UUINavBlueprintFunctionLibrary::FindWidgetOfClassesInWidget(UWidget* Widget, const TArray<TSubclassOf<UWidget>>& WidgetClasses)
{
	for (const TSubclassOf<UWidget> WidgetClass : WidgetClasses)
	{
		if (Widget->IsA(WidgetClass))
		{
			return Widget;
		}
	}

	const UPanelWidget* const PanelWidget = Cast<UPanelWidget>(Widget);
	if (!IsValid(PanelWidget))
	{
		return nullptr;
	}

	for (UWidget* ChildWidget : PanelWidget->GetAllChildren())
	{
		UWidget* FoundWidget = FindWidgetOfClassesInWidget(ChildWidget, WidgetClasses);
		if (IsValid(FoundWidget))
		{
			return FoundWidget;
		}
	}

	return nullptr;
}

int UUINavBlueprintFunctionLibrary::GetIndexInPanelWidget(const UWidget* const Widget, TSubclassOf<UPanelWidget> PanelWidgetSubclass)
{
	if (!IsValid(Widget))
	{
		return INDEX_NONE;
	}

	if (!IsValid(PanelWidgetSubclass))
	{
		PanelWidgetSubclass = UPanelWidget::StaticClass();
	}

	const UPanelWidget* const PanelWidget = Widget->GetParent();
	if (!IsValid(PanelWidget))
	{
		return INDEX_NONE;
	}

	if (!PanelWidget->IsA(PanelWidgetSubclass))
	{
		return GetIndexInPanelWidget(PanelWidget, PanelWidgetSubclass);
	}

	return PanelWidget->GetChildIndex(Widget);
}

UPanelWidget* UUINavBlueprintFunctionLibrary::GetParentPanelWidget(const UWidget* const Widget, TSubclassOf<UPanelWidget> PanelWidgetSubclass)
{
	if (!IsValid(Widget))
	{
		return nullptr;
	}

	if (!IsValid(PanelWidgetSubclass))
	{
		PanelWidgetSubclass = UPanelWidget::StaticClass();
	}

	UPanelWidget* const PanelWidget = Widget->GetParent();
	if (!IsValid(PanelWidget))
	{
		return nullptr;
	}


	if (!PanelWidget->IsA(PanelWidgetSubclass))
	{
		return GetParentPanelWidget(PanelWidget, PanelWidgetSubclass);
	}

	return PanelWidget;
}

void UUINavBlueprintFunctionLibrary::GetIndexInUniformGridWidget(const UWidget* const Widget, int& Column, int& Row)
{
	Column = -1;
	Row = -1;

	if (!IsValid(Widget))
	{
		return;
	}

	if (!IsValid(Widget->GetParent()))
	{
		return;
	}

	const UPanelWidget* GridPanelWidget = Cast<UUniformGridPanel>(Widget->GetParent());
	if (!IsValid(GridPanelWidget))
	{
		GridPanelWidget = Cast<UGridPanel>(Widget->GetParent());
		if (!IsValid(GridPanelWidget))
		{
			return GetIndexInUniformGridWidget(GridPanelWidget, Column, Row);
		}
	}

	for (int i = 0; i < GridPanelWidget->GetChildrenCount(); ++i)
	{
		UWidget* const Child = GridPanelWidget->GetChildAt(i);
		if (!IsValid(Child))
		{
			continue;
		}

		const UUniformGridSlot* const UniformGridSlot = Cast<UUniformGridSlot>(Child->Slot);
		if (IsValid(UniformGridSlot))
		{
			if (Child == Widget)
			{
				Column = UniformGridSlot->GetColumn();
				Row = UniformGridSlot->GetRow();
				return;
			}
		}

		const UGridSlot* const GridSlot = Cast<UGridSlot>(Child->Slot);
		if (IsValid(GridSlot))
		{
			if (Child == Widget)
			{
				Column = GridSlot->GetColumn();
				Row = GridSlot->GetRow();
				return;
			}
		}
	}
}

UWidget* UUINavBlueprintFunctionLibrary::GetFirstWidgetInUserWidget(const UUserWidget* const UserWidget)
{
	if (!IsValid(UserWidget))
	{
		return nullptr;
	}

	return UserWidget->WidgetTree->RootWidget;
}

const UUINavSettings* UUINavBlueprintFunctionLibrary::GetUINavSettings()
{
	return GetDefault<UUINavSettings>();
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
