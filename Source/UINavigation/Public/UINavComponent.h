// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "UINavComponent.generated.h"

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavComponent : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UUINavComponent(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	
	void CallCustomInput(FName ActionName, uint8* Buffer);

	UFUNCTION(BlueprintNativeEvent, Category = UINavComponent)
		void OnNavigatedTo();
	virtual void OnNavigatedTo_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = UINavComponent)
		void OnNavigatedFrom();
	virtual void OnNavigatedFrom_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = UINavComponent)
		void OnSelected();
	virtual void OnSelected_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = UINavComponent)
		void OnStartSelected();
	virtual void OnStartSelected_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = UINavComponent)
		void OnStopSelected();
	virtual void OnStopSelected_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavComponent)
		bool IsValid(const bool bIgnoreDisabledUINavButton = true) const;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = UINavComponent)
		class UUINavButton* NavButton;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, OptionalWidget = true), Category = UINavComponent)
		class UTextBlock* NavText;

};
