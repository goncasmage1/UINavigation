// Copyright (C) 2018 Gonçalo Marques - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "UINavButton.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomHoverDelegate, int, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomUnhoverDelegate, int, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomClickDelegate, int, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomReleaseDelegate, int, Index);

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavButton : public UButton
{
	GENERATED_BODY()
	
public:

	UUINavButton();

	UPROPERTY()
		FCustomHoverDelegate CustomHover;
	UPROPERTY()
		FCustomUnhoverDelegate CustomUnhover;
	UPROPERTY()
		FCustomUnhoverDelegate CustomClick;
	UPROPERTY()
		FCustomReleaseDelegate CustomRelease;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavButton)
		int ButtonIndex = -1;


	UFUNCTION()
		void OnHover();
	UFUNCTION()
		void OnUnhover();
	UFUNCTION()
		void OnClick();
	UFUNCTION()
		void OnRelease();

	void SetButtonIndex(int NewIndex);
	
};
