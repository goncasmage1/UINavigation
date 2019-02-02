// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UINavInputBox.generated.h"

#define DISPLAYERROR(Text) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("%s"), *(FString(TEXT("Error in ")).Append(GetName()).Append(TEXT(": ")).Append(Text))));

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

	bool ShouldRegisterKey(FKey NewKey, int Index) const;
	bool UpdateKeyIconForKey(int Index);
	FText GetKeyName(int Index);
	void CheckKeyIcon(int Index);

public:

	virtual void NativeConstruct() override;

	void BuildKeyMappings();
	void ResetKeyMappings();
	void UpdateInputKey(FKey NewKey, int Index);
	void RevertToActionText(int Index);

	void NotifySelected(int Index);

	bool ContainsKey(FKey CompareKey) const;

	bool bIsAxis = false;

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

	int InputsPerAction = 2;
};
