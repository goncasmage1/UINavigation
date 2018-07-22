// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UINavComponent.h"
#include "UINavInputBox.generated.h"

#define DISPLAYERROR(Text) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("%s"), *(FString(TEXT("Error in ")).Append(GetName()).Append(TEXT(": ")).Append(Text))));

/**
* This class contains the logic for rebinding input keys to their respective actions
*/
UCLASS()
class UINAVIGATION_API UUINavInputBox : public UUINavComponent
{
	GENERATED_BODY()
	
protected:

	void UpdateActionKey(FKey NewKey, bool SecondKey);

	bool bWaitingForInput = false;
	bool bSecondKey = false;

	FKey ConsumedKey;
	FKey Key1, Key2;

	class AUINavController* PC;

public:

	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry & InGeometry, const FKeyEvent & InKeyEvent);
	
	void OnReceiveNewKey();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UUINavButton* NavButton2;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* NavText2;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* ActionText;

	UPROPERTY(EditAnywhere)
		FString ActionName;

	
};
