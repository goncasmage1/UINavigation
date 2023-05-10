// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Framework/Application/NavigationConfig.h" // from Slate

class UINAVIGATION_API FUINavigationConfig : public FNavigationConfig
{
public:
	FUINavigationConfig();

	virtual EUINavigation GetNavigationDirectionFromAnalog(const FAnalogInputEvent& InAnalogEvent) override;

	virtual EUINavigationAction GetNavigationActionForKey(const FKey& InKey) const override;

	TArray<FKey> GetKeysForDirection(const EUINavigation Direction) const;

	TMap<FKey, EUINavigationAction> KeyActionRules;
};