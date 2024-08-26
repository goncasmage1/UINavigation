// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Blueprint/UserWidget.h"
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

	bool bAwaitingNewKey = false;
	FKey AwaitingNewKey = FKey();

	virtual FNavigationReply NativeOnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply) override;

	bool UpdateKeyIconForKey();
	FText GetKeyText();
	void UpdateKeyDisplay();
	void ProcessInputName(const UInputAction* Action);

	FText ActionDisplayName;

	UFUNCTION()
	void InputComponentClicked();

public:

	UUINavInputBox(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	void CreateEnhancedInputKeyWidgets();

	void CreateKeyWidgets();
	bool TrySetupNewKey(const FKey& NewKey);
	void ResetKeyWidgets();
	void UpdateInputKey(const FKey& NewKey, const bool bSkipChecks = false);
	void FinishUpdateNewKey();
	void FinishUpdateNewEnhancedInputKey(const FKey& PressedKey);
	void CancelUpdateInputKey(const ERevertRebindReason Reason);
	void RevertToKeyText();

	FText GetCurrentText() const;

	bool ContainsKey(const FKey& CompareKey) const;
	FORCEINLINE FKey GetKey() { return CurrentKey; }

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
	class UUINavInputComponent* InputButton = nullptr;

	UPROPERTY()
	class UUINavInputContainer* Container = nullptr;

	UPROPERTY()
	FName InputName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	TArray<int> EnhancedInputGroups;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	UInputMappingContext* InputContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	EInputRestriction InputRestriction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	FName PlayerMappableKeySettingsName;

	FInputRebindData InputData = FInputRebindData();
};
