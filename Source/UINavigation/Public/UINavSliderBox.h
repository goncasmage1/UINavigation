// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

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
	class UProgressBar* SliderBar = nullptr;

	virtual void Update() override;
	
public:

	virtual void NativeConstruct() override;

	FORCEINLINE virtual int GetMaxOptionIndex() const override
	{
		return (MaxRange - MinRange) / Interval;
	}

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = UINavSliderBox)
		float GetSliderPercent() const;
	
};
