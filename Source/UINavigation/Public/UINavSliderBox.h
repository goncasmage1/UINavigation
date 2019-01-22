// Copyright (C) 2018 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Engine.h"
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

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = UINavSliderBox)
		class UProgressBar* SliderBar;

	virtual void CheckRightLimit() override;

	virtual void UpdateTextBlock() override;
	
public:

	virtual void NativeConstruct() override;

	virtual void NavigateRight() override;

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = UINavSliderBox)
		float GetSliderPercent() const;
	
};
