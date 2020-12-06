// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#pragma once

#include "Framework/Application/IInputProcessor.h"

/**
* 
*/
class UINAVIGATION_API FUINavInputProcessor : public IInputProcessor
{

protected:

	class UUINavPCComponent* UINavPC;

public:

	void SetUINavPC(UUINavPCComponent* NewUINavPC)
	{
		UINavPC = NewUINavPC;
	}
	
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override;

	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGesture) override;
};
