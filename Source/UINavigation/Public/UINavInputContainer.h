// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Data/AxisType.h"
#include "Data/InputCollisionData.h"
#include "Data/InputRebindData.h"
#include "Data/InputRestriction.h"
#include "Data/RevertRebindReason.h"
#include "Data/TargetColumn.h"
#include "Blueprint/UserWidget.h"
#include "Data/InputContainerEnhancedActionData.h"
#include "EnhancedActionKeyMapping.h"
#include "UINavInputContainer.generated.h"

/**
* This class contains the logic for aggregating several input boxes
*/

UCLASS()
class UINAVIGATION_API UUINavInputContainer : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	void SetupInputBoxes(const int GridIndex);
	void CreateInputBoxes(const int GridIndex);

	//-----------------------------------------------------------------------

	TMap<FKey, FKey> KeyToAxisMap = {
		{EKeys::Gamepad_LeftTrigger, EKeys::Gamepad_LeftTriggerAxis},
		{EKeys::Gamepad_RightTrigger, EKeys::Gamepad_RightTriggerAxis},
		{EKeys::MixedReality_Left_Trigger_Click, EKeys::MixedReality_Left_Trigger_Axis},
		{EKeys::MixedReality_Right_Trigger_Click, EKeys::MixedReality_Right_Trigger_Axis},
		{EKeys::OculusTouch_Left_Grip_Click, EKeys::OculusTouch_Left_Grip_Axis},
		{EKeys::OculusTouch_Right_Grip_Click, EKeys::OculusTouch_Right_Grip_Axis},
		{EKeys::ValveIndex_Left_Trigger_Click, EKeys::ValveIndex_Left_Trigger_Axis},
		{EKeys::ValveIndex_Right_Trigger_Click, EKeys::ValveIndex_Right_Trigger_Axis},
		{EKeys::Vive_Left_Trigger_Click, EKeys::Vive_Left_Trigger_Axis},
		{EKeys::Vive_Right_Trigger_Click, EKeys::Vive_Right_Trigger_Axis},
	};

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
	class UPanelWidget* InputBoxesPanel = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "UINav Input")
	class UUINavWidget* ParentWidget = nullptr;

public:

	void Init(class UUINavWidget* NewParent, const int GridIndex);

	/**
	*	Called when this widget completed setting up InputBoxes
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
	void OnSetupCompleted();

	virtual void OnSetupCompleted_Implementation();

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

	ERevertRebindReason CanRegisterKey(const class UUINavInputBox* InputBox, const FKey NewKey, const int Index, int& OutCollidingActionIndex, int& OutCollidingKeyIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINav Input")
	bool CanUseKey(const class UUINavInputBox* InputBox, const FKey CompareKey, int& OutCollidingActionIndex, int& OutCollidingKeyIndex) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINav Input")
	bool RespectsRestriction(const FKey CompareKey, const int Index);

	void ResetInputBox(const FName InputName, const EAxisType AxisType);

	UUINavInputBox* GetOppositeInputBox(const FInputContainerEnhancedActionData& ActionData);
	UUINavInputBox* GetOppositeInputBox(const FName& InputName, const EAxisType AxisType);

	void GetAxisPropertiesFromMapping(const FEnhancedActionKeyMapping& ActionMapping, bool& bOutPositive, EInputAxis& OutAxis) const;

	//Fetches the index offset from the TargetColumn variable for both the top and bottom of the Input Container
	int GetOffsetFromTargetColumn(const bool bTop) const;

	void GetInputRebindData(const int InputIndex, FInputRebindData& RebindData) const;

	void GetEnhancedInputRebindData(const int InputIndex, FInputRebindData& RebindData) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINav Input")
	FKey GetAxisFromKey(FKey Key);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINav Input")
	FORCEINLINE ETargetColumn GetTargetColumn() const { return TargetColumn; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINav Input")
	FORCEINLINE class UUINavWidget* GetParentWidget() const { return ParentWidget; }

	//-----------------------------------------------------------------------

	UPROPERTY()
	class UUINavPCComponent* UINavPC = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "UINav Input")
	int NumberOfInputs = 0;

	UPROPERTY(BlueprintReadOnly, Category = "UINav Input")
	int KeysPerInput = 0;

	//The index of the button in the top left corner of the grid
	UPROPERTY(BlueprintReadOnly, Category = "UINav Input")
	int FirstButtonIndex = -1;

	//The index of the button in the bottom right corner of the grid
	UPROPERTY(BlueprintReadOnly, Category = "UINav Input")
	int LastButtonIndex = -1;

	/*The index of the button at the top of the grid that should be navigated to
	when entering this grid*/
	UPROPERTY(BlueprintReadOnly, Category = "UINav Input")
	int TopButtonIndex = -1;

	/*The index of the button at the bottom of the grid that should be navigated to
	when entering this grid*/
	UPROPERTY(BlueprintReadOnly, Category = "UINav Input")
	int BottomButtonIndex = -1;

	/*
	The names of the desired actions and axes to allow for rebinding.
	If you want to rebind axes, you have to specify whether they're
	the positive or negative axis by suffixing the axis name with
	either "+" or "-"
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	TArray<FName> InputNames;
	
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

	//Indicates which column to navigate to when navigating to this Input Container
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	ETargetColumn TargetColumn = ETargetColumn::Left;

	//The text used for empty key buttons
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	FText EmptyKeyText = FText::FromString(TEXT("Unbound"));

	//The text used for notifying the player to press a key
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	FText PressKeyText = FText::FromString(TEXT("Press Any Key"));

};
