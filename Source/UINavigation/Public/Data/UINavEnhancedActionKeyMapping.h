// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "UObject/SoftObjectPtr.h"
#include "EnhancedActionKeyMapping.h"
#include "UINavEnhancedActionKeyMapping.generated.h"

/**
 * Defines a mapping between a key activation and the resulting enhanced action
 * An key could be a button press, joystick axis movement, etc.
 * An enhanced action could be MoveForward, Jump, Fire, etc.
 *
**/
USTRUCT(BlueprintType)
struct FUINavEnhancedActionKeyMapping
{
	GENERATED_BODY()

	/** Action to be affected by the key  */
	UPROPERTY(EditAnywhere, Category = "Input")
	TSoftObjectPtr<const UInputAction> Action = nullptr;

	/** Key that affect the action. */
	UPROPERTY(EditAnywhere, Category = "Input")
	FKey Key = EKeys::Invalid;

	/**
	* Trigger qualifiers. If any trigger qualifiers exist the mapping will not trigger unless:
	* If there are any Explicit triggers in this list at least one of them must be met.
	* All Implicit triggers in this list must be met.
	*/
	UPROPERTY(EditAnywhere, Category = "Input")
	TArray<TSoftObjectPtr<class UInputTrigger>> Triggers;

	/**
	* Modifiers applied to the raw key value.
	* These are applied sequentially in array order.
	*/
	UPROPERTY(EditAnywhere, Category = "Input")
	TArray<TSoftObjectPtr<class UInputModifier>> Modifiers;

	FUINavEnhancedActionKeyMapping()
		: Action(nullptr)
		, Key(EKeys::Invalid)
	{ }

	FUINavEnhancedActionKeyMapping(const FEnhancedActionKeyMapping& InMapping)
		: Action(TSoftObjectPtr<const UInputAction>(InMapping.Action))
		, Key(InMapping.Key)
		, Triggers(InMapping.Triggers)
		, Modifiers(InMapping.Modifiers)
	{}

	bool operator==(const FUINavEnhancedActionKeyMapping& Other) const
	{
		return (Action == Other.Action &&
			Key == Other.Key &&
			Triggers == Other.Triggers &&
			Modifiers == Other.Modifiers);
	}

};