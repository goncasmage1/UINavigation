// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Components/Button.h"
#include "UINavButton.generated.h"

class UUINavComponent;

/**
 * This class represents a required button type for the UI Navigation plugin
 */
UCLASS()
class UINAVIGATION_API UUINavButton : public UButton
{
	GENERATED_BODY()
	
public:

	UUINavButton();

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

	UPROPERTY(BlueprintReadOnly, Category = UINavButton)
	UUINavComponent* NavComp = nullptr;

	

	bool bAutoCollapse = false;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavButton)
	bool IsValid(const bool bIgnoreDisabledUINavButton = true) const;

};
