// Copyright (C) 2023 Gon�alo Marques - All Rights Reserved

#pragma once

#include "Framework/Application/NavigationConfig.h" // from Slate

class UUINavPCComponent;

class UINAVIGATION_API FUINavigationConfig : public FNavigationConfig
{
public:
	FUINavigationConfig(const UUINavPCComponent* const UINavPC, const bool bAllowDirectionalInput = true, const bool bAllowSectionInput = true, const bool bAllowAccept = true, const bool bAllowBack = true, const bool bUseAnalogDirectionalInput = true, const bool bUsingThumbstickAsMouse = false);

	virtual EUINavigationAction GetNavigationActionForKey(const FKey& InKey) const override;

	virtual EUINavigation GetNavigationDirectionFromAnalog(const FAnalogInputEvent& InAnalogEvent) override;

	EUINavigation GetNavigationDirectionFromAnalogKey(const FKeyEvent& InKeyEvent) const;

	TArray<FKey> GetKeysForDirection(const EUINavigation Direction) const;

	virtual bool IsAnalogHorizontalKey(const FKey& InKey) const override { return InKey == EKeys::Gamepad_LeftX || InKey == EKeys::Gamepad_RightX; }
	virtual bool IsAnalogVerticalKey(const FKey& InKey) const override { return InKey == EKeys::Gamepad_LeftY || InKey == EKeys::Gamepad_RightY; }

	bool IsGamepadSelectKey(const FKey& Key) const { return GamepadSelectKeys.Contains(Key); }

	TMap<FKey, EUINavigationAction> KeyActionRules;

	TArray<FKey> GamepadSelectKeys;
};