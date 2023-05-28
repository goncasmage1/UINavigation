// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "UINavComponentAction.generated.h"

class UUINavComponent;

USTRUCT(Blueprintable, BlueprintType)
struct FComponentActions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "UINavComponentAction")
	TArray<UUINavComponentAction*> Actions;
};

UENUM(BlueprintType)
enum class EComponentAction : uint8
{
	OnClicked,
	OnPressed,
	OnReleased,
	OnNavigatedTo,
	OnNavigatedFrom
};

/**
 * 
 */
UCLASS(Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class UINAVIGATION_API UUINavComponentAction : public UObject
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintNativeEvent, Category = "UINavComponentAction")
	void ExecuteAction(UUINavComponent* Component);
	virtual void ExecuteAction_Implementation(UUINavComponent* Component) {}

};
