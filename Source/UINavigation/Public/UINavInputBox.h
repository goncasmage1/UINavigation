// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "Data/AxisType.h"
#include "Data/InputContainerEnhancedActionData.h"
#include "Data/InputRebindData.h"
#include "Data/InputRestriction.h"
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
	FKey CurrentKey = FKey();
	bool bUsingKeyImage;

	FKey AwaitingNewKey = FKey();
	int8 AwaitingIndex = -1;

	virtual FNavigationReply NativeOnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply) override;

	bool UpdateKeyIconForKey(const int Index);
	FText GetKeyText(const int Index);
	void UpdateKeyDisplay(const int Index);
	FKey GetKeyFromAxis(const FKey& AxisKey) const;
	void ProcessInputName();

	UFUNCTION()
	void InputComponentClicked();
		
	void InputComponentClicked(const int Index);

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
	void CancelUpdateInputKey(const ERevertRebindReason Reason);
	void RevertToKeyText(const int Index);

	FText GetCurrentText() const;

	int ContainsKey(const FKey& CompareKey) const;
	FORCEINLINE bool IsAxis() const { return IS_AXIS; }
	FORCEINLINE bool WantsAxisKey() const;
	FORCEINLINE FKey GetKey(const int Index) { return Index >= 0 ? CurrentKey : FKey(); }

	EAxisType AxisType = EAxisType::None;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
	class UUINavInputComponent* InputButton = nullptr;
		
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "UINav Input")
	class UTextBlock* InputText = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "UINav Input")
	class URichTextBlock* InputRichText = nullptr;

	UPROPERTY()
	class UUINavInputContainer* Container = nullptr;

	UPROPERTY()
	FName InputName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	TArray<int> EnhancedInputGroups;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	UInputMappingContext* InputContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	FInputContainerEnhancedActionData InputActionData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINavPromptWidget")
	FString InputTextStyleRowName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	EInputRestriction InputRestriction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	FName PlayerMappableKeySettingsName;

	FInputRebindData InputData = FInputRebindData();

	int KeysPerInput = 2;
};
