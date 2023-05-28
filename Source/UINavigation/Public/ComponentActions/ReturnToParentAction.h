// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ComponentActions/UINavComponentAction.h"
#include "Templates/SubclassOf.h"
#include "ReturnToParentAction.generated.h"

class UUINavWidget;

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UReturnToParentAction : public UUINavComponentAction
{
	GENERATED_BODY()

public:

	void ExecuteAction_Implementation(UUINavComponent* Component) override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReturnToParentAction")
	bool bRemoveAllParents = false;

};
