// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UINavComponent.h"
#include "UINavHorizontalComponent.generated.h"

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavHorizontalComponent : public UUINavComponent
{
	GENERATED_BODY()

protected:


public:

	//Indicates the option that should appear first in the slider
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavHorizontalComponent)
		int OptionIndex = 0;

	//If set to true, will loop between options (won't disable buttons, even if DisableButtons is set to true)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavComponentBox)
		bool bLoopOptions = false;

	UFUNCTION(BlueprintCallable, Category = UINavHorizontalComponent)
		virtual void Update();

	UFUNCTION(BlueprintCallable, Category = UINavHorizontalComponent)
		virtual void NavigateLeft();
	UFUNCTION(BlueprintCallable, Category = UINavHorizontalComponent)
		virtual void NavigateRight();

	UFUNCTION(BlueprintNativeEvent, Category = UINavHorizontalComponent)
		void OnNavigateLeft();
	void OnNavigateLeft_Implementation();
	UFUNCTION(BlueprintNativeEvent, Category = UINavHorizontalComponent)
		void OnNavigateRight();
	void OnNavigateRight_Implementation();
	
};
