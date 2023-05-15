// Fill out your copyright notice in the Description page of Project Settings.

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
