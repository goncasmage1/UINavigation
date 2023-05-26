// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "GameFramework/PlayerController.h"
#include "UINavPCComponent.h"
#include "UINavPCReceiver.h"
#include "UINavController.generated.h"


/**
 * This class contains the logic for input-related actions with UINavWidgets
 */
UCLASS()
class UINAVIGATION_API AUINavController : public APlayerController, public IUINavPCReceiver
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UINavController)
	class UUINavPCComponent* UINavPCComp = nullptr;

public:

	AUINavController();
	
};
