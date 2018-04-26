// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UINavComponentBox.h"
#include "UINavSliderBox.generated.h"

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavSliderBox : public UUINavComponentBox
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UProgressBar* SliderBar;

	virtual void CheckRightLimit() override;

	virtual void UpdateTextBlock() override;
	
public:

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
		virtual void NavigateRight() override;
	
};
