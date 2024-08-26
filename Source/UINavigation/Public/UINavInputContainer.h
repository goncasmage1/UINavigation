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

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
	class UPanelWidget* InputBoxesPanel = nullptr;

	class UUINavWidget* ParentWidget = nullptr;

public:

	virtual void NativeConstruct() override;

	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

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

	ERevertRebindReason CanRegisterKey(class UUINavInputBox* InputBox, const FKey NewKey, int& OutCollidingActionIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINav Input")
	bool CanUseKey(class UUINavInputBox* InputBox, const FKey CompareKey, int& OutCollidingActionIndex) const;

	UFUNCTION()
	void SwapKeysDecided(const UPromptDataBase* const PromptData);

	UUINavInputBox* GetInputBoxInDirection(UUINavInputBox* InputBox, const EUINavigation Direction);

	void GetInputRebindData(const int InputIndex, FInputRebindData& RebindData) const;

	void GetEnhancedInputRebindData(const int InputIndex, FInputRebindData& RebindData) const;

	//-----------------------------------------------------------------------

	class UUINavPCComponent* UINavPC = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "UINav Input")
	TArray<UUINavInputBox*> InputBoxes;

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

	//The title text used for the swap keys widget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	FText SwapKeysTitleText = FText::FromString(TEXT("Swap Keys"));

	//The message text used for the swap keys widget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	FText SwapKeysMessageText = FText::FromString(TEXT("{CollidingKey} is already being used by {CollidingAction}.\nDo you want to swap it with {OtherKey}?"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	TArray<UInputMappingContext*> InputContexts;
};
