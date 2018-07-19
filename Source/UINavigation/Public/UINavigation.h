// Copyright (C) 2018 Gonçalo Marques - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
//#include "ModuleManager.h"

class FUINavigationModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};