// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Widgets/Input/SButton.h"

//////////////////////////////////////////////////////////////////////////
// SUINavButton
//////////////////////////////////////////////////////////////////////////
/**
 * Lets us disable clicking on a button without disabling hit-testing
 * Needed because NativeOnMouseEnter is not received by disabled widgets,
 * but that also disables our anchored tooltips.
 */
class UINAVIGATION_API SUINavButton : public SButton
{
public:
	SLATE_BEGIN_ARGS(SUINavButton)
		: _Content()
		, _HAlign(HAlign_Fill)
		, _VAlign(VAlign_Fill)
		, _ClickMethod(EButtonClickMethod::DownAndUp)
		, _TouchMethod(EButtonTouchMethod::DownAndUp)
		, _PressMethod(EButtonPressMethod::DownAndUp)
		, _IsFocusable(true)
	{}
	SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_STYLE_ARGUMENT(FButtonStyle, ButtonStyle)
		SLATE_ARGUMENT(EHorizontalAlignment, HAlign)
		SLATE_ARGUMENT(EVerticalAlignment, VAlign)
		SLATE_EVENT(FOnClicked, OnClicked)
		SLATE_EVENT(FOnClicked, OnDoubleClicked)
		SLATE_EVENT(FSimpleDelegate, OnPressed)
		SLATE_EVENT(FSimpleDelegate, OnReleased)
		SLATE_EVENT(FSimpleDelegate, OnHovered)
		SLATE_EVENT(FSimpleDelegate, OnUnhovered)
		SLATE_ARGUMENT(EButtonClickMethod::Type, ClickMethod)
		SLATE_ARGUMENT(EButtonTouchMethod::Type, TouchMethod)
		SLATE_ARGUMENT(EButtonPressMethod::Type, PressMethod)
		SLATE_ARGUMENT(bool, IsFocusable)
		SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		SButton::Construct(SButton::FArguments()
			.ButtonStyle(InArgs._ButtonStyle)
			.HAlign(InArgs._HAlign)
			.VAlign(InArgs._VAlign)
			.ClickMethod(InArgs._ClickMethod)
			.TouchMethod(InArgs._TouchMethod)
			.PressMethod(InArgs._PressMethod)
			.OnClicked(InArgs._OnClicked)
			.OnPressed(InArgs._OnPressed)
			.OnReleased(InArgs._OnReleased)
			.OnHovered(InArgs._OnHovered)
			.OnUnhovered(InArgs._OnUnhovered)
			.IsFocusable(InArgs._IsFocusable)
			.Content()
			[
				InArgs._Content.Widget
			]);
	}

	void SetIsButtonFocusable(bool bInIsButtonFocusable);

};