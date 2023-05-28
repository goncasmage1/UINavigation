// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "ComponentActions/GoToWidgetAction.h"
#include "UINavComponent.h"
#include "UINavWidget.h"

void UGoToWidgetAction::ExecuteAction_Implementation(UUINavComponent* Component)
{
	if (!IsValid(Component))
	{
		return;
	}

	Component->ParentWidget->GoToWidget(WidgetClass, bRemoveParent, bDestroyParent, ZOrder);
}
