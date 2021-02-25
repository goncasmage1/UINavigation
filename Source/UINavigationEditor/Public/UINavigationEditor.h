// Fill out your copyright notice in the Description page of Project Settings.
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
