// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UINavWidget.h"
#include "UINavPromptWidget.generated.h"

UCLASS()
class UINAVIGATION_API UUINavPromptWidget : public UUINavWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category = UINavPromptWidget)
	int ReturnSelectedIndex = 0;

	virtual void OnSelect_Implementation(int Index) override;

	virtual void OnReturn_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = UINavPromptWidget)
	void ProcessPromptWidgetSelected(int Index);
	
};
