// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#pragma once

#include "Components/Button.h"
#include "Data/ButtonStyle.h"
#include "UINavButton.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomHoverDelegate, int, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomUnhoverDelegate, int, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomClickDelegate, int, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomPressDelegate, int, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomReleaseDelegate, int, Index);

/**
 * This class represents a required button type for the UI Navigation plugin
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
		FCustomPressDelegate CustomPress;
	UPROPERTY()
		FCustomReleaseDelegate CustomRelease;

	/*
	WARNING: Edit this index manually at your own risk. If the indices are
	not correctly alligned with the automatically set indices several bugs may appear.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavButton)
		int ButtonIndex = -1;

	//The index of the grid this button is in
	UPROPERTY(BlueprintReadOnly, Category = UINavButton)
		int GridIndex = - 1;
	//This button's index in its associated grid
	UPROPERTY(BlueprintReadOnly, Category = UINavButton)
		int IndexInGrid = - 1;

	EButtonStyle CurrentStyle = EButtonStyle::Normal;

	EButtonStyle ForcedStyle = EButtonStyle::None;

	bool bAutoCollapse = false;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavButton)
		bool IsValid();

	UFUNCTION()
		void OnHover();
	UFUNCTION()
		void OnUnhover();
	UFUNCTION()
		void OnClick();
	UFUNCTION()
		void OnPress();
	UFUNCTION()
		void OnRelease();
	
};
