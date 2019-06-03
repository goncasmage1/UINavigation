// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#include "UINavController.h"
#include "UINavPCComponent.h"
#include "UINavWidget.h"
#include "UINavSettings.h"
#include "Data/InputIconMapping.h"
#include "Data/InputNameMapping.h"

AUINavController::AUINavController()
{
	UINavPCComp = CreateDefaultSubobject<UUINavPCComponent>(TEXT("UINav PC Component"));
}