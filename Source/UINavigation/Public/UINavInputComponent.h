// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UINavComponent.h"
#include "UINavInputComponent.generated.h"

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavInputComponent : public UUINavComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, OptionalWidget = true))
		class UImage* InputImage;

};
