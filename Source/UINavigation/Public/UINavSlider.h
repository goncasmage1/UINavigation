// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "UINavComponent.h"
#include "UINavSlider.generated.h"

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavSlider : public UUINavComponent
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UButton* SliderHandle;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UImage* SliderBar;

	bool bDraggingSlider;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = UINavSlider)
		int DefaultValue = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = UINavSlider)
		int Steps = 2;

	int StepIndex;

	float CurrentValue;
	float AttemptedValue;

	class AUINavController* UINavPC;

	UFUNCTION()
		void OnClick();
	UFUNCTION()
		void OnRelease();

	/*UFUNCTION()
		void OnCaptureBegin();
	UFUNCTION()
		void OnValueUpdate(float NewValue);*/

	//Moves the SliderHandle to the mouse's position
	void MoveHandle();
	//Calculates the slider's value from 0 to 1
	void CalculateValue();
	//Snaps the slider's value to a valid location and value
	void SnapToValue();

public:

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry & MyGeometry, float DeltaTime) override;

	void SetPC(class AUINavController* NewPC);

	UFUNCTION(BlueprintCallable)
		void IncreaseStepIndex();
	UFUNCTION(BlueprintCallable)
		void DecreaseStepIndex();

	UFUNCTION(BlueprintCallable)
		FORCEINLINE int GetSteps() const { return Steps; }

	UFUNCTION(BlueprintCallable)
		FORCEINLINE int GetStepIndex() const { return StepIndex; }
	
	
};
