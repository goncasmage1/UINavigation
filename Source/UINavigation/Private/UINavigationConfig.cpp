// Copyright (C) 2023 Gon�alo Marques - All Rights Reserved

#include "UINavigationConfig.h"
#include "UINavSettings.h"
#include "Data/UINavEnhancedInputActions.h"
#include "InputMappingContext.h"
#include "UINavPCComponent.h"

FUINavigationConfig::FUINavigationConfig(const UUINavPCComponent* const UINavPC, const bool bAllowDirectionalInput /*= true*/, const bool bAllowSectionInput /*= true*/, const bool bAllowAccept /*= true*/, const bool bAllowBack /*= true*/, const bool bUseAnalogDirectionalInput /*= true*/, const bool bUsingThumbstickAsMouse /*= false*/)
{
	KeyEventRules.Reset();
	bTabNavigation = false;
	bKeyNavigation = true;
	bAnalogNavigation = bUseAnalogDirectionalInput && !bUsingThumbstickAsMouse;
	const UInputMappingContext* const InputContext = UINavPC->GetUINavInputContext();
	const UUINavSettings* const UINavSettings = GetDefault<UUINavSettings>();
	const UUINavEnhancedInputActions* const InputActions = UINavSettings->EnhancedInputActions.LoadSynchronous();
	if (InputActions == nullptr || InputContext == nullptr)
	{
		return;
	}

	for (const FEnhancedActionKeyMapping& Mapping : InputContext->GetMappings())
	{
		if (bAllowDirectionalInput)
		{
			if (Mapping.Action == InputActions->IA_MenuUp)
			{
				KeyEventRules.Emplace(UINavPC->GetCurrentKey(Mapping), EUINavigation::Up);
			}
			else if (Mapping.Action == InputActions->IA_MenuDown)
			{
				KeyEventRules.Emplace(UINavPC->GetCurrentKey(Mapping), EUINavigation::Down);
			}
			else if (Mapping.Action == InputActions->IA_MenuLeft)
			{
				KeyEventRules.Emplace(UINavPC->GetCurrentKey(Mapping), EUINavigation::Left);
			}
			else if (Mapping.Action == InputActions->IA_MenuRight)
			{
				KeyEventRules.Emplace(UINavPC->GetCurrentKey(Mapping), EUINavigation::Right);
			}
		}
		
		if (bAllowSectionInput)
		{
			if (Mapping.Action == InputActions->IA_MenuNext)
			{
				KeyEventRules.Emplace(UINavPC->GetCurrentKey(Mapping), EUINavigation::Next);
			}
			else if (Mapping.Action == InputActions->IA_MenuPrevious)
			{
				KeyEventRules.Emplace(UINavPC->GetCurrentKey(Mapping), EUINavigation::Previous);
			}
		}

		if (bAllowAccept && Mapping.Action == InputActions->IA_MenuSelect)
		{
			const bool bIsGamepadKey = UINavPC->GetCurrentKey(Mapping).IsGamepadKey();
			if (bIsGamepadKey)
			{
				GamepadSelectKeys.AddUnique(UINavPC->GetCurrentKey(Mapping));
			}

			if (!bIsGamepadKey || !bUsingThumbstickAsMouse)
			{
				KeyActionRules.Emplace(UINavPC->GetCurrentKey(Mapping), EUINavigationAction::Accept);
			}
		}
		else if (bAllowBack && Mapping.Action == InputActions->IA_MenuReturn)
		{
			KeyActionRules.Emplace(UINavPC->GetCurrentKey(Mapping), EUINavigationAction::Back);
		}
	}
}

EUINavigationAction FUINavigationConfig::GetNavigationActionForKey(const FKey& InKey) const
{
	const EUINavigationAction* NavAction = KeyActionRules.Find(InKey);
	return NavAction == nullptr ? EUINavigationAction::Invalid : *NavAction;
}

EUINavigation FUINavigationConfig::GetNavigationDirectionFromAnalog(const FAnalogInputEvent& InAnalogEvent)
{
	if (!bAnalogNavigation || (InAnalogEvent.GetKey() != EKeys::Gamepad_LeftX && InAnalogEvent.GetKey() != EKeys::Gamepad_LeftY))
	{
		return EUINavigation::Invalid;
	}

	return FNavigationConfig::GetNavigationDirectionFromAnalog(InAnalogEvent);
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
