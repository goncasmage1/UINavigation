// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UINavComponent.h"
#include "UINavOptionBox.generated.h"

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavOptionBox : public UUINavComponent
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(BlueprintReadOnly)
		int OptionIndex = 0;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UButton* LeftButton;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UButton* RightButton;

	void CheckLeftLimit();
	void CheckRightLimit();

	void UpdateTextBlock();
	UFUNCTION(BlueprintCallable, Category = UINavSlider)
		void UpdateTextWithValue(int NewIndex);

public:

	virtual void NativeConstruct() override;

	/*If set to false, will use StringOptions, otherwise will use
	all integers in designated range (from MinRange to MaxRange, inclusive)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavSlider)
		bool bUseNumberRange = false;

	//Indicates the option that should appear first in the slider
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavSlider)
		int DefaultOptionIndex;

	//The list of Names to display as options in this slider
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavSlider, meta = (EditCondition = "!bUseNumberRange"))
		TArray<FName> StringOptions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavSlider, meta = (EditCondition = "bUseNumberRange"))
		int MinRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavSlider, meta = (EditCondition = "bUseNumberRange"))
		int MaxRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavSlider, meta = (EditCondition = "bUseNumberRange"))
		int Interval = 1;

	//If set to true, will disable buttons if the slider runs out of options on either side
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavSlider)
		bool bDisableButtons = true;

	UFUNCTION(BlueprintCallable)
		void NavigateLeft();
	UFUNCTION(BlueprintCallable)
		void NavigateRight();
	
	
};
