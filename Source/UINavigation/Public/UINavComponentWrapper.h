// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "UINavComponentBox.h"
#include "UINavHorizontalComponent.h"
#include "UINavOptionBox.h"
#include "UINavSlider.h"
#include "UINavSliderBox.h"
#include "UINavComponentWrapper.generated.h"

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavComponentWrapper : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = UINavComponentWrapper)
	class UUINavComponent* UINavComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = UINavComponentWrapper)
	UUINavComponentWrapper* UINavWrapper = nullptr;

public:

	UFUNCTION(BlueprintCallable, Category = UINavComponentWrapper)
	FORCEINLINE UUINavComponent* GetUINavComponent() const
	{
		if (UINavComponent != nullptr) return UINavComponent;

		if (UINavWrapper != nullptr) return UINavWrapper->GetUINavComponent();

		return nullptr;
	}

	UFUNCTION(BlueprintCallable, Category = UINavComponentWrapper)
	FORCEINLINE UUINavComponentBox* GetUINavComponentBox() const
	{
		return Cast<UUINavComponentBox>(GetUINavComponent());
	}

	UFUNCTION(BlueprintCallable, Category = UINavComponentWrapper)
	FORCEINLINE UUINavHorizontalComponent* GetUINavHorizontalComponent() const
	{
		return Cast<UUINavHorizontalComponent>(GetUINavComponent());
	}

	UFUNCTION(BlueprintCallable, Category = UINavComponentWrapper)
	FORCEINLINE UUINavOptionBox* GetUINavOptionBox() const
	{
		return Cast<UUINavOptionBox>(GetUINavComponent());
	}

	UFUNCTION(BlueprintCallable, Category = UINavComponentWrapper)
	FORCEINLINE UUINavSlider* GetUINavSlider() const
	{
		return Cast<UUINavSlider>(GetUINavComponent());
	}

	UFUNCTION(BlueprintCallable, Category = UINavComponentWrapper)
	FORCEINLINE UUINavSliderBox* GetUINavSliderBox() const
	{
		return Cast<UUINavSliderBox>(GetUINavComponent());
	}
	
};
