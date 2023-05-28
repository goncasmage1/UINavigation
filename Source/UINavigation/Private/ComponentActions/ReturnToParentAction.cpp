// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "ComponentActions/ReturnToParentAction.h"
#include "UINavComponent.h"
#include "UINavWidget.h"

void UReturnToParentAction::ExecuteAction_Implementation(UUINavComponent* Component)
{
	if (!IsValid(Component))
	{
		return;
	}

	Component->ParentWidget->ReturnToParent(bRemoveAllParents);
}
