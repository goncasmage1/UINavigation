// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FUINavigationModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};