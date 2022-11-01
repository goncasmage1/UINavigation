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

void UUINavComponentWrapper::NavComponentUpdated()
{
	OnNavComponentUpdated();

	if(ParentCollection != nullptr)
		ParentCollection->OnNavComponentUpdated();
}
