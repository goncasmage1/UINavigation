// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
#include "UINavLocalPlayerSubsystem.generated.h"

class FSubsystemCollectionBase;
class UInputMappingContext;

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavLocalPlayerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void SaveInputContextState(UInputMappingContext* InputContext);
	
	void ApplySavedInputContexts();
};
