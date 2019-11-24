// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Data/InputIconMapping.h"
#include "Data/InputNameMapping.h"
#include "Data/InputRebindData.h"
#include "Data/AxisType.h"
#include "UINavInputBox.generated.h"

#define DISPLAYERROR(Text) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("%s"), *(FString(TEXT("Error in ")).Append(GetName()).Append(TEXT(": ")).Append(Text))));

#define IS_AXIS (AxisType != EAxisType::None)
#define IS_POSITIVE_AXIS (AxisType == EAxisType::Positive)
#define IS_NEGATIVE_AXIS (AxisType == EAxisType::Negative)
#define IS_SCALE_CORRECT(AxisMapping) ((TempAxes[i].Scale > 0.0f && IS_POSITIVE_AXIS) || (TempAxes[i].Scale < 0.0f && IS_NEGATIVE_AXIS))

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

	bool UpdateKeyIconForKey(int Index);
	FText GetKeyText(int Index);
	void UpdateKeyDisplay(int Index);
	FKey GetKeyFromAxis(FKey AxisKey);
	void ProcessInputName();

public:

	virtual void NativeConstruct() override;

	void CreateKeyWidgets();
	bool TrySetupNewKey(FKey NewKey, int KeyIndex, class UUINavInputComponent* NewInputButton);
	void ResetKeyWidgets();
	void UpdateInputKey(FKey NewKey, int Index);
	void RevertToKeyText(int Index);

	void NotifySelected(int Index);

	bool ContainsKey(FKey CompareKey) const;
	FORCEINLINE bool IsAxis() const { return IS_AXIS; }

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

	class UUINavInputContainer* Container;

	FName InputName;
	FInputRebindData InputData = FInputRebindData();

	int KeysPerInput = 2;
};
