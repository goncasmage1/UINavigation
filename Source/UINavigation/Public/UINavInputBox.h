// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "Data/AxisType.h"
#include "Data/InputContainerEnhancedActionData.h"
#include "Data/InputRebindData.h"
#include "Data/RevertRebindReason.h"
#include "EnhancedActionKeyMapping.h"
#include "UINavInputBox.generated.h"

#define IS_AXIS (AxisType != EAxisType::None)

class UUINavInputComponent;
class UInputAction;
class UInputMappingContext;
class UInputSettings;

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
	int GetNumValidKeys(const int Index) const;

	void GetEnhancedMappingsForAction(const UInputAction* Action, const EInputAxis& Axis, const int Index, TArray<int32>& OutMappingIndices);
	void GetMappingsForAction(const UInputSettings* const Settings, const FName ActionName, const int Index, TArray<int32>& OutMappingIndices);
	void GetMappingsForAxis(const UInputSettings* const Settings, const FName AxisName, const bool bPositive, const int Index, TArray<int32>& OutMappingIndices);

public:

	virtual void NativeConstruct() override;
	void CreateEnhancedInputKeyWidgets();
	void CreateInputKeyWidgets();

	void CreateKeyWidgets();
	bool TrySetupNewKey(const FKey NewKey, const int KeyIndex, const UUINavInputComponent* const NewInputButton);
	void ResetKeyWidgets();
	void UpdateInputKey(const FKey NewKey, const int Index, const bool bSkipChecks = false);
	void FinishUpdateNewKey();
	void FinishUpdateNewEnhancedInputKey(const FKey PressedKey, int Index);
	void FinishUpdateNewInputKey(const FKey PressedKey, int Index);
	void TryMapEnhancedAxisKey(const FKey& NewKey, const int32 Index);
	void TryMapInputAxisKey(UInputSettings* Settings, const FKey& NewKey, const int32 Index);
	void TryMap2DAxisKey(const FKey& NewMappingKey, const int Index);
	void UnmapEnhancedAxisKey(const FKey& NewAxisKey, const FKey& OldAxisKey, const FKey& NewKey, const int32 Index);
	void UnmapInputAxisKey(UInputSettings* Settings, const FKey& NewAxisKey, const FKey& OldAxisKey, const FInputAxisKeyMapping& AxisMapping, const int32 Index);
	void AddRelevantModifiers(const FInputContainerEnhancedActionData& ActionData, FEnhancedActionKeyMapping& Mapping);
	void CancelUpdateInputKey(const ERevertRebindReason Reason);
	void RevertToKeyText(const int Index);

	void NotifySelected(const int Index);

	int ContainsKey(const FKey CompareKey) const;
	FORCEINLINE bool IsAxis() const { return IS_AXIS; }
	FORCEINLINE bool WantsAxisKey() const;
	FORCEINLINE FKey GetKey(const int Index) { return Index >= 0 && Index < Keys.Num() ? Keys[Index] : FKey(); }

	EAxisType AxisType = EAxisType::None;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
	class UUINavInputComponent* InputButton1 = nullptr;
		
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
	class UUINavInputComponent* InputButton2 = nullptr;
		
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
	class UUINavInputComponent* InputButton3 = nullptr;

	TArray<class UUINavInputComponent*> InputButtons;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
	class UTextBlock* InputText = nullptr;

	UPROPERTY()
	class UUINavInputContainer* Container = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	FName InputName;
	TArray<int> EnhancedInputGroups;

	UPROPERTY(BlueprintReadOnly, Category = "Enhanced Input")
	UInputMappingContext* InputContext = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Enhanced Input")
	FInputContainerEnhancedActionData InputActionData;
	
	FInputRebindData InputData = FInputRebindData();

	int KeysPerInput = 2;
};
