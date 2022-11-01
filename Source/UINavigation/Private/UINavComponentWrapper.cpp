// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavComponentWrapper.h"
#include "UINavCollection.h"

void UUINavComponentWrapper::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	UUINavComponent* NavComponent = GetUINavComponent();
	
	if(NavComponent != nullptr && NavComponent->ParentWrapper == nullptr)
	{
		NavComponent->ParentWrapper = this;
	}
}

void UUINavComponentWrapper::RefreshNavComponent()
{
	OnRefreshNavComponent();
}

void UUINavComponentWrapper::NavComponentUpdated()
{
	OnNavComponentUpdated();

	// Loop over all the Linked UINavWrapper declared in the UINavCollection
	for (UUINavComponentWrapper* NavWrapper : LinkedUINavWrapperToRefresh)
	{
		if(NavWrapper != nullptr)
		{
			NavWrapper->RefreshNavComponent();			
		}
	}

	if(ParentCollection != nullptr)
		ParentCollection->OnNavComponentUpdated();
}
