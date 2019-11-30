// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#pragma once

#include "UINavWidget.h"
#include "Data/InputCollisionData.h"
#include "SwapKeysWidget.generated.h"

UCLASS()
class UINAVIGATION_API USwapKeysWidget : public UUINavWidget
{
	GENERATED_BODY()

public:

	virtual void OnSelect_Implementation(int Index) override;

	virtual void OnReturn_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = SwapKeysWidget)
	void NotifySwapResult(bool bAccept);


	UPROPERTY(BlueprintReadOnly, Category = SwapKeysWidget)
	FInputCollisionData InputCollisionData;

	class UUINavInputBox* CurrentInputBox;
	class UUINavInputBox* CollidingInputBox;
	
};
