// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UUINavButton* NavButton;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, OptionalWidget = true))
		class UTextBlock* NavText;
};
