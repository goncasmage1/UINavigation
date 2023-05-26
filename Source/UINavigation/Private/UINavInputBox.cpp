// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavInputBox.h"
#include "UINavInputComponent.h"
#include "UINavInputContainer.h"
#include "UINavMacros.h"
#include "UINavSettings.h"
#include "UINavPCComponent.h"
#include "UINavWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Data/RevertRebindReason.h"
#include "Engine/DataTable.h"
#include "GameFramework/InputSettings.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "InputAction.h"
#include "InputMappingContext.h"

#define IS_POSITIVE_AXIS (AxisType == EAxisType::Positive)
#define IS_NEGATIVE_AXIS (AxisType == EAxisType::Negative)
#define IS_RIGHT_SCALE(Axis) ((Axis.Scale > 0.0f && IS_POSITIVE_AXIS) || (Axis.Scale < 0.0f && IS_NEGATIVE_AXIS))
#define GET_REVERSE_AXIS (AxisType == EAxisType::Positive ? EAxisType::Negative : EAxisType::Positive)

UUINavInputBox::UUINavInputBox(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bIsFocusable = false;
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
				Container->GetAxisPropertiesFromMapping(ActionMapping, bPositive, Axis);
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
						NewKey = Container->UINavPC->GetKeyFromAxis(NewKey, bPositive ? IS_POSITIVE_AXIS : !IS_POSITIVE_AXIS, InputActionData.Axis);
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
			NewInputButton->NavText->SetText(Container->EmptyKeyText);
			NewInputButton->InputImage->SetVisibility(ESlateVisibility::Collapsed);
			NewInputButton->NavText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
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

bool UUINavInputBox::TrySetupNewKey(const FKey NewKey, const int KeyIndex, const UUINavInputComponent* const NewInputButton)
{
	if (!NewKey.IsValid() || Keys.IsValidIndex(KeyIndex) || Keys.Contains(NewKey)) return false;

	Keys.Add(NewKey);

	if (UpdateKeyIconForKey(KeyIndex))
	{
		bUsingKeyImage[KeyIndex] = true;
		NewInputButton->InputImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		NewInputButton->NavText->SetVisibility(ESlateVisibility::Collapsed);
	}
	NewInputButton->NavText->SetText(GetKeyText(KeyIndex));

	return true;
}

void UUINavInputBox::ResetKeyWidgets()
{
	Keys.Empty();
	bUsingKeyImage = { false, false, false };
	InputButtons.Empty();
	CreateKeyWidgets();
}

void UUINavInputBox::UpdateInputKey(const FKey NewKey, int Index, const bool bSkipChecks)
{
	if (Index < 0) Index = AwaitingIndex;

	if (AwaitingIndex < 0) AwaitingIndex = Index;

	AwaitingNewKey = NewKey;
	if (Index < 0)
	{
		CancelUpdateInputKey(ERevertRebindReason::None);
		return;
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
				return;
			}

			int SelfIndex = INDEX_NONE;

			FInputRebindData CollidingInputData;
			Container->GetEnhancedInputRebindData(CollidingActionIndex, CollidingInputData);
			if (!Container->InputBoxes.Find(this, SelfIndex) ||
				!Container->RequestKeySwap(FInputCollisionData(InputText->GetText(),
					CollidingInputData.InputText,
					CollidingKeyIndex,
					Keys[Index],
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

	FinishUpdateNewKey();
}

void UUINavInputBox::FinishUpdateNewKey()
{
	const FKey OldKey = Keys[AwaitingIndex];

	FinishUpdateNewEnhancedInputKey(AwaitingNewKey, AwaitingIndex);

	Container->OnKeyRebinded(InputName, OldKey, Keys[AwaitingIndex]);
	Container->UINavPC->RefreshNavigationKeys();
	AwaitingIndex = -1;
}

void UUINavInputBox::FinishUpdateNewEnhancedInputKey(const FKey PressedKey, const int Index)
{
	const TArray<FEnhancedActionKeyMapping>& ActionMappings = InputContext->GetMappings();

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
		if (ActionMapping.Action == InputActionData.Action)
		{
			EInputAxis Axis = InputActionData.Axis;
			Container->GetAxisPropertiesFromMapping(ActionMapping, bPositive, Axis);
			if (InputActionData.Axis == Axis)
			{
				const FKey& MappingKey = GetKeyFromAxis(ActionMapping.Key);
				if (Container->RespectsRestriction(NewKey, Index) &&
					Container->RespectsRestriction(MappingKey, Index))
				{
					if (MappingKey == Keys[Index])
					{
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
						InputButtons[Index]->NavText->SetText(GetKeyText(Index));
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
						break;
					}
				}
			}
		}
	}

	if (!bFound)
	{
		// Remove old key
		if (InputActionData.AxisScale == EAxisType::Positive)
		{
			UnmapEnhancedAxisKey(NewAxisKey, OldAxisKey, NewKey, Index);
			bRemoved2DAxis = true;
		}

		// Add new key
		const FKey Key = NewAxisKey.IsValid() ? NewAxisKey : NewKey;
		FEnhancedActionKeyMapping& NewMapping = InputContext->MapKey(InputActionData.Action, Key);
		if (OldAxisKey.IsValid())
		{
			AddRelevantModifiers(InputActionData, NewMapping);
		}
		if (NewAxisKey.IsValid())
		{
			InputContext->UnmapKey(InputActionData.Action, Keys[Index]);
		}

		// Remove old key
		if (InputActionData.AxisScale == EAxisType::Negative)
		{
			UnmapEnhancedAxisKey(NewAxisKey, OldAxisKey, NewKey, Index);
			bRemoved2DAxis = true;
		}

		Keys[Index] = NewKey;
		InputButtons[Index]->NavText->SetText(GetKeyText(Index));

		TryMap2DAxisKey(NewMapping.Key, Index);
	}
	else
	{
		// Check if the current setup allows replacing 2 keys with an axis key
		TryMapEnhancedAxisKey(NewKey, Index);
	}

	Container->UINavPC->RequestRebuildMappings();

	UpdateKeyDisplay(Index);

	if (bRemoved2DAxis)
	{
		Container->ResetInputBox(InputName, GET_REVERSE_AXIS);
	}
}

void UUINavInputBox::TryMapEnhancedAxisKey(const FKey& NewKey, const int32 Index)
{
	UUINavInputBox* OppositeInputBox = Container->GetOppositeInputBox(InputActionData);
	if (OppositeInputBox != nullptr)
	{
		const FKey OppositeInputBoxKey = OppositeInputBox->Keys.IsValidIndex(Index) ? OppositeInputBox->Keys[Index] : FKey();
		const FKey NewOppositeKey = Container->UINavPC->GetOppositeAxisKey(NewKey);
		if (NewOppositeKey == OppositeInputBoxKey)
		{
			InputContext->UnmapKey(InputActionData.Action, NewOppositeKey);
			InputContext->UnmapKey(InputActionData.Action, NewKey);
			bool bPositive;
			FEnhancedActionKeyMapping& NewMapping = InputContext->MapKey(InputActionData.Action, Container->UINavPC->GetAxisFromScaledKey(NewKey, bPositive));
			AddRelevantModifiers(InputActionData, NewMapping);

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
		for (const int32 MappingIndex : MappingsForAction)
		{
			if (InputContext->GetMapping(MappingIndex).Key == OppositeAxis)
			{
				InputContext->UnmapKey(InputActionData.Action, NewMappingKey);
				InputContext->UnmapKey(InputActionData.Action, OppositeAxis);
				InputContext->MapKey(InputActionData.Action, Container->UINavPC->GetAxis2DFromAxis1D(OppositeAxis));
				break;
			}
		}
	}
}

void UUINavInputBox::UnmapEnhancedAxisKey(const FKey& NewAxisKey, const FKey& OldAxisKey, const FKey& NewKey, const int32 Index)
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
			InputContext->UnmapKey(InputActionData.Action, Container->UINavPC->GetOppositeAxisKey(NewKey));
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

void UUINavInputBox::CancelUpdateInputKey(const ERevertRebindReason Reason)
{
	Container->OnRebindCancelled(Reason, AwaitingNewKey);
	RevertToKeyText(AwaitingIndex);
	AwaitingIndex = -1;
}

FKey UUINavInputBox::GetKeyFromAxis(const FKey AxisKey) const
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
	InputText->SetText(InputActionData.DisplayName);
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

	InputButtons[Index]->NavText->SetText(Container->PressKeyText);

	if (bUsingKeyImage[Index])
	{
		InputButtons[Index]->InputImage->SetVisibility(ESlateVisibility::Collapsed);
		InputButtons[Index]->NavText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
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
			Container->GetAxisPropertiesFromMapping(ActionMapping, bPositive, ActionAxis);
			if (ActionAxis == Axis && Container->RespectsRestriction(ActionMapping.Key, Index))
			{
				OutMappingIndices.Add(i);
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
	UTexture2D* NewTexture = Container->UINavPC->GetKeyIcon(Keys[Index]);
	if (NewTexture != nullptr)
	{
		InputButtons[Index]->InputImage->SetBrushFromTexture(NewTexture);
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
		InputButtons[Index]->NavText->SetVisibility(ESlateVisibility::Collapsed);
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

	InputButtons[Index]->NavText->SetText(OldName);
}

int UUINavInputBox::ContainsKey(const FKey CompareKey) const
{
	return Keys.IndexOfByKey(CompareKey);
}

bool UUINavInputBox::WantsAxisKey() const
{
	return IS_AXIS && InputActionData.AxisScale == EAxisType::None;
}

