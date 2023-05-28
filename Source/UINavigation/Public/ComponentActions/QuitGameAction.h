// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ComponentActions/UINavComponentAction.h"
#include "Templates/SubclassOf.h"
#include "QuitGameAction.generated.h"

class UUINavWidget;

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UQuitGameAction : public UUINavComponentAction
{
	GENERATED_BODY()

public:

	void ExecuteAction_Implementation(UUINavComponent* Component) override;

public:
	
};
