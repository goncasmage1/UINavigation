// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Data/AxisType.h"
#include "Data/InputCollisionData.h"
#include "Data/InputRebindData.h"
#include "Data/InputRestriction.h"
#include "Data/RevertRebindReason.h"
#include "Blueprint/UserWidget.h"
#include "Data/InputContainerEnhancedActionData.h"
#include "EnhancedActionKeyMapping.h"
#include "UINavWidget.h"
#include "UINavInputContainer.generated.h"

class UPromptDataBase;
class FReply;
struct FGeometry;
struct FFocusEvent;

/**
* This class contains the logic for aggregating several input boxes
*/

UCLASS()
class UINAVIGATION_API UUINavInputContainer : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	void SetupInputBoxes();
	void CreateInputBoxes();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
	class UPanelWidget* InputBoxesPanel = nullptr;

	class UUINavWidget* ParentWidget = nullptr;

public:

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

	/**
	*	Called when a new input box is added
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
	void OnAddInputBox(class UUINavInputBox* NewInputBox);

	virtual void OnAddInputBox_Implementation(class UUINavInputBox* NewInputBox);

	/*
	*	Called when key was successfully rebinded
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
	void OnKeyRebinded(FName InputName, FKey OldKey, FKey NewKey);

	virtual void OnKeyRebinded_Implementation(FName InputName, FKey OldKey, FKey NewKey);

	/*
	*	Called when a rebind was cancelled, specifying the reason for the revert
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
	void OnRebindCancelled(ERevertRebindReason RevertReason, FKey PressedKey);

	virtual void OnRebindCancelled_Implementation(ERevertRebindReason RevertReason, FKey PressedKey);

	/**
	*	Called when the player presses a key being used by another action
	*/
	bool RequestKeySwap(const FInputCollisionData& InputCollisionData, const int CurrentInputIndex, const int CollidingInputIndex) const;

	UFUNCTION(BlueprintCallable, Category = "UINav Input")
	void ResetKeyMappings();

	UFUNCTION(BlueprintCallable, Category = "UINav Input")
	void ForceUpdateInputBoxes();

	UFUNCTION(BlueprintCallable, Category = "UINav Input")
	UUINavInputBox* GetInputBoxAtIndex(const int Index) const;

	ERevertRebindReason CanRegisterKey(class UUINavInputBox* InputBox, const FKey NewKey, const bool bIsHold, const int Index, int& OutCollidingActionIndex, int& OutCollidingKeyIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINav Input")
	bool CanUseKey(class UUINavInputBox* InputBox, const FKey CompareKey, const bool bIsHold, int& OutCollidingActionIndex, int& OutCollidingKeyIndex) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINav Input")
	bool RespectsRestriction(const FKey CompareKey, const int Index);

	void ResetInputBox(const FName InputName, const EAxisType AxisType);

	UFUNCTION()
	void OnInputTypeChanged(const EInputType InputType);

	UFUNCTION()
	void SwapKeysDecided(const UPromptDataBase* const PromptData);

	UUINavInputBox* GetInputBoxInDirection(UUINavInputBox* InputBox, const EUINavigation Direction);
	
	UUINavInputBox* GetOppositeInputBox(const FInputContainerEnhancedActionData& ActionData);
	UUINavInputBox* GetOppositeInputBox(const FName& InputName, const EAxisType AxisType);

	void GetInputRebindData(const int InputIndex, FInputRebindData& RebindData) const;

	void GetEnhancedInputRebindData(const int InputIndex, FInputRebindData& RebindData) const;

	//-----------------------------------------------------------------------

	class UUINavPCComponent* UINavPC = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "UINav Input")
	int NumberOfInputs = 0;

	UPROPERTY(BlueprintReadOnly, Category = "UINav Input")
	int KeysPerInput = 0;

	UPROPERTY(BlueprintReadOnly, Category = "UINav Input")
	TArray<UUINavInputBox*> InputBoxes;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	TMap<UInputMappingContext*, FInputContainerEnhancedActionDataArray> EnhancedInputs;
	
	/*
	The restrictions for the type of input associated with each column
	in the Input Container
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	TArray<EInputRestriction> InputRestrictions;

	/*
	A list of the keys that the player should only be able to use for the inputs
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	TArray<FKey> KeyWhitelist;
	
	/*
	A list of the keys that the player shouldn't be able to use for the inputs.
	Only used if KeyWhitelist is empty.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	TArray<FKey> KeyBlacklist =
	{
		EKeys::Escape,
		EKeys::LeftCommand,
		EKeys::RightCommand,
	};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	TSubclassOf<class UUINavInputBox> InputBox_BP;

	/*
	The widget class of the widget that will tell the player that 2 keys can be swapped.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	TSubclassOf<class USwapKeysWidget> SwapKeysWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	int SpawnKeysWidgetZOrder = 0;
	
	/*
	Indicates whether unused input boxes will hidden or collapsed
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	bool bCollapseInputBoxes = false;

	FPromptWidgetDecided DecidedCallback;

	//The text used for empty key buttons
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	FText EmptyKeyText = FText::FromString(TEXT("Unbound"));

	//The text used for notifying the player to press a key
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	FText PressKeyText = FText::FromString(TEXT("Press Any Key"));

	//The text used for specifying that an key is used as a Hold
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	FText HoldText = FText::FromString(TEXT("{} (Hold)"));

	//The title text used for the swap keys widget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	FText SwapKeysTitleText = FText::FromString(TEXT("Swap Keys"));

	//The message text used for the swap keys widget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	FText SwapKeysMessageText = FText::FromString(TEXT("{CollidingKey} is already being used by {CollidingAction}.\nDo you want to swap it with {OtherKey}?"));

};
