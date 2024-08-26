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
	if (KeyMapping != nullptr && IsValid(KeyMapping->GetAssociatedInputAction()))
	{
		ProcessInputName(KeyMapping->GetAssociatedInputAction());
	}
	if (KeyMapping != nullptr && KeyMapping->GetCurrentKey().IsValid())
	{
		TrySetupNewKey(KeyMapping->GetCurrentKey());
	} else
	{
		InputButton->SetText(Container->EmptyKeyText);
		InputButton->InputImage->SetVisibility(ESlateVisibility::Collapsed);
		if (IsValid(InputButton->NavText)) InputButton->NavText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (IsValid(InputButton->NavRichText)) InputButton->NavRichText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		CurrentKey = FKey();
	}
}

bool UUINavInputBox::TrySetupNewKey(const FKey& NewKey)
{
	if (!NewKey.IsValid()) return false;
	CurrentKey = NewKey;
	
	if (UpdateKeyIconForKey())
	{
		bUsingKeyImage = true;
		InputButton->InputImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (IsValid(InputButton->NavText)) InputButton->NavText->SetVisibility(ESlateVisibility::Collapsed);
		if (IsValid(InputButton->NavRichText)) InputButton->NavRichText->SetVisibility(ESlateVisibility::Collapsed);
	}
	InputButton->SetText(GetKeyText());

	return true;
}

void UUINavInputBox::ResetKeyWidgets()
{
	CurrentKey = FKey();
	bUsingKeyImage = false;
	CreateKeyWidgets();
}

void UUINavInputBox::UpdateInputKey(const FKey& NewKey, const bool bSkipChecks)
{
	AwaitingNewKey = NewKey;

	if (!bSkipChecks)
	{
		int CollidingActionIndex = INDEX_NONE;
		const ERevertRebindReason RevertReason = Container->CanRegisterKey(this, NewKey, CollidingActionIndex);
		if (RevertReason == ERevertRebindReason::UsedBySameInputGroup)
		{
			if (!CurrentKey.IsValid())
			{
				CancelUpdateInputKey(RevertReason);
				return;
			}

			int SelfIndex = INDEX_NONE;

			FInputRebindData CollidingInputData;
			Container->GetEnhancedInputRebindData(CollidingActionIndex, CollidingInputData);
			if (!Container->InputBoxes.Find(this, SelfIndex) ||
				!Container->RequestKeySwap(FInputCollisionData(GetCurrentText(),
					CollidingInputData.InputText,
					CurrentKey,
					NewKey),
					SelfIndex,
					CollidingActionIndex))
			{
				CancelUpdateInputKey(RevertReason);
			}

			return;
		}
		else if (RevertReason != ERevertRebindReason::None)
		{
			CancelUpdateInputKey(RevertReason);
			return;
		}
	}

	return FinishUpdateNewKey();
}

void UUINavInputBox::FinishUpdateNewKey()
{
	const FKey OldKey = CurrentKey;
	FinishUpdateNewEnhancedInputKey(AwaitingNewKey);
	Container->OnKeyRebinded(InputName, OldKey, CurrentKey);
	Container->UINavPC->RefreshNavigationKeys();
	Container->UINavPC->UpdateInputIconsDelegate.Broadcast();
	bAwaitingNewKey = false;
}

void UUINavInputBox::FinishUpdateNewEnhancedInputKey(const FKey& PressedKey)
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
		return;
	}
	
	PlayerSettings->ApplySettings();
	PlayerSettings->AsyncSaveSettings();

	Container->UINavPC->RequestRebuildMappings();

	CurrentKey = PressedKey;
	InputButton->SetText(GetKeyText());
	UpdateKeyDisplay();
}

void UUINavInputBox::CancelUpdateInputKey(const ERevertRebindReason Reason)
{
	if (!bAwaitingNewKey)
	{
		return;
	}

	Container->OnRebindCancelled(Reason, AwaitingNewKey);
	RevertToKeyText();
	bAwaitingNewKey = false;
}

void UUINavInputBox::ProcessInputName(const UInputAction* Action)
{
	InputName = Action->GetFName();
	if (UPlayerMappableKeySettings* Settings = Action->GetPlayerMappableKeySettings().Get(); IsValid(Settings))
	{
		if (Settings->DisplayName.IsEmpty())
		{
			ActionDisplayName = FText::FromName(InputName);
		} else
		{
			ActionDisplayName = Settings->DisplayName;
		}
	}
}

void UUINavInputBox::InputComponentClicked()
{
	if (Container->UINavPC->GetAndConsumeIgnoreSelectRelease())
	{
		return;
	}

	bAwaitingNewKey = true;

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

bool UUINavInputBox::UpdateKeyIconForKey()
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

FText UUINavInputBox::GetKeyText()
{
	const FKey Key = CurrentKey;
	return Container->UINavPC->GetKeyText(Key);
}

void UUINavInputBox::UpdateKeyDisplay()
{
	bUsingKeyImage = UpdateKeyIconForKey();
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

void UUINavInputBox::RevertToKeyText()
{
	FText OldName;
	if (!(CurrentKey.GetFName().IsEqual(FName("None"))))
	{
		OldName = GetKeyText();
		UpdateKeyDisplay();
	}
	else
	{
		OldName = Container->EmptyKeyText;
	}

	InputButton->SetText(OldName);
}

FText UUINavInputBox::GetCurrentText() const
{
	return ActionDisplayName;
}

bool UUINavInputBox::ContainsKey(const FKey& CompareKey) const
{
	return CurrentKey == CompareKey;
}
