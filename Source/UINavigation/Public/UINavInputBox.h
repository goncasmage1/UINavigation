// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

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
class UTextBlock;
class URichTextBlock;
struct FInputAxisKeyMapping;

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

	FKey AwaitingNewKey = FKey();
	int8 AwaitingIndex = -1;

	virtual FNavigationReply NativeOnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply) override;

	bool UpdateKeyIconForKey(const int Index);
	FText GetKeyText(const int Index);
	void UpdateKeyDisplay(const int Index);
	FKey GetKeyFromAxis(const FKey& AxisKey) const;
	void ProcessInputName();
	int GetNumValidKeys(const int Index) const;

	UFUNCTION()
	void InputComponent1Clicked();
	UFUNCTION()
	void InputComponent2Clicked();
	UFUNCTION()
	void InputComponent3Clicked();
		
	void InputComponentClicked(const int Index);

	void GetEnhancedMappingsForAction(const UInputAction* Action, const EInputAxis& Axis, const int Index, TArray<int32>& OutMappingIndices);
	void GetKeyMappingNegateAxes(const FKey& OldAxisKey, bool& bNegateX, bool& bNegateY, bool& bNegateZ);

public:

	UUINavInputBox(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	void CreateEnhancedInputKeyWidgets();

	void CreateKeyWidgets();
	bool TrySetupNewKey(const FKey& NewKey, const int KeyIndex, UUINavInputComponent* const NewInputButton);
	void ResetKeyWidgets();
	int32 UpdateInputKey(const FKey& NewKey, int Index = -1, const bool bSkipChecks = false, const int32 MappingIndexToIgnore = -1);
	int32 FinishUpdateNewKey(const int32 MappingIndexToIgnore = -1);
	int32 FinishUpdateNewEnhancedInputKey(const FKey& PressedKey, int Index, const int32 MappingIndexToIgnore = -1);
	void TryMapEnhancedAxisKey(const FKey& NewKey, const int32 Index);
	void TryMap2DAxisKey(const FKey& NewMappingKey, const int Index);
	void UnmapEnhancedAxisKey(const FKey& NewAxisKey, const FKey& OldAxisKey, const FKey& NewKey, const int32 Index, const bool bNegateX, const bool bNegateY, const bool bNegateZ);
	void AddRelevantModifiers(const FInputContainerEnhancedActionData& ActionData, FEnhancedActionKeyMapping& Mapping);
	void ApplyNegateModifiers(UUINavInputBox* InputBox, FEnhancedActionKeyMapping& Mapping, const bool bNegateX, const bool bNegateY, const bool bNegateZ);
	void CancelUpdateInputKey(const ERevertRebindReason Reason);
	void RevertToKeyText(const int Index);

	FText GetCurrentText() const;

	int ContainsKey(const FKey& CompareKey) const;
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

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "UINav Input")
	class UTextBlock* InputText = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "UINav Input")
	class URichTextBlock* InputRichText = nullptr;

	UPROPERTY()
	class UUINavInputContainer* Container = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	FName InputName;
	TArray<int> EnhancedInputGroups;

	UPROPERTY(BlueprintReadOnly, Category = "Enhanced Input")
	UInputMappingContext* InputContext = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Enhanced Input")
	FInputContainerEnhancedActionData InputActionData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINavPromptWidget")
	FString InputTextStyleRowName;
	
	FInputRebindData InputData = FInputRebindData();

	int KeysPerInput = 2;
};
