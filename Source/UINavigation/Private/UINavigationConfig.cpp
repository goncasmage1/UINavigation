// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavigationConfig.h"
#include "UINavSettings.h"
#include "Data/UINavEnhancedInputActions.h"
#include "InputMappingContext.h"

FUINavigationConfig::FUINavigationConfig(const bool bAllowAccept /*= true*/, const bool bAllowBack /*= true*/, const bool bUseAnalogDirectionalInput /*= true*/)
{
	KeyEventRules.Reset();
	bTabNavigation = false;
	bKeyNavigation = true;
	bAnalogNavigation = bUseAnalogDirectionalInput;

	const UUINavSettings* const UINavSettings = GetDefault<UUINavSettings>();
	const UUINavEnhancedInputActions* const InputActions = UINavSettings->EnhancedInputActions.LoadSynchronous();
	const UInputMappingContext* const InputContext = UINavSettings->EnhancedInputContext.LoadSynchronous();
	if (InputActions == nullptr || InputContext == nullptr)
	{
		return;
	}

	for (const FEnhancedActionKeyMapping& Mapping : InputContext->GetMappings())
	{
		if (Mapping.Action == InputActions->IA_MenuUp)
		{
			KeyEventRules.Emplace(Mapping.Key, EUINavigation::Up);
		}
		else if (Mapping.Action == InputActions->IA_MenuDown)
		{
			KeyEventRules.Emplace(Mapping.Key, EUINavigation::Down);
		}
		else if (Mapping.Action == InputActions->IA_MenuLeft)
		{
			KeyEventRules.Emplace(Mapping.Key, EUINavigation::Left);
		}
		else if (Mapping.Action == InputActions->IA_MenuRight)
		{
			KeyEventRules.Emplace(Mapping.Key, EUINavigation::Right);
		}
		else if (Mapping.Action == InputActions->IA_MenuNext)
		{
			KeyEventRules.Emplace(Mapping.Key, EUINavigation::Next);
		}
		else if (Mapping.Action == InputActions->IA_MenuPrevious)
		{
			KeyEventRules.Emplace(Mapping.Key, EUINavigation::Previous);
		}
		else if (Mapping.Action == InputActions->IA_MenuSelect && bAllowAccept)
		{
			KeyActionRules.Emplace(Mapping.Key, EUINavigationAction::Accept);
		}
		else if (Mapping.Action == InputActions->IA_MenuReturn && bAllowBack)
		{
			KeyActionRules.Emplace(Mapping.Key, EUINavigationAction::Back);
		}
	}
}

EUINavigationAction FUINavigationConfig::GetNavigationActionForKey(const FKey& InKey) const
{
	const EUINavigationAction* NavAction = KeyActionRules.Find(InKey);
	return NavAction == nullptr ? EUINavigationAction::Invalid : *NavAction;
}

EUINavigation FUINavigationConfig::GetNavigationDirectionFromAnalogKey(const FKeyEvent& InKeyEvent) const
{
	if (!bAnalogNavigation) return EUINavigation::Invalid;

	if (InKeyEvent.GetKey() == EKeys::Gamepad_LeftStick_Up)
	{
		return EUINavigation::Up;
	}
	if (InKeyEvent.GetKey() == EKeys::Gamepad_LeftStick_Down)
	{
		return EUINavigation::Down;
	}
	if (InKeyEvent.GetKey() == EKeys::Gamepad_LeftStick_Right)
	{
		return EUINavigation::Right;
	}
	if (InKeyEvent.GetKey() == EKeys::Gamepad_LeftStick_Left)
	{
		return EUINavigation::Left;
	}

	return EUINavigation::Invalid;
}

TArray<FKey> FUINavigationConfig::GetKeysForDirection(const EUINavigation Direction) const
{
	TArray<FKey> DirectionsKeys;
	for (const TPair<FKey, EUINavigation>& KeyEventRule : KeyEventRules)
	{
		if (KeyEventRule.Value == Direction)
		{
			DirectionsKeys.Add(KeyEventRule.Key);
		}
	}
	return DirectionsKeys;
}
