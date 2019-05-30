// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Data/InputRestriction.h"
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

	TArray<FString> PossibleAxisNames = {
		TEXT("Gamepad_LeftTrigger"),
		TEXT("Gamepad_RightTrigger"),
		TEXT("Gamepad_LeftStick_Up"),
		TEXT("Gamepad_LeftStick_Down"),
		TEXT("Gamepad_LeftStick_Right"),
		TEXT("Gamepad_LeftStick_Left"),
		TEXT("Gamepad_RightStick_Up"),
		TEXT("Gamepad_RightStick_Down"),
		TEXT("Gamepad_RightStick_Right"),
		TEXT("Gamepad_RightStick_Left"),
		TEXT("MotionController_Left_Thumbstick_Up"),
		TEXT("MotionController_Left_Thumbstick_Down"),
		TEXT("MotionController_Left_Thumbstick_Left"),
		TEXT("MotionController_Left_Thumbstick_Right"),
		TEXT("MotionController_Right_Thumbstick_Up"),
		TEXT("MotionController_Right_Thumbstick_Down"),
		TEXT("MotionController_Right_Thumbstick_Left"),
		TEXT("MotionController_Right_Thumbstick_Right"),
		TEXT("MotionController_Left_Trigger"),
		TEXT("MotionController_Left_Grip1"),
		TEXT("MotionController_Left_Grip2"),
		TEXT("MotionController_Right_Trigger"),
		TEXT("MotionController_Right_Grip1"),
		TEXT("MotionController_Right_Grip2"),
	};

	//Indicates which column to navigate to when navigating to this Input Container
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		ETargetColumn TargetColumn = ETargetColumn::Left;

	UPROPERTY(EditDefaultsOnly, Category = "UINav Input")
		TSubclassOf<class UUINavInputBox> InputBox_BP;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
		class UPanelWidget* ActionPanel;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
		class UPanelWidget* AxisPanel;
	
	class UUINavWidget* ParentWidget;

public:

	void Init(class UUINavWidget* NewParent);

	UFUNCTION(BlueprintCallable, Category = "UINav Input")
		void ResetKeyMappings();

	UFUNCTION(BlueprintCallable, Category = "UINav Input")
		bool IsKeyBeingUsed(FKey CompareKey) const;

	UFUNCTION(BlueprintCallable, Category = "UINav Input")
		bool RespectsRestriction(FKey CompareKey, int Index);

	//Fetches the index offset from the TargetColumn variable for both the top and bottom of the Input Container
	int GetOffsetFromTargetColumn(bool bTop);

	FKey GetAxisKeyFromActionKey(FKey ActionKey);

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
	The names of the desired actions to allow for rebinding
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		TArray<FName> ActionNames;

	/*
	The names of the desired axes to allow for rebinding
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		TArray<FName> AxisNames;

	//The name used for empty key buttons
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		FName EmptyKeyName = FName("Unbound");

	/*
	The restrictions for the type of input associated with each column
	in the Input Container
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		TArray<EInputRestriction> InputRestrictions;

	class AUINavController* UINavPC = nullptr;

};
