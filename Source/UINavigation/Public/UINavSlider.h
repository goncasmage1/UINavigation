// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UINavHorizontalComponent.h"
#include "UINavSlider.generated.h"

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavSlider : public UUINavHorizontalComponent
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadWrite, Category = UINavSlider, meta = (BindWidget))
		class USlider* Slider;

	UPROPERTY(BlueprintReadWrite, Category = UINavSlider, meta = (BindWidget, OptionalWidget = true))
		class USpinBox* NavSpinBox;

	UFUNCTION()
		void HandleOnSliderValueChanged(const float InValue);
	UFUNCTION()
		void HandleOnMouseCaptureBegin();
	UFUNCTION()
		void HandleOnMouseCaptureEnd();
	UFUNCTION()
		void HandleOnSpinBoxValueChanged(float InValue, ETextCommit::Type CommitMethod);

	float IndexFromPercent(const float Value);
	float IndexFromValue(const float Value);

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavSlider)
		float MinValue = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavSlider)
		float MaxValue = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavSlider, meta = (ClampMin="0"))
		float Interval = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavSlider, meta = (ClampMin="0"))
		int MaxDecimalDigits = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavSlider, meta = (ClampMin="0"))
		int MinDecimalDigits = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavSlider)
		bool bUseComma = false;

	float Difference = 0.0f;

	FLinearColor HandleDefaultColor = FColor::Black;
	FLinearColor BarDefaultColor = FColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavSlider, meta = (ClampMin = "0"))
		FLinearColor HandleHoverColor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavSlider, meta = (ClampMin = "0"))
		FLinearColor BarHoverColor;

	virtual void NativeConstruct() override;

	virtual void Update() override;

	virtual FORCEINLINE int GetMaxOptionIndex() const override
	{
		return (MaxValue - MinValue) / Interval;
	}

	virtual void OnNavigatedTo_Implementation() override;
	virtual void OnNavigatedFrom_Implementation() override;

	//Get Current Value inserted in the specified number range (not 0 to 1)
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = UINavSlider)
		FORCEINLINE float GetCurrentValue() const { return (MinValue + OptionIndex * Interval); }

	UFUNCTION(BlueprintCallable, Category = UINavSlider)
		void SetValueClamped(const float Value);

	//Get Current Slider value (0 to 1)
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = UINavSlider)
		float GetSliderValue() const;

	virtual void NavigateLeft() override;
	virtual void NavigateRight() override;

};
