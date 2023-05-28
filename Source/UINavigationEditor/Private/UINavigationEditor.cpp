// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavigationEditor.h"

#include "UINavSettings.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FUINavigationEditorModule"

void FUINavigationEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	RegisterSettings();
}

void FUINavigationEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (UObjectInitialized())
	{
		UnregisterSettings();
	}
}

void FUINavigationEditorModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "UINavigation",
			LOCTEXT("UINavigationSettingsName", "UI Navigation"),
			LOCTEXT("UINavigationSettingsDescription", "Configure the UI Navigation settings."),
			GetMutableDefault<UUINavSettings>()
		);

		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindRaw(this, &FUINavigationEditorModule::HandleSettingsSaved);
		}
	}
}

void FUINavigationEditorModule::UnregisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "UINavigation");
	}
}

bool FUINavigationEditorModule::HandleSettingsSaved()
{
	GetMutableDefault<UUINavSettings>()->SaveConfig();

	return true;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUINavigationEditorModule, UINavigationEditor)
