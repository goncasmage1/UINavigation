// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Slider.h"
#include "UINavCustomSlider.generated.h"

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavCustomSlider : public USlider
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere, Category = UINavSlider)
		int DefaultValue = 0.f;

	UPROPERTY(EditAnywhere, Category = UINavSlider)
		int Steps = 2;

	float AttemptedValue;
	int StepIndex;

	UFUNCTION()
		void OnCaptureBegin();
	UFUNCTION()
		void OnValueUpdate(float NewValue);

	void SnapToValue();
	
public:

	UUINavCustomSlider();

	void IncreaseStepIndex();
	void DecreaseStepIndex();

	UFUNCTION(BlueprintCallable)
		FORCEINLINE int GetSteps() const { return Steps; }

	UFUNCTION(BlueprintCallable)
		FORCEINLINE int GetStepIndex() const { return StepIndex; }

};
