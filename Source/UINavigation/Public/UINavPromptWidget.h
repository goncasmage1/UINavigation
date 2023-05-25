// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UINavWidget.h"
#include "UINavPromptWidget.generated.h"

class UTextBlock;

UCLASS()
class UINAVIGATION_API UUINavPromptWidget : public UUINavWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	virtual void OnSelect_Implementation(UUINavComponent* Component) override;

	virtual void OnReturn_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "UINavPromptWidget")
	void ProcessPromptWidgetSelected(UPromptDataBase* InPromptData);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINavPromptWidget")
	bool IsAcceptComponent(UUINavComponent* Component) const;

	UFUNCTION(BlueprintCallable, Category = "UINavPromptWidget")
	void SetCallback(const FPromptWidgetDecided& InCallback) { Callback = InCallback; }

public:
	UPROPERTY(EditAnywhere, Category = "UINavPromptWidget")
	FText Title;

	UPROPERTY(EditAnywhere, Category = "UINavPromptWidget")
	FText Message;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UINavPromptWidget")
	bool FirstComponentIsAccept = false;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UINavPromptWidget")
	UTextBlock* TitleText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UINavPromptWidget")
	UTextBlock* MessageText = nullptr;

	FPromptWidgetDecided Callback;
	
};
