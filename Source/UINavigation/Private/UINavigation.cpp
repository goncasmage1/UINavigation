// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavigation.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FUINavigationModule"

void FUINavigationModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FUINavigationModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUINavigationModule, UINavigation)