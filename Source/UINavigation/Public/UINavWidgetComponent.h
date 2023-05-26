// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Components/WidgetComponent.h"
#include "UINavWidgetComponent.generated.h"

UCLASS( ClassGroup=(UINav), meta=(BlueprintSpawnableComponent) )
class UINAVIGATION_API UUINavWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:	

	UUINavWidgetComponent();

protected:

	virtual void BeginPlay() override;

};
