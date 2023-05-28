// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FUINavigationEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterSettings();
	void UnregisterSettings();
	bool HandleSettingsSaved();
};
