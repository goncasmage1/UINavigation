// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

	/*
	The desired InputBox widget blueprint
	*/
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class UUINavInputBox> InputBox_BP;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UPanelWidget* Panel;
	
	class UUINavWidget* ParentWidget;

public:

	virtual void NativePreConstruct() override;

	void SetParentWidget(class UUINavWidget* NewParent);

	//-----------------------------------------------------------------------

	int StartingIndex = -1;

	/*
	The indexes of the desired actions to allow for rebinding
	*/
	UPROPERTY(EditAnywhere)
		TArray<int> ActionIndexes;

	UPROPERTY()
		TArray<class UUINavInputBox*> InputBoxes;

};
