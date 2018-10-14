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

	UPROPERTY(EditDefaultsOnly)
		class UDataTable* ControllerMapping;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UPanelWidget* Panel;
	
	class UUINavWidget* ParentWidget;

public:

	virtual void NativePreConstruct() override;

	void SetParentWidget(class UUINavWidget* NewParent);

	UFUNCTION(BlueprintCallable, Category = "UINav Input")
		void ResetKeyMappings();

	//-----------------------------------------------------------------------

	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", ClampMax = "3", UIMin = "1", UIMax = "3"))
		int InputsPerAction = 2;

	int StartingIndex = -1;

	/*
	The indexes of the desired actions to allow for rebinding
	*/
	UPROPERTY(EditAnywhere)
		TArray<FName> ActionNames;

};
