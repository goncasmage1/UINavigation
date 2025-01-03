// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UINavWidget.h"
#include "UINavPromptWidget.generated.h"

class UTextBlock;
class URichTextBlock;

UCLASS()
class UINAVIGATION_API UUINavPromptWidget : public UUINavWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	virtual void OnSelect_Implementation(UUINavComponent* Component) override;

	virtual void OnReturn_Implementation() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UINavPromptWidget")
	void ProcessPromptWidgetSelected(UPromptDataBase* InPromptData);
	virtual void ProcessPromptWidgetSelected_Implementation(UPromptDataBase* InPromptData);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINavPromptWidget")
	bool IsAcceptComponent(UUINavComponent* Component) const;

	UFUNCTION(BlueprintCallable, Category = "UINavPromptWidget")
	void SetCallback(const FPromptWidgetDecided& InCallback) { Callback = InCallback; }

	UFUNCTION(BlueprintCallable, Category = "UINavPromptWidget")
	void ExecuteCallback(UPromptDataBase* InPromptData);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UINavPromptWidget")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UINavPromptWidget")
	FText Message;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UINavPromptWidget")
	bool FirstComponentIsAccept = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINavPromptWidget")
	FString TitleStyleRowName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINavPromptWidget")
	FString MessageStyleRowName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UINavPromptWidget")
	UTextBlock* TitleText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UINavPromptWidget")
	UTextBlock* MessageText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UINavPromptWidget")
	URichTextBlock* TitleRichText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UINavPromptWidget")
	URichTextBlock* MessageRichText = nullptr;

	FPromptWidgetDecided Callback;
	
};
