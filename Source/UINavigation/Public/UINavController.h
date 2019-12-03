// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#pragma once

#include "GameFramework/PlayerController.h"
#include "UINavPCReceiver.h"
#include "Engine/DataTable.h"
#include "Data/CountdownPhase.h"
#include "Data/InputRestriction.h"
#include "Data/InputType.h"
#include "Data/NavigationDirection.h"
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
		class UUINavPCComponent* UINavPCComp;

public:

	AUINavController();
	
};
