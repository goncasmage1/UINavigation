// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ComponentActions/UINavComponentAction.h"
#include "Templates/SubclassOf.h"
#include "GoToWidgetAction.generated.h"

class UUINavWidget;

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UGoToWidgetAction : public UUINavComponentAction
{
	GENERATED_BODY()

public:

	void ExecuteAction_Implementation(UUINavComponent* Component) override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoToWidgetAction")
	TSubclassOf<UUINavWidget> WidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoToWidgetAction")
	bool bRemoveParent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoToWidgetAction")
	bool bDestroyParent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoToWidgetAction")
	int ZOrder = 0;
	
};
