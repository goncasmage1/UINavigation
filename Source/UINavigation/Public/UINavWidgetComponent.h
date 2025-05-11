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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UINavigation Selector")
	bool bTakeFocus = true;

};
