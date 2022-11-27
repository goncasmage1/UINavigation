// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

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

#define IS_ENHANCED_INPUT IsValid(InputContext) && IsValid(InputActionData.Action)

void UUINavInputBox::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = false;
}

void UUINavInputBox::CreateKeyWidgets()
{
	InputButtons = { InputButton1, InputButton2, InputButton3 };
	ProcessInputName();

	if (IS_ENHANCED_INPUT)
	{
		CreateEnhancedInputKeyWidgets();
	}
	else
	{
		CreateInputKeyWidgets();
	}
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
				GetActionMappingsForAction(ActionMapping.Action, InputActionData.Axis, MappingsForAction);
				UUINavInputBox* OppositeInputBox = Container->GetOppositeInputBox(InputActionData);
				FKey NewKey = ActionMapping.Key;

				if ((InputActionData.Axis == Axis || Container->UINavPC->IsAxis2D(NewKey)) &&
					(OppositeInputBox == nullptr || !OppositeInputBox->Keys.IsValidIndex(j) || OppositeInputBox->Keys[j] != ActionMapping.Key) &&
					(MappingsForAction.Num() < 2 || GetNumValidKeys() < (MappingsForAction.Num() / 2)))
				{
					if (Container->UINavPC->IsAxis(NewKey) && InputActionData.AxisScale != EAxisType::None)
					{
						NewKey = Container->UINavPC->GetKeyFromAxis(NewKey, bPositive ? IS_POSITIVE_AXIS : !IS_POSITIVE_AXIS, InputActionData.Axis);
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

void UUINavInputBox::CreateInputKeyWidgets()
{
	const UInputSettings* Settings = GetDefault<UInputSettings>();
	TArray<FInputActionKeyMapping> Actions;
	TArray<FInputAxisKeyMapping> Axes;

	if (IS_AXIS) Settings->GetAxisMappingByName(InputName, Axes);
	else Settings->GetActionMappingByName(InputName, Actions);

	if ((IS_AXIS && Axes.Num() == 0) || 
		(!IS_AXIS && Actions.Num() == 0))
	{
		FString Message = TEXT("Couldn't find Input with name ");
		Message.Append(*InputName.ToString());
		DISPLAYERROR(Message);
		return;
	}

	for (int j = 0; j < 3; j++)
	{
		UUINavInputComponent* NewInputButton = InputButtons[j];

		if (j < KeysPerInput)
		{
			const int Iterations = IS_AXIS ? Axes.Num() : Actions.Num();
			FKey PotentialAxisKey;
			for (int i = Iterations - 1; i >= 0; --i)
			{
				if ((IS_AXIS && !PotentialAxisKey.IsValid() && IS_RIGHT_SCALE(Axes[i])) ||
					(!IS_AXIS))
				{
					const FKey NewKey = IS_AXIS ? GetKeyFromAxis(Axes[i].Key) : Actions[i].Key;
					if (!Container->RespectsRestriction(NewKey, j))
					{
						continue;
					}

					if (TrySetupNewKey(NewKey, j, NewInputButton))
					{
						break;
					}
				}
				else if (IS_AXIS && !PotentialAxisKey.IsValid() && Container->RespectsRestriction(Axes[i].Key, j))
				{
					FKey NewPotentialKey = Container->UINavPC->GetKeyFromAxis(Axes[i].Key, AxisType == EAxisType::Positive);
					if (!Keys.Contains(NewPotentialKey))
					{
						PotentialAxisKey = NewPotentialKey;
					}
				}
			}
			if (PotentialAxisKey.IsValid())
			{
				TrySetupNewKey(PotentialAxisKey, j, NewInputButton);
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

	if (Container->UINavPC != nullptr && Container->UINavPC->KeyMap.Contains(InputName.ToString()))
	{
		Container->UINavPC->KeyMap.Add(InputName.ToString(), Keys);
	}
}

void UUINavInputBox::UpdateInputKey(const FKey NewKey, const int Index, const bool bSkipChecks)
{
	AwaitingNewKey = NewKey;
	AwaitingIndex = Index;

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

			if (IS_ENHANCED_INPUT)
			{
				FInputRebindData CollidingInputData;
				Container->GetEnhancedInputRebindData(CollidingActionIndex, CollidingInputData);
				if (!Container->GetParentWidget()->UINavInputBoxes.Find(this, SelfIndex) ||
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
			}
			else
			{
				FInputRebindData CollidingInputData;
				Container->GetInputRebindData(CollidingActionIndex, CollidingInputData);
				if (!Container->GetParentWidget()->UINavInputBoxes.Find(this, SelfIndex) ||
					!Container->RequestKeySwap(FInputCollisionData(InputData.InputText,
																   CollidingInputData.InputText,
																   CollidingKeyIndex,
																   Keys[Index],
																   NewKey),
											   SelfIndex,
											   CollidingActionIndex))
				{
					CancelUpdateInputKey(RevertReason);
				}
			}

			return;
		}
		else if (Keys.Contains(NewKey))
		{
			CancelUpdateInputKey(ERevertRebindReason::UsedBySameInput);
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
	const FKey NewKey = AwaitingNewKey;
	const int Index = AwaitingIndex;

	if (IS_ENHANCED_INPUT)
	{
		FinishUpdateNewEnhancedInputKey(NewKey, Index);
	}
	else
	{
		FinishUpdateNewInputKey(NewKey, Index);
	}
}

void UUINavInputBox::FinishUpdateNewEnhancedInputKey(const FKey NewKey, const int Index)
{
	const TArray<FEnhancedActionKeyMapping>& ActionMappings = InputContext->GetMappings();

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
			bool bPositive;
			EInputAxis Axis = InputActionData.Axis;
			Container->GetAxisPropertiesFromMapping(ActionMapping, bPositive, Axis);
			if (InputActionData.Axis == Axis)
			{
				const FKey InputKey = GetKeyFromAxis(ActionMapping.Key);
				if (Container->RespectsRestriction(NewKey, Index))
				{
					if (InputKey == Keys[Index])
					{
						if (IS_AXIS)
						{
							if (Container->UINavPC->IsAxis(ActionMapping.Key))
							{
								if (InputActionData.AxisScale != EAxisType::None)
								{
									bool bNewKeyPositive = true;
									if (Container->UINavPC->GetAxisFromKey(NewKey, bNewKeyPositive).IsValid())
									{
										NewAxisKey = NewKey;
									}
									else
									{
										OldAxisKey = ActionMapping.Key;
									}
									break;
								}
							}

							ActionMapping.Key = NewKey;
						}
						else ActionMapping.Key = NewKey;
						Keys[Index] = NewKey;
						InputButtons[Index]->NavText->SetText(GetKeyText(Index));
						bFound = true;
						break;
					}
					else if (!Container->UINavPC->IsAxis(ActionMapping.Key) &&
						Container->GetOppositeInputBox(InputActionData) != nullptr)
					{
						bool bOtherKeyPositive = true;
						const FKey OtherKeyAxis = Container->UINavPC->GetAxisFromKey(ActionMapping.Key, bOtherKeyPositive);
						bool bNewKeyPositive = true;
						const FKey NewKeyAxis = Container->UINavPC->GetAxisFromKey(NewKey, bNewKeyPositive);

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

	const FKey NewMappingKey = ActionMappings[i].Key;

	if (!bFound)
	{
		// Remove old key
		if (InputActionData.AxisScale == EAxisType::Positive)
		{
			UnmapAxisKey(NewAxisKey, OldAxisKey, NewKey, NewMappingKey, Index);
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
			UnmapAxisKey(NewAxisKey, OldAxisKey, NewKey, NewMappingKey, Index);
			bRemoved2DAxis = true;
		}

		Keys[Index] = NewKey;
		InputButtons[Index]->NavText->SetText(GetKeyText(Index));

		TryMap2DAxisKey(NewMapping.Key);
	}
	else
	{
		// Check if the current setup allows replacing 2 keys with an axis key
		TryMapAxisKey(NewKey, NewMappingKey, Index);
	}

	if (Container->UINavPC != nullptr)
	{
		Container->UINavPC->PressedActions.Empty();
		if (Container->UINavPC->KeyMap.Contains(InputName.ToString()))
		{
			Container->UINavPC->KeyMap.Add(InputName.ToString(), Keys);
		}
		Container->UINavPC->UnbindMouseWorkaround();
	}

	UpdateKeyDisplay(Index);

	if (bRemoved2DAxis)
	{
		Container->ResetInputBox(InputName, GET_REVERSE_AXIS);
	}
}

void UUINavInputBox::FinishUpdateNewInputKey(const FKey NewKey, const int Index)
{
	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	TArray<FInputActionKeyMapping>& Actions = const_cast<TArray<FInputActionKeyMapping>&>(Settings->GetActionMappings());
	TArray<FInputAxisKeyMapping>& Axes = const_cast<TArray<FInputAxisKeyMapping>&>(Settings->GetAxisMappings());

	// TODO: Copy new logic into here
	// TODO: Reuse input modifier functions

	FKey NewAxisKey;
	bool bFound = false;
	bool bRemoved2DAxis = false;
	const int Iterations = IS_AXIS ? Axes.Num() : Actions.Num();
	int i = Iterations - 1;
	for (; i >= 0; --i)
	{
		if ((IS_AXIS && Axes[i].AxisName.IsEqual(InputName)) ||
			(!IS_AXIS && Actions[i].ActionName.IsEqual(InputName)))
		{
			if (Container->RespectsRestriction(NewKey, Index))
			{
				FKey InputKey = IS_AXIS ? GetKeyFromAxis(Axes[i].Key) : Actions[i].Key;
				if (InputKey == Keys[Index])
				{
					if (IS_AXIS)
					{
						if (InputKey != Axes[i].Key &&
							!IS_RIGHT_SCALE(Axes[i]))
						{
							NewAxisKey = NewKey;
							break;
						}

						//Remove indirect axis in opposite scale input
						if (Container->UINavPC->IsAxis(Axes[i].Key))
						{
							bRemoved2DAxis = true;
						}

						Axes[i].Key = NewKey;
					}
					else Actions[i].Key = NewKey;
					Keys[Index] = NewKey;
					InputButtons[Index]->NavText->SetText(GetKeyText(Index));
					bFound = true;
					break;
				}
				else if (!Container->UINavPC->IsAxis(Axes[i].Key) &&
						Container->GetOppositeInputBox(InputName, AxisType) != nullptr)
				{
					bool bOtherKeyPositive = true;
					const FKey OtherKeyAxis = Container->UINavPC->GetAxisFromKey(Axes[i].Key, bOtherKeyPositive);
					bool bNewKeyPositive = true;
					const FKey NewKeyAxis = Container->UINavPC->GetAxisFromKey(NewKey, bNewKeyPositive);

					if (OtherKeyAxis == NewKeyAxis &&
						bOtherKeyPositive != bNewKeyPositive &&
						AxisType != EAxisType::None &&
						(AxisType == EAxisType::Positive) == bNewKeyPositive)
					{
						NewAxisKey = NewKeyAxis;
						break;
					}
				}
			}
		}
	}
	if (!bFound)
	{
		if (IS_AXIS)
		{
			const FKey AxisKey = NewAxisKey.IsValid() ? NewAxisKey : NewKey;

			if (NewAxisKey.IsValid())
			{
				Settings->RemoveAxisMapping(Axes[i]);
				bRemoved2DAxis = true;
			}

			FInputAxisKeyMapping Axis;
			Axis.Key = AxisKey;
			Axis.AxisName = InputName;
			Axis.Scale = NewAxisKey.IsValid() ? 1.0f : (AxisType == EAxisType::Positive ? 1.0f : -1.0f);
			Settings->AddAxisMapping(Axis, true);

			Keys[Index] = AxisKey;
			InputButtons[Index]->NavText->SetText(GetKeyText(Index));
		}
		else
		{
			FInputActionKeyMapping Action = FInputActionKeyMapping();
			Action.ActionName = InputName;
			Action.Key = NewKey;
			Settings->AddActionMapping(Action, true);
			Keys[Index] = NewKey;
			InputButtons[Index]->NavText->SetText(GetKeyText(Index));
		}
	}

	if (Container->UINavPC != nullptr)
	{
		Container->UINavPC->PressedActions.Empty();
		if (Container->UINavPC->KeyMap.Contains(InputName.ToString()))
		{
			Container->UINavPC->KeyMap.Add(InputName.ToString(), Keys);
		}
		Container->UINavPC->UnbindMouseWorkaround();
	}

	UpdateKeyDisplay(Index);

	Settings->SaveConfig();
	Settings->ForceRebuildKeymaps();

	if (bRemoved2DAxis)
	{
		Container->ResetInputBox(InputName, GET_REVERSE_AXIS);
	}
}

void UUINavInputBox::TryMapAxisKey(const FKey& NewKey, const FKey& ActionMappingKey, const int32 Index)
{
	UUINavInputBox* OppositeInputBox = Container->GetOppositeInputBox(InputActionData);
	if (OppositeInputBox != nullptr)
	{
		const FKey OppositeInputBoxKey = OppositeInputBox->Keys.IsValidIndex(Index) ? OppositeInputBox->Keys[Index] : FKey();
		const FKey NewOppositeKey = Container->UINavPC->GetOppositeAxisKey(NewKey);
		if (NewOppositeKey == OppositeInputBoxKey)
		{
			InputContext->UnmapKey(InputActionData.Action, NewOppositeKey);
			InputContext->UnmapKey(InputActionData.Action, ActionMappingKey);
			bool bPositive;
			FEnhancedActionKeyMapping& NewMapping = InputContext->MapKey(InputActionData.Action, Container->UINavPC->GetAxisFromKey(NewKey, bPositive));
			AddRelevantModifiers(InputActionData, NewMapping);

			TryMap2DAxisKey(NewMapping.Key);
		}
	}
}

void UUINavInputBox::TryMap2DAxisKey(const FKey& NewMappingKey)
{
	const FKey OppositeAxis = Container->UINavPC->GetOppositeAxis2DAxis(NewMappingKey);
	if (OppositeAxis.IsValid())
	{
		TArray<int32> MappingsForAction;
		const EInputAxis Axis = InputActionData.Axis == EInputAxis::X ? EInputAxis::Y : EInputAxis::X;
		GetActionMappingsForAction(InputActionData.Action, Axis, MappingsForAction);
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

void UUINavInputBox::UnmapAxisKey(const FKey& NewAxisKey, const FKey& OldAxisKey, const FKey& NewKey, const FKey& ActionMappingKey, const int32 Index)
{
	UUINavInputBox* OppositeInputBox = Container->GetOppositeInputBox(InputActionData);
	if (OppositeInputBox != nullptr)
	{
		const FKey OppositeInputBoxKey = OppositeInputBox->Keys.IsValidIndex(Index) ? OppositeInputBox->Keys[Index] : FKey();
		const FKey CurrentOppositeKey = Container->UINavPC->GetOppositeAxisKey(Keys[Index]);
		const FKey NewOppositeKey = Container->UINavPC->GetOppositeAxisKey(NewKey);
		if (OldAxisKey.IsValid())
		{
			InputContext->UnmapKey(InputActionData.Action, ActionMappingKey);
			FEnhancedActionKeyMapping& NewMapping = InputContext->MapKey(InputActionData.Action, OppositeInputBoxKey);
			AddRelevantModifiers(OppositeInputBox->InputActionData, NewMapping);

			if (Container->UINavPC->IsAxis2D(ActionMappingKey))
			{
				const EInputAxis OppositeAxis = InputActionData.Axis == EInputAxis::X ? EInputAxis::Y : EInputAxis::X;
				const FKey NewAxis1D = Container->UINavPC->GetAxis1DFromAxis2D(ActionMappingKey, OppositeAxis);
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
}

FKey UUINavInputBox::GetKeyFromAxis(const FKey AxisKey) const
{
	FKey NewKey = Container->UINavPC->GetKeyFromAxis(AxisKey, AxisType == EAxisType::Positive);
	if (!NewKey.IsValid())
	{
		NewKey = AxisKey;
	}
	return NewKey;
}

void UUINavInputBox::ProcessInputName()
{
	if (IS_ENHANCED_INPUT)
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
	else
	{
		FString InputNameString = InputName.ToString();
		const FString LastNameChar = InputNameString.Right(1);
		if (LastNameChar.Equals(TEXT("+")))
		{
			AxisType = EAxisType::Positive;
			InputNameString.RemoveAt(InputNameString.Len() - 1);
			InputName = FName(*InputNameString);
		}
		else if (LastNameChar.Equals(TEXT("-")))
		{
			AxisType = EAxisType::Negative;
			InputNameString.RemoveAt(InputNameString.Len() - 1);
			InputName = FName(*InputNameString);
		}

		if (InputData.InputText.IsEmpty())
		{
			InputData.InputText = FText::FromName(InputName);
		}
		InputText->SetText(InputData.InputText);
	}
}

int UUINavInputBox::GetNumValidKeys() const
{
	int ValidKeys = 0;
	for (const FKey& Key : Keys)
	{
		if (Key.IsValid())
		{
			++ValidKeys;
		}
	}
	return ValidKeys;
}

void UUINavInputBox::GetActionMappingsForAction(const UInputAction* Action, const EInputAxis& Axis, TArray<int32>& OutMappingIndices)
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
			if (ActionAxis == Axis)
			{
				OutMappingIndices.Add(i);
			}
		}
	}

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

	if (Container->UINavPC != nullptr)
	{
		Container->UINavPC->PressedActions.Empty();
		Container->UINavPC->UnbindMouseWorkaround();
	}
}

void UUINavInputBox::NotifySelected(const int Index)
{
	InputButtons[Index]->NavText->SetText(Container->PressKeyText);

	if (Container->UINavPC != nullptr) Container->UINavPC->BindMouseWorkaround();

	if (bUsingKeyImage[Index])
	{
		InputButtons[Index]->InputImage->SetVisibility(ESlateVisibility::Collapsed);
		InputButtons[Index]->NavText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

int UUINavInputBox::ContainsKey(const FKey CompareKey) const
{
	return Keys.IndexOfByKey(CompareKey);
}

bool UUINavInputBox::WantsAxisKey() const
{
	return IS_AXIS && (!IS_ENHANCED_INPUT || InputActionData.AxisScale == EAxisType::None);
}

