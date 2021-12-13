// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "Data/AxisType.h"
#include "Data/InputContainerEnhancedActionData.h"
#include "Data/InputRebindData.h"
#include "Data/RevertRebindReason.h"
#include "UINavInputBox.generated.h"

#define IS_AXIS (AxisType != EAxisType::None)

class UUINavInputComponent;
class UInputAction;
class UInputMappingContext;

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

	bool UpdateKeyIconForKey(const int Index);
	FText GetKeyText(const int Index);
	void UpdateKeyDisplay(const int Index);
	FKey GetKeyFromAxis(FKey AxisKey) const;
	void ProcessInputName();

public:

	virtual void NativeConstruct() override;
	void CreateEnhancedInputKeyWidgets();
	void CreateInputKeyWidgets();

	void CreateKeyWidgets();
	bool TrySetupNewKey(const FKey NewKey, const int KeyIndex, const UUINavInputComponent* const NewInputButton);
	void ResetKeyWidgets();
	void UpdateInputKey(const FKey NewKey, const int Index, const bool bSkipChecks = false);
	void FinishUpdateNewKey();
	void FinishUpdateNewEnhancedInputKey(FKey NewKey, int Index);
	void FinishUpdateNewInputKey(FKey NewKey, int Index);
	void CancelUpdateInputKey(const ERevertRebindReason Reason);
	void RevertToKeyText(const int Index);

	void NotifySelected(const int Index);

	int ContainsKey(const FKey CompareKey) const;
	FORCEINLINE bool IsAxis() const { return IS_AXIS; }
	FORCEINLINE FKey GetKey(const int Index) { return Index >= 0 && Index < Keys.Num() ? Keys[Index] : FKey(); }

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

	UPROPERTY()
	class UUINavInputContainer* Container = nullptr;

	FName InputName;
	int32 FirstMappingIndex = -1;
	TArray<int> EnhancedInputGroups;

	UPROPERTY(BlueprintReadOnly, Category = "Enhanced Input")
	UInputMappingContext* InputContext = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Enhanced Input")
	FInputContainerEnhancedActionData InputActionData;
	
	FInputRebindData InputData = FInputRebindData();

	int KeysPerInput = 2;
};
