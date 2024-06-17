// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UINavHorizontalComponent.h"
#include "UINavSlider.generated.h"

class FText;

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavSlider : public UUINavHorizontalComponent
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadWrite, Category = UINavSlider, meta = (BindWidget))
	class USlider* Slider = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = UINavSlider, meta = (BindWidget, OptionalWidget = true))
	class USpinBox* NavSpinBox = nullptr;

	UFUNCTION()
	void HandleOnSliderValueChanged(const float InValue);

	UFUNCTION()
	void HandleOnSliderMouseCaptureBegin();
	UFUNCTION()
	void HandleOnSliderMouseCaptureEnd();

	UFUNCTION()
	void HandleOnSpinBoxMouseCaptureBegin();

	UFUNCTION()
	void HandleOnSpinBoxValueChanged(const float InValue);

	UFUNCTION()
	void HandleOnSpinBoxValueCommitted(const float InValue, const ETextCommit::Type CommitMethod);

	int IndexFromPercent(const float Value);
	int IndexFromValue(const float Value);

	void UpdateTextFromValue(const float Value, const bool bUpdateSpinBox = true);
	void UpdateTextFromPercent(const float Percent, const bool bUpdateSpinBox = true);

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

	bool bMovingSlider = false;
	bool bMovingSpinBox = false;

	FLinearColor HandleDefaultColor = FColor::Black;
	FLinearColor BarDefaultColor = FColor::White;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavSlider)
	FLinearColor HandleHoverColor = FColor::Red;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavSlider)
	FLinearColor BarHoverColor = FColor::Blue;

	virtual void NativeConstruct() override;

	virtual bool Update(const bool bNotify = true) override;

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
