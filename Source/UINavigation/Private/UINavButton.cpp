// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#include "UINavButton.h"
#include "UINavComponent.h"
#include "Internationalization/Internationalization.h"

UUINavButton::UUINavButton()
{
	IsFocusable = true;
}

#if WITH_EDITOR
const FText UUINavButton::GetPaletteCategory()
{
	return NSLOCTEXT("UMG", "UINav", "UINav");
}
#endif

bool UUINavButton::IsValid(const bool bIgnoreDisabledUINavButton) const
{
	return (Visibility != ESlateVisibility::Collapsed &&
			Visibility != ESlateVisibility::Hidden &&
			(!bIgnoreDisabledUINavButton || bIsEnabled));
}
