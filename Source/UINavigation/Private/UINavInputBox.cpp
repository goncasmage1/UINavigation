// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavInputBox.h"
#include "UINavInputComponent.h"
#include "UINavInputContainer.h"
#include "UINavMacros.h"
#include "UINavSettings.h"
#include "UINavPCComponent.h"
#include "UINavWidget.h"
#include "UINavLocalPlayerSubsystem.h"
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

UUINavInputBox::UUINavInputBox(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SetIsFocusable(false);
}

void UUINavInputBox::NativeConstruct()
{
	Super::NativeConstruct();

	InputButton1->OnClicked.RemoveAll(this);
	InputButton1->OnClicked.AddDynamic(this, &UUINavInputBox::InputComponent1Clicked);

	InputButton2->OnClicked.RemoveAll(this);
	InputButton2->OnClicked.AddDynamic(this, &UUINavInputBox::InputComponent2Clicked);

	InputButton3->OnClicked.RemoveAll(this);
	InputButton3->OnClicked.AddDynamic(this, &UUINavInputBox::InputComponent3Clicked);
;}

void UUINavInputBox::CreateKeyWidgets()
{
	InputButtons = { InputButton1, InputButton2, InputButton3 };
	ProcessInputName();

	CreateEnhancedInputKeyWidgets();
}

void UUINavInputBox::CreateEnhancedInputKeyWidgets()
{
	const TArray<FEnhancedActionKeyMapping>& ActionMappings = InputContext->GetMappings();
	for (int j = 0; j < 3; j++)
	{
		UUINavInputComponent* NewInputButton = InputButtons[j];
		if (j < KeysPerInput)
		{
			for (int i = ActionMappings.Num() - 1; i >= 0; --i)
			{
				const FEnhancedActionKeyMapping& ActionMapping = ActionMappings[i];
				if (ActionMapping.Action != InputActionData.Action)
				{
					continue;
				}

				bool bPositive;
				EInputAxis Axis = InputActionData.Axis;
				Container->UINavPC->GetAxisPropertiesFromMapping(ActionMapping, bPositive, Axis);
				TArray<int32> MappingsForAction;
				GetEnhancedMappingsForAction(ActionMapping.Action, InputActionData.Axis, j, MappingsForAction);
				UUINavInputBox* OppositeInputBox = Container->GetOppositeInputBox(InputActionData);
				FKey NewKey = ActionMapping.Key;

				if ((InputActionData.Axis == Axis || Container->UINavPC->IsAxis2D(NewKey)) &&
					(OppositeInputBox == nullptr || !OppositeInputBox->Keys.IsValidIndex(j) || OppositeInputBox->Keys[j] != ActionMapping.Key) &&
					(MappingsForAction.Num() < 2 || GetNumValidKeys(j) < (MappingsForAction.Num() / 2)))
				{
					if (Container->UINavPC->IsAxis(NewKey) && InputActionData.AxisScale != EAxisType::None)
					{
						NewKey = Container->UINavPC->GetKeyFromAxis(NewKey, bPositive ? AxisType == EAxisType::Positive : AxisType != EAxisType::Positive, InputActionData.Axis);
					}
					else if (OppositeInputBox != nullptr &&
						InputActionData.AxisScale != EAxisType::None &&
						(InputActionData.AxisScale == EAxisType::Positive) != bPositive)
					{
						continue;
					}
						
					if (!Container->RespectsRestriction(NewKey, j))
					{
						continue;
					}

					if (TrySetupNewKey(NewKey, j, NewInputButton))
					{
						break;
					}
				}
			}
		}
		else
		{
			NewInputButton->SetVisibility(Container->bCollapseInputBoxes ? ESlateVisibility::Collapsed : ESlateVisibility::Hidden);
		}

		if (Keys.Num() - 1 < j)
		{
			NewInputButton->SetText(Container->EmptyKeyText);
			NewInputButton->InputImage->SetVisibility(ESlateVisibility::Collapsed);
			if (IsValid(NewInputButton->NavText)) NewInputButton->NavText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			if (IsValid(NewInputButton->NavRichText)) NewInputButton->NavRichText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			Keys.Add(FKey());
		}
	}

	if (Keys.Num() == 0)
	{
		FString Message = TEXT("Couldn't find Input with name ");
		Message.Append(*InputName.ToString());
		Message.Append(TEXT(" or wtih valid restriction."));
		DISPLAYERROR(Message);
		return;
	}
}

bool UUINavInputBox::TrySetupNewKey(const FKey& NewKey, const int KeyIndex, UUINavInputComponent* const NewInputButton)
{
	if (!NewKey.IsValid() || Keys.IsValidIndex(KeyIndex) || Keys.Contains(NewKey)) return false;

	Keys.Add(NewKey);

	if (UpdateKeyIconForKey(KeyIndex))
	{
		bUsingKeyImage[KeyIndex] = true;
		NewInputButton->InputImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (IsValid(NewInputButton->NavText)) NewInputButton->NavText->SetVisibility(ESlateVisibility::Collapsed);
		if (IsValid(NewInputButton->NavRichText)) NewInputButton->NavRichText->SetVisibility(ESlateVisibility::Collapsed);
	}
	NewInputButton->SetText(GetKeyText(KeyIndex));

	return true;
}

void UUINavInputBox::ResetKeyWidgets()
{
	Keys.Empty();
	bUsingKeyImage = { false, false, false };
	InputButtons.Empty();
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
			if (!Keys[Index].IsValid())
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
					Keys[Index],
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
	const FKey OldKey = Keys[AwaitingIndex];

	int32 ModifiedActionMappingIndex = FinishUpdateNewEnhancedInputKey(AwaitingNewKey, AwaitingIndex, MappingIndexToIgnore);

	Container->OnKeyRebinded(InputName, OldKey, Keys[AwaitingIndex]);
	Container->UINavPC->RefreshNavigationKeys();
	Container->UINavPC->UpdateInputIconsDelegate.Broadcast();
	AwaitingIndex = -1;

	return ModifiedActionMappingIndex;
}

int32 UUINavInputBox::FinishUpdateNewEnhancedInputKey(const FKey& PressedKey, const int Index, const int32 MappingIndexToIgnore /*= -1*/)
{
	const TArray<FEnhancedActionKeyMapping>& ActionMappings = InputContext->GetMappings();

	int32 ModifiedActionMappingIndex = -1;
	bool bPositive;
	const FKey PressedAxisKey = Container->UINavPC->GetAxisFromScaledKey(PressedKey, bPositive);
	const FKey NewKey = WantsAxisKey() && PressedAxisKey.IsValid() ? PressedAxisKey : PressedKey;

	FKey NewAxisKey;
	FKey OldAxisKey;
	bool bFound = false;
	bool bRemoved2DAxis = false;
	const int Iterations = ActionMappings.Num();
	int i = Iterations - 1;
	for (; i >= 0; --i)
	{
		FEnhancedActionKeyMapping& ActionMapping = InputContext->GetMapping(i);
		if (ActionMapping.Action == InputActionData.Action && i != MappingIndexToIgnore)
		{
			EInputAxis Axis = InputActionData.Axis;
			Container->UINavPC->GetAxisPropertiesFromMapping(ActionMapping, bPositive, Axis);
			if (InputActionData.Axis == Axis)
			{
				const FKey& MappingKey = GetKeyFromAxis(ActionMapping.Key);
				if (Container->RespectsRestriction(NewKey, Index) &&
					Container->RespectsRestriction(MappingKey, Index))
				{
					if (MappingKey == Keys[Index])
					{
						ModifiedActionMappingIndex = i;
						if (IS_AXIS)
						{
							if (Container->UINavPC->IsAxis(ActionMapping.Key) &&
								InputActionData.AxisScale != EAxisType::None)
							{
								bool bNewKeyPositive = true;
								if (Container->UINavPC->GetAxisFromScaledKey(NewKey, bNewKeyPositive).IsValid())
								{
									NewAxisKey = NewKey;
								}
								else
								{
									OldAxisKey = ActionMapping.Key;
								}
								break;
							}

							ActionMapping.Key = NewKey;
						}
						else
						{
							ActionMapping.Key = NewKey;
						}
						Keys[Index] = NewKey;
						InputButtons[Index]->SetText(GetKeyText(Index));
						bFound = true;
						break;
					}
					else if (!Container->UINavPC->IsAxis(ActionMapping.Key) &&
						Container->GetOppositeInputBox(InputActionData) != nullptr)
					{
						bool bOtherKeyPositive = true;
						const FKey OtherKeyAxis = Container->UINavPC->GetAxisFromScaledKey(ActionMapping.Key, bOtherKeyPositive);
						bool bNewKeyPositive = true;
						const FKey NewKeyAxis = Container->UINavPC->GetAxisFromScaledKey(NewKey, bNewKeyPositive);

						if (OtherKeyAxis == NewKeyAxis &&
							bOtherKeyPositive != bNewKeyPositive &&
							InputActionData.AxisScale != EAxisType::None &&
							(InputActionData.AxisScale == EAxisType::Positive) == bNewKeyPositive)
						{
							NewAxisKey = NewKeyAxis;
							break;
						}
					}
					else
					{
						OldAxisKey = ActionMapping.Key;
						break;
					}
				}
			}
		}
	}

	if (!bFound)
	{
		bool bNegateX = false;
		bool bNegateY = false;
		bool bNegateZ = false;

		bRemoved2DAxis = InputActionData.AxisScale != EAxisType::None;
		GetKeyMappingNegateAxes(OldAxisKey, bNegateX, bNegateY, bNegateZ);

		if (InputActionData.AxisScale == EAxisType::Positive)
		{
			UnmapEnhancedAxisKey(NewAxisKey, OldAxisKey, NewKey, Index, bNegateX, bNegateY, bNegateZ);
		}

		// Add new key
		const FKey Key = NewAxisKey.IsValid() ? NewAxisKey : NewKey;
		FEnhancedActionKeyMapping& NewMapping = InputContext->MapKey(InputActionData.Action, Key);
		if (OldAxisKey.IsValid())
		{
			AddRelevantModifiers(InputActionData, NewMapping);
			if (bRemoved2DAxis)
			{
				ApplyNegateModifiers(this, NewMapping, bNegateX, bNegateY, bNegateZ);
			}
		}
		if (NewAxisKey.IsValid())
		{
			InputContext->UnmapKey(InputActionData.Action, Keys[Index]);
		}

		// Remove old key
		if (InputActionData.AxisScale == EAxisType::Negative)
		{
			UnmapEnhancedAxisKey(NewAxisKey, OldAxisKey, NewKey, Index, bNegateX, bNegateY, bNegateZ);
		}

		Keys[Index] = NewKey;
		InputButtons[Index]->SetText(GetKeyText(Index));

		TryMap2DAxisKey(NewMapping.Key, Index);
	}
	else
	{
		// Check if the current setup allows replacing 2 keys with an axis key
		TryMapEnhancedAxisKey(NewKey, Index);
	}

	Container->UINavPC->RequestRebuildMappings();

	ULocalPlayer::GetSubsystem<UUINavLocalPlayerSubsystem>(GetOwningLocalPlayer())->SaveInputContextState(InputContext);

	UpdateKeyDisplay(Index);

	if (bRemoved2DAxis)
	{
		Container->ResetInputBox(InputName, AxisType == EAxisType::Positive ? EAxisType::Negative : EAxisType::Positive);
	}

	return ModifiedActionMappingIndex;
}

void UUINavInputBox::TryMapEnhancedAxisKey(const FKey& NewKey, const int32 Index)
{
	UUINavInputBox* OppositeInputBox = Container->GetOppositeInputBox(InputActionData);
	if (OppositeInputBox != nullptr)
	{
		const FKey OppositeInputBoxKey = OppositeInputBox->Keys.IsValidIndex(Index) ? OppositeInputBox->Keys[Index] : FKey();
		bool bIsOppositeKeyPositive = false;
		const FKey NewOppositeKey = Container->UINavPC->GetOppositeAxisKey(NewKey, bIsOppositeKeyPositive);
		if (NewOppositeKey == OppositeInputBoxKey)
		{
			bool bNegateX = false;
			bool bNegateY = false;
			bool bNegateZ = false;

			const TArray<FEnhancedActionKeyMapping>& ActionMappings = InputContext->GetMappings();
			const FKey& CurrentKey = Keys[Index];
			int32 MappingIndex = ActionMappings.IndexOfByPredicate([this, &CurrentKey](const FEnhancedActionKeyMapping& Other) { return Other.Action == InputActionData.Action && Other.Key == CurrentKey; });
			if (MappingIndex != INDEX_NONE)
			{
				const FEnhancedActionKeyMapping& OldMapping = InputContext->GetMapping(MappingIndex);
				for (const UInputModifier* const Modifier : OldMapping.Modifiers)
				{
					const UInputModifierNegate* const NegateModifier = Cast<UInputModifierNegate>(Modifier);
					if (IsValid(NegateModifier))
					{
						bNegateX = InputActionData.Axis == EInputAxis::X && NegateModifier->bX;
						bNegateY = InputActionData.Axis == EInputAxis::Y && NegateModifier->bY;
						bNegateZ = InputActionData.Axis == EInputAxis::Z && NegateModifier->bZ;
					}
				}
			}

			InputContext->UnmapKey(InputActionData.Action, NewOppositeKey);
			InputContext->UnmapKey(InputActionData.Action, NewKey);
			bool bPositive;
			FEnhancedActionKeyMapping& NewMapping = InputContext->MapKey(InputActionData.Action, Container->UINavPC->GetAxisFromScaledKey(NewKey, bPositive));
			AddRelevantModifiers(InputActionData, NewMapping);
			ApplyNegateModifiers(this, NewMapping, bNegateX, bNegateY, bNegateZ);

			TryMap2DAxisKey(NewMapping.Key, Index);
		}
	}
}

void UUINavInputBox::TryMap2DAxisKey(const FKey& NewMappingKey, const int Index)
{
	const FKey OppositeAxis = Container->UINavPC->GetOppositeAxis2DAxis(NewMappingKey);
	if (OppositeAxis.IsValid())
	{
		TArray<int32> MappingsForAction;
		const EInputAxis Axis = InputActionData.Axis == EInputAxis::X ? EInputAxis::Y : EInputAxis::X;
		GetEnhancedMappingsForAction(InputActionData.Action, Axis, Index, MappingsForAction);
		bool bShouldMap2DKey = false;
		const FEnhancedActionKeyMapping* KeyMapping = nullptr;
		const FEnhancedActionKeyMapping* OppositeKeyMapping = nullptr;
		for (const int32 MappingIndex : MappingsForAction)
		{
			if (InputContext->GetMapping(MappingIndex).Key == NewMappingKey)
			{
				KeyMapping = &InputContext->GetMapping(MappingIndex);
			}

			if (InputContext->GetMapping(MappingIndex).Key == OppositeAxis)
			{
				bShouldMap2DKey = true;
				OppositeKeyMapping = &InputContext->GetMapping(MappingIndex);
			}
		}

		if (!bShouldMap2DKey)
		{
			return;
		}

		bool bNegateX = false;
		bool bNegateY = false;
		bool bNegateZ = false;
		GetKeyMappingNegateAxes(NewMappingKey, bNegateX, bNegateY, bNegateZ);
		bNegateX = InputActionData.Axis == EInputAxis::X && bNegateX;
		bNegateY = InputActionData.Axis == EInputAxis::Y && bNegateY;
		bNegateZ = InputActionData.Axis == EInputAxis::Z && bNegateZ;
		
		InputContext->UnmapKey(InputActionData.Action, NewMappingKey);
		InputContext->UnmapKey(InputActionData.Action, OppositeAxis);
		FEnhancedActionKeyMapping& NewMapping = InputContext->MapKey(InputActionData.Action, Container->UINavPC->GetAxis2DFromAxis1D(OppositeAxis));
		ApplyNegateModifiers(this, NewMapping, bNegateX, bNegateY, bNegateZ);
	}
}

void UUINavInputBox::UnmapEnhancedAxisKey(const FKey& NewAxisKey, const FKey& OldAxisKey, const FKey& NewKey, const int32 Index, const bool bNegateX, const bool bNegateY, const bool bNegateZ)
{
	UUINavInputBox* OppositeInputBox = Container->GetOppositeInputBox(InputActionData);
	if (OppositeInputBox != nullptr)
	{
		if (OldAxisKey.IsValid())
		{
			InputContext->UnmapKey(InputActionData.Action, OldAxisKey);
			const FKey OppositeInputBoxKey = OppositeInputBox->Keys.IsValidIndex(Index) ? OppositeInputBox->Keys[Index] : FKey();
			FEnhancedActionKeyMapping& NewMapping = InputContext->MapKey(InputActionData.Action, OppositeInputBoxKey);
			AddRelevantModifiers(OppositeInputBox->InputActionData, NewMapping);

			ApplyNegateModifiers(OppositeInputBox, NewMapping, bNegateX, bNegateY, bNegateZ);

			if (Container->UINavPC->IsAxis2D(OldAxisKey))
			{
				const EInputAxis OppositeAxis = InputActionData.Axis == EInputAxis::X ? EInputAxis::Y : EInputAxis::X;
				const FKey NewAxis1D = Container->UINavPC->GetAxis1DFromAxis2D(OldAxisKey, OppositeAxis);
				if (NewAxis1D.IsValid())
				{
					FEnhancedActionKeyMapping& NewAxis1DMapping = InputContext->MapKey(InputActionData.Action, NewAxis1D);
					if (OppositeAxis == EInputAxis::Y)
					{
						UInputModifierSwizzleAxis* SwizzleModifier = NewObject<UInputModifierSwizzleAxis>();
						NewAxis1DMapping.Modifiers.Add(SwizzleModifier);
					}
				}
			}
		}
		else if (NewAxisKey.IsValid())
		{
			bool bIsOppositeKeyPositive = false;
			InputContext->UnmapKey(InputActionData.Action, Container->UINavPC->GetOppositeAxisKey(NewKey, bIsOppositeKeyPositive));
		}
	}
}

void UUINavInputBox::AddRelevantModifiers(const FInputContainerEnhancedActionData& ActionData, FEnhancedActionKeyMapping& Mapping)
{
	if (ActionData.AxisScale == EAxisType::Negative)
	{
		UInputModifierNegate* NegateModifier = NewObject<UInputModifierNegate>();
		NegateModifier->bX = ActionData.Axis == EInputAxis::X;
		NegateModifier->bY = ActionData.Axis == EInputAxis::Y;
		NegateModifier->bZ = ActionData.Axis == EInputAxis::Z;
		Mapping.Modifiers.Add(NegateModifier);
	}

	if (ActionData.Axis != EInputAxis::X)
	{
		UInputModifierSwizzleAxis* SwizzleModifier = NewObject<UInputModifierSwizzleAxis>();
		if (ActionData.Axis == EInputAxis::Z)
		{
			SwizzleModifier->Order = EInputAxisSwizzle::ZXY;
		}
		Mapping.Modifiers.Add(SwizzleModifier);
	}
}

void UUINavInputBox::ApplyNegateModifiers(UUINavInputBox* InputBox, FEnhancedActionKeyMapping& Mapping, const bool bNegateX, const bool bNegateY, const bool bNegateZ)
{
	const bool bShouldNegateX = InputBox->InputActionData.Axis == EInputAxis::X && bNegateX && (InputBox->InputActionData.AxisScale == EAxisType::Positive) != bNegateX;
	const bool bShouldNegateY = InputBox->InputActionData.Axis == EInputAxis::Y && bNegateY && (InputBox->InputActionData.AxisScale == EAxisType::Positive) != bNegateY;
	const bool bShouldNegateZ = InputBox->InputActionData.Axis == EInputAxis::Z && bNegateZ && (InputBox->InputActionData.AxisScale == EAxisType::Positive) != bNegateZ;

	bool bHasNegateModifier = false;
	for (int i = Mapping.Modifiers.Num() - 1; i >= 0; --i)
	{
		UInputModifierNegate* NegateModifier = Cast<UInputModifierNegate>(Mapping.Modifiers[i]);
		if (IsValid(NegateModifier))
		{
			bHasNegateModifier = true;
			NegateModifier->bX = NegateModifier->bX && bShouldNegateX;
			NegateModifier->bY = NegateModifier->bY && bShouldNegateY;
			NegateModifier->bZ = NegateModifier->bZ && bShouldNegateZ;
			if (!NegateModifier->bX && !NegateModifier->bY && !NegateModifier->bZ)
			{
				bHasNegateModifier = false;
				Mapping.Modifiers.RemoveAt(i);
			}
		}
	}

	if (!bHasNegateModifier && (bShouldNegateX || bShouldNegateY || bShouldNegateZ))
	{
		UInputModifierNegate* NewNegateModifier = NewObject<UInputModifierNegate>();
		NewNegateModifier->bX = bShouldNegateX;
		NewNegateModifier->bY = bShouldNegateY;
		NewNegateModifier->bZ = bShouldNegateZ;
		Mapping.Modifiers.Add(NewNegateModifier);
	}
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

int UUINavInputBox::GetNumValidKeys(const int Index) const
{
	int ValidKeys = 0;
	for (const FKey& Key : Keys)
	{
		if (Key.IsValid() && Container->RespectsRestriction(Key, Index))
		{
			++ValidKeys;
		}
	}
	return ValidKeys;
}

void UUINavInputBox::InputComponent1Clicked()
{
	InputComponentClicked(0);
}

void UUINavInputBox::InputComponent2Clicked()
{
	InputComponentClicked(1);
}

void UUINavInputBox::InputComponent3Clicked()
{
	InputComponentClicked(2);
}

void UUINavInputBox::InputComponentClicked(const int Index)
{
	if (Container->UINavPC->GetAndConsumeIgnoreSelectRelease())
	{
		return;
	}

	AwaitingIndex = Index;

	InputButtons[Index]->SetText(Container->PressKeyText);

	if (bUsingKeyImage[Index])
	{
		InputButtons[Index]->InputImage->SetVisibility(ESlateVisibility::Collapsed);
		
		if (IsValid(InputButtons[Index]->NavText)) InputButtons[Index]->NavText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (IsValid(InputButtons[Index]->NavRichText)) InputButtons[Index]->NavRichText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	Container->UINavPC->ListenToInputRebind(this);
}

void UUINavInputBox::GetEnhancedMappingsForAction(const UInputAction* Action, const EInputAxis& Axis, const int Index, TArray<int32>& OutMappingIndices)
{
	const TArray<FEnhancedActionKeyMapping>& ActionMappings = InputContext->GetMappings();
	for (int i = ActionMappings.Num() - 1; i >= 0; --i)
	{
		const FEnhancedActionKeyMapping& ActionMapping = ActionMappings[i];
		if (ActionMapping.Action == Action)
		{
			bool bPositive;
			EInputAxis ActionAxis = InputActionData.Axis;
			Container->UINavPC->GetAxisPropertiesFromMapping(ActionMapping, bPositive, ActionAxis);
			if (ActionAxis == Axis && Container->RespectsRestriction(ActionMapping.Key, Index))
			{
				OutMappingIndices.Add(i);
			}
		}
	}
}

void UUINavInputBox::GetKeyMappingNegateAxes(const FKey& OldAxisKey, bool& bNegateX, bool& bNegateY, bool& bNegateZ)
{
	const TArray<FEnhancedActionKeyMapping>& ActionMappings = InputContext->GetMappings();
	bNegateX = false;
	bNegateY = false;
	bNegateZ = false;
	int32 MappingIndex = ActionMappings.IndexOfByPredicate([this, &OldAxisKey](const FEnhancedActionKeyMapping& Other) { return Other.Action == InputActionData.Action && Other.Key == OldAxisKey; });
	if (MappingIndex != INDEX_NONE)
	{
		const FEnhancedActionKeyMapping& OldMapping = InputContext->GetMapping(MappingIndex);
		for (const UInputModifier* const Modifier : OldMapping.Modifiers)
		{
			const UInputModifierNegate* const NegateModifier = Cast<UInputModifierNegate>(Modifier);
			if (IsValid(NegateModifier))
			{
				bNegateX = NegateModifier->bX;
				bNegateY = NegateModifier->bY;
				bNegateZ = NegateModifier->bZ;
			}
		}
	}
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

	int TargetIndex = -1;
	for (int i = 0; i < InputButtons.Num(); ++i)
	{
		const UUINavInputComponent* const InputButton = InputButtons[i];
		if (InputButton->ForcedStyle == EButtonStyle::Hovered)
		{
			TargetIndex = i;
			break;
		}
	}

	if (!TargetInputBox->InputButtons.IsValidIndex(TargetIndex))
	{
		return Reply;
	}

	return FNavigationReply::Explicit(TargetInputBox->InputButtons[TargetIndex]->TakeWidget());
}

bool UUINavInputBox::UpdateKeyIconForKey(const int Index)
{
	TSoftObjectPtr<UTexture2D> NewSoftTexture = GetDefault<UUINavSettings>()->bLoadInputIconsAsync ?
		Container->UINavPC->GetSoftKeyIcon(Keys[Index]) :
		Container->UINavPC->GetKeyIcon(Keys[Index]);
	if (!NewSoftTexture.IsNull())
	{
		InputButtons[Index]->InputImage->SetBrushFromSoftTexture(NewSoftTexture);
		return true;
	}
	return false;
}

FText UUINavInputBox::GetKeyText(const int Index)
{
	const FKey Key = Keys[Index];
	return Container->UINavPC->GetKeyText(Key);
}

void UUINavInputBox::UpdateKeyDisplay(const int Index)
{
	bUsingKeyImage[Index] = UpdateKeyIconForKey(Index);
	if (bUsingKeyImage[Index])
	{
		InputButtons[Index]->InputImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (IsValid(InputButtons[Index]->NavText)) InputButtons[Index]->NavText->SetVisibility(ESlateVisibility::Collapsed);
		if (IsValid(InputButtons[Index]->NavRichText)) InputButtons[Index]->NavRichText->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		InputButtons[Index]->InputImage->SetVisibility(ESlateVisibility::Collapsed);
		if (IsValid(InputButtons[Index]->NavText)) InputButtons[Index]->NavText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (IsValid(InputButtons[Index]->NavRichText)) InputButtons[Index]->NavRichText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UUINavInputBox::RevertToKeyText(const int Index)
{
	FText OldName;
	if (Index < KeysPerInput && !(Keys[Index].GetFName().IsEqual(FName("None"))))
	{
		OldName = GetKeyText(Index);
		UpdateKeyDisplay(Index);
	}
	else
	{
		OldName = Container->EmptyKeyText;
	}

	InputButtons[Index]->SetText(OldName);
}

FText UUINavInputBox::GetCurrentText() const
{
	if (IsValid(InputText)) return InputText->GetText();
	if (IsValid(InputRichText)) return UUINavBlueprintFunctionLibrary::GetRawTextFromRichText(InputRichText->GetText());
	return FText();
}

int UUINavInputBox::ContainsKey(const FKey& CompareKey) const
{
	return Keys.IndexOfByKey(CompareKey);
}

bool UUINavInputBox::WantsAxisKey() const
{
	return IS_AXIS && InputActionData.AxisScale == EAxisType::None;
}

