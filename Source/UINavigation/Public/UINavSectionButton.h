// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "UINavSectionButton.generated.h"

class UButton;

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavSectionButton : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UINavSectionButton")
	UButton* SectionButton = nullptr;

};
 
