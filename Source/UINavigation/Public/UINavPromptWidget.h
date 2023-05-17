// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UINavWidget.h"
#include "UINavPromptWidget.generated.h"

UCLASS()
class UINAVIGATION_API UUINavPromptWidget : public UUINavWidget
{
	GENERATED_BODY()

public:
	virtual void OnSelect_Implementation(UUINavComponent* Component) override;

	virtual void OnReturn_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "UINavPromptWidget")
	void ProcessPromptWidgetSelected(UPromptDataBase* const InPromptData);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINavPromptWidget")
	bool IsAcceptComponent(UUINavComponent* Component) const;

	void SetCallback(const FPromptWidgetDecided& InCallback) { Callback = InCallback; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UINavPromptWidget")
	bool FirstComponentIsAccept = false;

	UPROPERTY(EditDefaultsOnly, Category = "UINavPromptWidget", meta (ExposeOnSpawn = true))
	FPromptWidgetDecided Callback;
	
};
