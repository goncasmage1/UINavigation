// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

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

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
	class UImage* InputImage = nullptr;
};
