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
	
	UFUNCTION()
		void HandleOnValueChanged(float InValue);

	float IndexFromPercent(float Value);

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavSlider, meta = (ClampMin="0"))
		float MinValue = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavSlider, meta = (ClampMin="0"))
		float MaxValue = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavSlider, meta = (ClampMin="0"))
		float Interval = 0.1f;
	int OptionCount = 0;

	virtual void NativeConstruct() override;

	virtual void Update() override;

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = UINavSlider)
		FORCEINLINE float GetCurrentValue() const { return (MinValue + OptionIndex * Interval); }

	virtual void NavigateLeft() override;
	virtual void NavigateRight() override;

};
