// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavInputBox.h"

#include "EnhancedInputSubsystems.h"
#include "UINavInputComponent.h"
#include "UINavInputContainer.h"
#include "UINavMacros.h"
#include "UINavSettings.h"
#include "UINavPCComponent.h"
#include "UINavWidget.h"
#include "UINavBlueprintFunctionLibrary.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/Image.h"
#include "Data/RevertRebindReason.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "GameFramework/InputSettings.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "PlayerMappableKeySettings.h"
#include "UserSettings/EnhancedInputUserSettings.h"

UUINavInputBox::UUINavInputBox(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SetIsFocusable(false);
}

void UUINavInputBox::NativeConstruct()
{
	Super::NativeConstruct();

	InputButton->OnClicked.RemoveAll(this);
	InputButton->OnClicked.AddDynamic(this, &UUINavInputBox::InputComponentClicked);
;}

void UUINavInputBox::CreateKeyWidgets()
{
	ProcessInputName();

	CreateEnhancedInputKeyWidgets();
}

void UUINavInputBox::CreateEnhancedInputKeyWidgets()
{
	if (!IsValid(Container) || !IsValid(Container->UINavPC) || !IsValid(Container->UINavPC->GetPC()) || !IsValid(Container->UINavPC->GetPC()->GetLocalPlayer()))
	{
		return;
	}
	UEnhancedInputLocalPlayerSubsystem* PlayerSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Container->UINavPC->GetPC()->GetLocalPlayer());
	if (!IsValid(PlayerSubsystem))
	{
		return;
	}
	UEnhancedInputUserSettings* PlayerSettings = PlayerSubsystem->GetUserSettings();
	if (!IsValid(PlayerSettings))
	{
		return;
	}
	if (!PlayerSettings->IsMappingContextRegistered(InputContext))
	{
		PlayerSettings->RegisterInputMappingContext(InputContext);
	}
	const FPlayerKeyMapping* KeyMapping = PlayerSettings->FindCurrentMappingForSlot(PlayerMappableKeySettingsName, EPlayerMappableKeySlot::First);
	if (KeyMapping != nullptr && KeyMapping->GetCurrentKey().IsValid())
	{
		TrySetupNewKey(KeyMapping->GetCurrentKey(), 0, InputButton);
	} else
	{
		InputButton->SetText(Container->EmptyKeyText);
		InputButton->InputImage->SetVisibility(ESlateVisibility::Collapsed);
		if (IsValid(InputButton->NavText)) InputButton->NavText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (IsValid(InputButton->NavRichText)) InputButton->NavRichText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		CurrentKey = FKey();
	}
}

bool UUINavInputBox::TrySetupNewKey(const FKey& NewKey, const int KeyIndex, UUINavInputComponent* const NewInputButton)
{
	if (!NewKey.IsValid()) return false;
	CurrentKey = NewKey;
	
	if (UpdateKeyIconForKey(KeyIndex))
	{
		bUsingKeyImage = true;
		InputButton->InputImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (IsValid(InputButton->NavText)) InputButton->NavText->SetVisibility(ESlateVisibility::Collapsed);
		if (IsValid(InputButton->NavRichText)) InputButton->NavRichText->SetVisibility(ESlateVisibility::Collapsed);
	}
	InputButton->SetText(GetKeyText(KeyIndex));

	return true;
}

void UUINavInputBox::ResetKeyWidgets()
{
	CurrentKey = FKey();
	bUsingKeyImage = false;
	CreateKeyWidgets();
}

int32 UUINavInputBox::UpdateInputKey(const FKey& NewKey, int Index, const bool bSkipChecks, const int32 MappingIndexToIgnore /*= -1*/)
{
	if (Index < 0) Index = AwaitingIndex;

	if (AwaitingIndex < 0) AwaitingIndex = Index;

	AwaitingNewKey = NewKey;
	if (Index < 0)
	{
		CancelUpdateInputKey(ERevertRebindReason::None);
		return -1;
	}

	if (!bSkipChecks)
	{
		int CollidingActionIndex = INDEX_NONE;
		int CollidingKeyIndex = INDEX_NONE;
		const ERevertRebindReason RevertReason = Container->CanRegisterKey(this, NewKey, Index, CollidingActionIndex, CollidingKeyIndex);
		if (RevertReason == ERevertRebindReason::UsedBySameInputGroup)
		{
			if (!CurrentKey.IsValid())
			{
				CancelUpdateInputKey(RevertReason);
				return -1;
			}

			int SelfIndex = INDEX_NONE;

			FInputRebindData CollidingInputData;
			Container->GetEnhancedInputRebindData(CollidingActionIndex, CollidingInputData);
			if (!Container->InputBoxes.Find(this, SelfIndex) ||
				!Container->RequestKeySwap(FInputCollisionData(GetCurrentText(),
					CollidingInputData.InputText,
					CollidingKeyIndex,
					CurrentKey,
					NewKey),
					SelfIndex,
					CollidingActionIndex))
			{
				CancelUpdateInputKey(RevertReason);
			}

			return -1;
		}
		else if (RevertReason != ERevertRebindReason::None)
		{
			CancelUpdateInputKey(RevertReason);
			return -1;
		}
	}

	return FinishUpdateNewKey(MappingIndexToIgnore);
}

int32 UUINavInputBox::FinishUpdateNewKey(const int32 MappingIndexToIgnore /*= -1*/)
{
	const FKey OldKey = CurrentKey;

	int32 ModifiedActionMappingIndex = FinishUpdateNewEnhancedInputKey(AwaitingNewKey, AwaitingIndex, MappingIndexToIgnore);

	Container->OnKeyRebinded(InputName, OldKey, CurrentKey);
	Container->UINavPC->RefreshNavigationKeys();
	Container->UINavPC->UpdateInputIconsDelegate.Broadcast();
	AwaitingIndex = -1;

	return ModifiedActionMappingIndex;
}

int32 UUINavInputBox::FinishUpdateNewEnhancedInputKey(const FKey& PressedKey, const int Index, const int32 MappingIndexToIgnore /*= -1*/)
{

	if (!IsValid(Container) || !IsValid(Container->UINavPC) || !IsValid(Container->UINavPC->GetPC()) || !IsValid(Container->UINavPC->GetPC()->GetLocalPlayer()))
	{
		return -1;
	}
	UEnhancedInputLocalPlayerSubsystem* PlayerSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Container->UINavPC->GetPC()->GetLocalPlayer());
	if (!IsValid(PlayerSubsystem))
	{
		return -1;
	}
	UEnhancedInputUserSettings* PlayerSettings = PlayerSubsystem->GetUserSettings();
	if (!IsValid(PlayerSettings))
	{
		return -1;
	}
	FMapPlayerKeyArgs Args = {};
	Args.MappingName = PlayerMappableKeySettingsName;
	Args.Slot = EPlayerMappableKeySlot::First;
	Args.NewKey = PressedKey;
	FGameplayTagContainer FailureReason;
	PlayerSettings->MapPlayerKey(Args, FailureReason);
	if (FailureReason.IsValid())
	{
		FString Message = TEXT("Failed to rebind ");
		Message.Append(*InputName.ToString());
		Message.Append(TEXT(": "));
		Message.Append(FailureReason.ToStringSimple(true));
		DISPLAYERROR(Message);
		return -1;
	}
	
	PlayerSettings->ApplySettings();
	PlayerSettings->AsyncSaveSettings();

	Container->UINavPC->RequestRebuildMappings();

	CurrentKey = PressedKey;
	InputButton->SetText(GetKeyText(Index));
	UpdateKeyDisplay(Index);

	return 0;
}

void UUINavInputBox::CancelUpdateInputKey(const ERevertRebindReason Reason)
{
	if (AwaitingIndex < 0)
	{
		return;
	}

	Container->OnRebindCancelled(Reason, AwaitingNewKey);
	RevertToKeyText(AwaitingIndex);
	AwaitingIndex = -1;
}

FKey UUINavInputBox::GetKeyFromAxis(const FKey& AxisKey) const
{
	if (IS_AXIS && InputActionData.AxisScale == EAxisType::None)
	{
		return AxisKey;
	}

	FKey NewKey = Container->UINavPC->GetKeyFromAxis(AxisKey, AxisType == EAxisType::Positive);
	if (!NewKey.IsValid())
	{
		NewKey = AxisKey;
	}
	return NewKey;
}

void UUINavInputBox::ProcessInputName()
{
	if (InputActionData.Action->ValueType != EInputActionValueType::Boolean)
	{
		AxisType = InputActionData.AxisScale == EAxisType::Negative ? EAxisType::Negative : EAxisType::Positive;
	}
	InputName = InputActionData.Action->GetFName();
	if (InputActionData.DisplayName.IsEmpty())
	{
		InputActionData.DisplayName = FText::FromName(InputActionData.Action->GetFName());
	}

	if (IsValid(InputText))
	{
		InputText->SetText(InputActionData.DisplayName);
	}

	if (IsValid(InputRichText))
	{
		InputRichText->SetText(UUINavBlueprintFunctionLibrary::ApplyStyleRowToText(InputActionData.DisplayName, InputTextStyleRowName));
	}
}

void UUINavInputBox::InputComponentClicked()
{
	InputComponentClicked(0);
}

void UUINavInputBox::InputComponentClicked(const int Index)
{
	if (Container->UINavPC->GetAndConsumeIgnoreSelectRelease())
	{
		return;
	}

	AwaitingIndex = Index;

	InputButton->SetText(Container->PressKeyText);

	if (bUsingKeyImage)
	{
		InputButton->InputImage->SetVisibility(ESlateVisibility::Collapsed);
		
		if (IsValid(InputButton->NavText)) InputButton->NavText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (IsValid(InputButton->NavRichText)) InputButton->NavRichText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	Container->UINavPC->ListenToInputRebind(this);
}

FNavigationReply UUINavInputBox::NativeOnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply)
{
	FNavigationReply Reply = Super::NativeOnNavigation(MyGeometry, InNavigationEvent, InDefaultReply);

	if (!IsValid(Container))
	{
		return Reply;
	}

	UUINavInputBox* TargetInputBox = Container->GetInputBoxInDirection(this, InNavigationEvent.GetNavigationType());
	if (!IsValid(TargetInputBox))
	{
		return Reply;
	}

	if (InputButton->ForcedStyle == EButtonStyle::Hovered)
	{
		return FNavigationReply::Explicit(TargetInputBox->InputButton->TakeWidget());
	}
	return Reply;
}

bool UUINavInputBox::UpdateKeyIconForKey(const int Index)
{
	TSoftObjectPtr<UTexture2D> NewSoftTexture = GetDefault<UUINavSettings>()->bLoadInputIconsAsync ?
		Container->UINavPC->GetSoftKeyIcon(CurrentKey) :
		Container->UINavPC->GetKeyIcon(CurrentKey);
	if (!NewSoftTexture.IsNull())
	{
		InputButton->InputImage->SetBrushFromSoftTexture(NewSoftTexture);
		return true;
	}
	return false;
}

FText UUINavInputBox::GetKeyText(const int Index)
{
	const FKey Key = CurrentKey;
	return Container->UINavPC->GetKeyText(Key);
}

void UUINavInputBox::UpdateKeyDisplay(const int Index)
{
	bUsingKeyImage = UpdateKeyIconForKey(Index);
	if (bUsingKeyImage)
	{
		InputButton->InputImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (IsValid(InputButton->NavText)) InputButton->NavText->SetVisibility(ESlateVisibility::Collapsed);
		if (IsValid(InputButton->NavRichText)) InputButton->NavRichText->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		InputButton->InputImage->SetVisibility(ESlateVisibility::Collapsed);
		if (IsValid(InputButton->NavText)) InputButton->NavText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (IsValid(InputButton->NavRichText)) InputButton->NavRichText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UUINavInputBox::RevertToKeyText(const int Index)
{
	FText OldName;
	if (Index < KeysPerInput && !(CurrentKey.GetFName().IsEqual(FName("None"))))
	{
		OldName = GetKeyText(Index);
		UpdateKeyDisplay(Index);
	}
	else
	{
		OldName = Container->EmptyKeyText;
	}

	InputButton->SetText(OldName);
}

FText UUINavInputBox::GetCurrentText() const
{
	if (IsValid(InputText)) return InputText->GetText();
	if (IsValid(InputRichText)) return UUINavBlueprintFunctionLibrary::GetRawTextFromRichText(InputRichText->GetText());
	return FText();
}

int UUINavInputBox::ContainsKey(const FKey& CompareKey) const
{
	return CurrentKey == CompareKey ? 0 : -1;
}

bool UUINavInputBox::WantsAxisKey() const
{
	return IS_AXIS && InputActionData.AxisScale == EAxisType::None;
}

