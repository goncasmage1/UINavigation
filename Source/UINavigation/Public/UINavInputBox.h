// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "Data/AxisType.h"
#include "Data/InputIconMapping.h"
#include "Data/InputNameMapping.h"
#include "Data/InputRebindData.h"
#include "Data/RevertRebindReason.h"
#include "UINavInputBox.generated.h"

#define IS_AXIS (AxisType != EAxisType::None)
#define IS_POSITIVE_AXIS (AxisType == EAxisType::Positive)
#define IS_NEGATIVE_AXIS (AxisType == EAxisType::Negative)
#define IS_SCALE_CORRECT(AxisMapping) ((TempAxes[i].Scale > 0.0f && IS_POSITIVE_AXIS) || (TempAxes[i].Scale < 0.0f && IS_NEGATIVE_AXIS))
#define IS_RIGHT_SCALE(Axis) ((Axis.Scale > 0.0f && IS_POSITIVE_AXIS) || (Axis.Scale < 0.0f && IS_NEGATIVE_AXIS))
#define GET_REVERSE_AXIS (AxisType == EAxisType::Positive ? EAxisType::Negative : EAxisType::Positive)

/**
* This class contains the logic for rebinding input keys to their respective actions
*/
UCLASS()
class UINAVIGATION_API UUINavInputBox : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	TArray<FKey> Keys;

	TArray<bool> bUsingKeyImage = { false, false, false };

	FKey AwaitingNewKey;
	int AwaitingIndex;

	bool UpdateKeyIconForKey(int Index);
	FText GetKeyText(int Index);
	void UpdateKeyDisplay(int Index);
	FKey GetKeyFromAxis(FKey AxisKey);
	void ProcessInputName();

public:

	virtual void NativeConstruct() override;

	void CreateKeyWidgets();
	bool TrySetupNewKey(FKey NewKey, int KeyIndex, class UUINavInputComponent* NewInputButton);
	void ResetKeyWidgets();
	void UpdateInputKey(FKey NewKey, int Index, bool bSkipChecks = false);
	void FinishUpdateInputKey();
	void CancelUpdateInputKey(ERevertRebindReason Reason);
	void RevertToKeyText(int Index);

	void NotifySelected(int Index);

	int ContainsKey(FKey CompareKey) const;
	FORCEINLINE bool IsAxis() const { return IS_AXIS; }

	EAxisType AxisType = EAxisType::None;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
		class UUINavInputComponent* InputButton1;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
		class UUINavInputComponent* InputButton2;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
		class UUINavInputComponent* InputButton3;

	TArray<class UUINavInputComponent*> InputButtons;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
		class UTextBlock* InputText;

	class UUINavInputContainer* Container;

	FName InputName;
	FInputRebindData InputData = FInputRebindData();

	int KeysPerInput = 2;
};
