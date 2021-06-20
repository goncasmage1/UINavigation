// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#pragma once

#include "Components/Button.h"
#include "UINavButton.generated.h"

class UUINavComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomHoverDelegate, int, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomUnhoverDelegate, int, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomClickDelegate, int, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomPressDelegate, int, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomReleaseDelegate, int, Index);

UENUM(BlueprintType, meta = (ScriptName = "UINavButtonStyle"))
enum class EButtonStyle : uint8
{
	None UMETA(DisplayName = "None"),
	Normal UMETA(DisplayName = "Normal"),
	Hovered UMETA(DisplayName = "Hovered"),
	Pressed UMETA(DisplayName = "Pressed")
};

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
	not correctly aligned with the automatically set indices several bugs may appear.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavButton)
		int ButtonIndex = -1;

	//The index of the grid this button is in
	UPROPERTY(BlueprintReadOnly, Category = UINavButton)
		int GridIndex = - 1;
	//This button's index in its associated grid
	UPROPERTY(BlueprintReadOnly, Category = UINavButton)
		int IndexInGrid = - 1;

	UPROPERTY(BlueprintReadOnly, Category = UINavButton)
		UUINavComponent* NavComp = nullptr;

	EButtonStyle CurrentStyle = EButtonStyle::Normal;

	EButtonStyle ForcedStyle = EButtonStyle::None;

	bool bAutoCollapse = false;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavButton)
		bool IsValid(const bool bIgnoreDisabledUINavButton = true) const;

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
