// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavInputProcessor.h"
#include "UINavPCComponent.h"

void FUINavInputProcessor::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
{
}

bool FUINavInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (UINavPC != nullptr)
	{
		UINavPC->HandleKeyDownEvent(SlateApp, InKeyEvent);
	}

	return IInputProcessor::HandleKeyDownEvent(SlateApp, InKeyEvent);
}

bool FUINavInputProcessor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (UINavPC != nullptr)
	{
		UINavPC->HandleKeyUpEvent(SlateApp, InKeyEvent);
	}

	return IInputProcessor::HandleKeyUpEvent(SlateApp, InKeyEvent);
}

bool FUINavInputProcessor::HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent)
{
	if (UINavPC != nullptr)
	{
		UINavPC->HandleAnalogInputEvent(SlateApp, InAnalogInputEvent);
	}

	return IInputProcessor::HandleAnalogInputEvent(SlateApp, InAnalogInputEvent);
}

bool FUINavInputProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (UINavPC != nullptr)
	{
		UINavPC->HandleMouseMoveEvent(SlateApp, MouseEvent);
	}

	return IInputProcessor::HandleMouseMoveEvent(SlateApp, MouseEvent);
}

bool FUINavInputProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (UINavPC != nullptr)
	{
		UINavPC->HandleMouseButtonDownEvent(SlateApp, MouseEvent);
	}

	return IInputProcessor::HandleMouseButtonDownEvent(SlateApp, MouseEvent);
}

bool FUINavInputProcessor::HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (UINavPC != nullptr)
	{
		UINavPC->HandleMouseButtonUpEvent(SlateApp, MouseEvent);
	}

	return IInputProcessor::HandleMouseButtonUpEvent(SlateApp, MouseEvent);
}

bool FUINavInputProcessor::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGesture)
{
	if (UINavPC != nullptr)
	{
		UINavPC->HandleMouseWheelOrGestureEvent(SlateApp, InWheelEvent, InGesture);
	}

	return IInputProcessor::HandleMouseWheelOrGestureEvent(SlateApp, InWheelEvent, InGesture);
}