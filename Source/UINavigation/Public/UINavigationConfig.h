// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Framework/Application/NavigationConfig.h" // from Slate

class UInputMappingContext;

class UINAVIGATION_API FUINavigationConfig : public FNavigationConfig
{
public:
	FUINavigationConfig(const UInputMappingContext* const InputContext, const bool bAllowDirectionalInput = true, const bool bAllowSectionInput = true, const bool bAllowAccept = true, const bool bAllowBack = true, const bool bUseAnalogDirectionalInput = true, const bool bUsingThumbstickAsMouse = false);

	virtual EUINavigationAction GetNavigationActionForKey(const FKey& InKey) const override;

	virtual EUINavigation GetNavigationDirectionFromAnalog(const FAnalogInputEvent& InAnalogEvent) override;

	EUINavigation GetNavigationDirectionFromAnalogKey(const FKeyEvent& InKeyEvent) const;

	TArray<FKey> GetKeysForDirection(const EUINavigation Direction) const;

	virtual bool IsAnalogHorizontalKey(const FKey& InKey) const override { return InKey == EKeys::Gamepad_LeftX || InKey == EKeys::Gamepad_RightX; }
	virtual bool IsAnalogVerticalKey(const FKey& InKey) const override { return InKey == EKeys::Gamepad_LeftY || InKey == EKeys::Gamepad_RightY; }

	bool IsGamepadSelectKey(const FKey& Key) const { return GamepadSelectKeys.Contains(Key); }

	TArray<FKey> GamepadSelectKeys;
};