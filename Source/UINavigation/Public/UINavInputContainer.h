// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Data/InputRestriction.h"
#include "Data/RevertRebindReason.h"
#include "Data/TargetColumn.h"
#include "Blueprint/UserWidget.h"
#include "UINavInputContainer.generated.h"

/**
* This class contains the logic for aggregating several input boxes
*/
UCLASS()
class UINAVIGATION_API UUINavInputContainer : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	void CreateInputBoxes();
	void CreateActionBoxes();
	void CreateAxisBoxes();

	//-----------------------------------------------------------------------

	TArray<FKey> FullRangeAxes = {
		EKeys::Gamepad_LeftX,
		EKeys::Gamepad_LeftY,
		EKeys::Gamepad_RightX,
		EKeys::Gamepad_RightY,
		EKeys::MotionController_Left_Thumbstick_X,
		EKeys::MotionController_Left_Thumbstick_Y,
		EKeys::MotionController_Right_Thumbstick_X,
		EKeys::MotionController_Right_Thumbstick_Y,
	};

	TMap<FKey, FKey> KeyToAxisMap = {
		{EKeys::Gamepad_LeftTrigger, EKeys::Gamepad_LeftTriggerAxis},
		{EKeys::Gamepad_RightTrigger, EKeys::Gamepad_RightTriggerAxis},
		{EKeys::Gamepad_LeftStick_Up, EKeys::Gamepad_LeftY},
		{EKeys::Gamepad_LeftStick_Down, EKeys::Gamepad_LeftY},
		{EKeys::Gamepad_LeftStick_Left, EKeys::Gamepad_LeftX},
		{EKeys::Gamepad_LeftStick_Right, EKeys::Gamepad_LeftX},
		{EKeys::Gamepad_RightStick_Up, EKeys::Gamepad_RightY},
		{EKeys::Gamepad_RightStick_Down, EKeys::Gamepad_RightY},
		{EKeys::Gamepad_RightStick_Left, EKeys::Gamepad_RightX},
		{EKeys::Gamepad_RightStick_Right, EKeys::Gamepad_RightX},
		{EKeys::MotionController_Left_Trigger, EKeys::MotionController_Left_TriggerAxis},
		{EKeys::MotionController_Left_Grip1, EKeys::MotionController_Left_Grip1Axis},
		{EKeys::MotionController_Left_Grip2, EKeys::MotionController_Left_Grip2Axis},
		{EKeys::MotionController_Right_Trigger, EKeys::MotionController_Right_TriggerAxis},
		{EKeys::MotionController_Right_Grip1, EKeys::MotionController_Right_Grip1Axis},
		{EKeys::MotionController_Right_Grip2, EKeys::MotionController_Right_Grip2Axis},
		{EKeys::MotionController_Left_Thumbstick_Up, EKeys::MotionController_Left_Thumbstick_Y},
		{EKeys::MotionController_Left_Thumbstick_Down, EKeys::MotionController_Left_Thumbstick_Y},
		{EKeys::MotionController_Left_Thumbstick_Left, EKeys::MotionController_Left_Thumbstick_X},
		{EKeys::MotionController_Left_Thumbstick_Right, EKeys::MotionController_Left_Thumbstick_X},
		{EKeys::MotionController_Right_Thumbstick_Up, EKeys::MotionController_Right_Thumbstick_Y},
		{EKeys::MotionController_Right_Thumbstick_Down, EKeys::MotionController_Right_Thumbstick_Y},
		{EKeys::MotionController_Right_Thumbstick_Left, EKeys::MotionController_Right_Thumbstick_X},
		{EKeys::MotionController_Right_Thumbstick_Right, EKeys::MotionController_Right_Thumbstick_X},
	};

	//Indicates which column to navigate to when navigating to this Input Container
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		ETargetColumn TargetColumn = ETargetColumn::Left;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UINav Input")
		TSubclassOf<class UUINavInputBox> InputBox_BP;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
		class UPanelWidget* ActionPanel;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
		class UPanelWidget* AxisPanel;
	
	UPROPERTY(BlueprintReadOnly, Category = "UINav Input")
		class UUINavWidget* ParentWidget;

public:

	void Init(class UUINavWidget* NewParent);

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
		void OnInputBoxAdded(class UUINavInputBox* NewInputBox);
	virtual void OnInputBoxAdded_Implementation(class UUINavInputBox* NewInputBox);

	/*
	*	Called when a rebind was cancelled, specifying the reason for the revert.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnRebindCancelled(ERevertRebindReason RevertReason, FKey PressedKey);
	virtual void OnRebindCancelled_Implementation(ERevertRebindReason RevertReason, FKey PressedKey);

	UFUNCTION(BlueprintCallable, Category = "UINav Input")
		void ResetKeyMappings();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINav Input")
		bool IsKeyBeingUsed(FKey CompareKey) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINav Input")
		bool RespectsRestriction(FKey CompareKey, int Index);

	//Fetches the index offset from the TargetColumn variable for both the top and bottom of the Input Container
	int GetOffsetFromTargetColumn(bool bTop);

	FKey GetAxisFromKey(FKey Key);

	UFUNCTION(BlueprintCallable, Category = "UINav Input")
		FORCEINLINE ETargetColumn GetTargetColumn() const { return TargetColumn; }

	//-----------------------------------------------------------------------

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
	Indicates whether the player can cancel changing the keybind for an action
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		bool bCanCancelKeybind = true;

	/*
	Indicates whether the input boxes will hide or collapse unused InputBoxes
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		bool bCollapseInputBox = false;

	/*
	The preffered names of the given input names
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		class UDataTable* InputNameTable;

	/*
	The names of the desired actions to allow for rebinding,
	in the input settings and with their desirable name respectively.
	Leave desirable name as empty if similar to input settings.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		TMap<FName, FText> ActionNames;

	/*
	The names of the desired axes to allow for rebinding,
	in the input settings and with their desirable name respectively
	Leave desirable name as empty if similar to input settings.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		TMap<FName, FText> AxisNames;

	/*
	A list of the keys that the player shouldn't be able to use as rebinds
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		TArray<FKey> Blacklist;

	//The name used for empty key buttons
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		FName EmptyKeyName = FName("Unbound");

	/*
	The restrictions for the type of input associated with each column
	in the Input Container
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		TArray<EInputRestriction> InputRestrictions;

	class UUINavPCComponent* UINavPC = nullptr;

};
