// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavButtonBase.h"
#include "SUINavButton.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Components/ButtonSlot.h"

void UUINavButtonBase::SetIsFocusable(const bool bInIsButtonFocusable)
{
	InitIsFocusable(bInIsButtonFocusable);
	if (MyUINavButton.IsValid())
	{
		MyUINavButton->SetIsButtonFocusable(bInIsButtonFocusable);
	}
}

TSharedRef<SWidget> UUINavButtonBase::RebuildWidget()
{
	MyButton = MyUINavButton = SNew(SUINavButton)
		.OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, SlateHandleClicked))
		.OnPressed(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandlePressed))
		.OnReleased(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleReleased))
		.OnHovered_UObject(this, &ThisClass::SlateHandleHovered)
		.OnUnhovered_UObject(this, &ThisClass::SlateHandleUnhovered)
		.ButtonStyle(&GetStyle())
		.ClickMethod(GetClickMethod())
		.TouchMethod(GetTouchMethod())
		.PressMethod(GetPressMethod())
		.IsFocusable(GetIsFocusable());

	if (GetChildrenCount() > 0)
	{
		Cast<UButtonSlot>(GetContentSlot())->BuildSlot(MyUINavButton.ToSharedRef());
	}

	return MyButton.ToSharedRef();
}

void UUINavButtonBase::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MyUINavButton.Reset();
}