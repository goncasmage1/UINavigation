// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Components/WidgetComponent.h"
#include "UINavWidgetComponent.generated.h"

UCLASS( ClassGroup=(UINav), meta=(BlueprintSpawnableComponent) )
class UINAVIGATION_API UUINavWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:

	/*
	Whether this UINavWidget should Take Focus when on the screen. Usually, you'll want to set this to true if the player is supposed to only 
	be able to navigate this widget when it's on the screen, and set it to false if this is a widget where the player can navigate while moving around.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UINavigation Selector")
	bool bTakeFocus = true;

};
