// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved


#include "UINavPCReceiver.h"

void IUINavPCReceiver::OnRootWidgetAdded_Implementation()
{
}

void IUINavPCReceiver::OnRootWidgetRemoved_Implementation()
{
}

void IUINavPCReceiver::OnInputChanged_Implementation(EInputType From, EInputType To)
{
}

void IUINavPCReceiver::OnThumbstickCursorInput_Implementation(const FVector2D& ThumbstickDelta)
{
}

void IUINavPCReceiver::OnControllerConnectionChanged_Implementation(bool bConnected, int32 UserId, int32 UserIndex)
{
}

void IUINavPCReceiver::OnActiveWidgetChanged_Implementation(UUINavWidget* OldActiveWidget, UUINavWidget* NewActiveWidget)
{
}

void IUINavPCReceiver::OnNavigated_Implementation(EUINavigation NewDirection)
{
}

void IUINavPCReceiver::OnSelect_Implementation()
{
}

void IUINavPCReceiver::OnReturn_Implementation()
{
}

void IUINavPCReceiver::OnNext_Implementation()
{
}

void IUINavPCReceiver::OnPrevious_Implementation()
{
}
