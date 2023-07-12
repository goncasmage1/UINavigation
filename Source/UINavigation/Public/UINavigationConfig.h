// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Framework/Application/NavigationConfig.h" // from Slate

class UINAVIGATION_API FUINavigationConfig : public FNavigationConfig
{
public:
	FUINavigationConfig(const bool bAllowAccept = true, const bool bAllowBack = true, const bool bUseAnalogDirectionalInput = true);

	virtual EUINavigationAction GetNavigationActionForKey(const FKey& InKey) const override;

	EUINavigation GetNavigationDirectionFromAnalogKey(const FKeyEvent& InKeyEvent) const;

	TArray<FKey> GetKeysForDirection(const EUINavigation Direction) const;

	virtual bool IsAnalogHorizontalKey(const FKey& InKey) const override { return InKey == EKeys::Gamepad_LeftX || InKey == EKeys::Gamepad_RightX; }
	virtual bool IsAnalogVerticalKey(const FKey& InKey) const override { return InKey == EKeys::Gamepad_LeftY || InKey == EKeys::Gamepad_RightY; }

	TMap<FKey, EUINavigationAction> KeyActionRules;
};