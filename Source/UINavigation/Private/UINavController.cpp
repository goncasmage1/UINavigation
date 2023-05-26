// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavController.h"
#include "UINavPCComponent.h"

AUINavController::AUINavController()
{
	UINavPCComp = CreateDefaultSubobject<UUINavPCComponent>(TEXT("UINav PC Component"));
}