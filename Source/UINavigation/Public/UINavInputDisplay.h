// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "InputAction.h"
#include "Data/InputType.h"
#include "Data/AxisType.h"
#include "Data/InputContainerEnhancedActionData.h"
#include "Math/Vector2D.h"
#include "UINavInputDisplay.generated.h"

class UImage;
class UTextBlock;
class UUINavPCComponent;

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavInputDisplay : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void NativePreConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "InputDisplay")
	void UpdateInputVisuals();

	UFUNCTION(BlueprintCallable, Category = "InputDisplay")
	void SetInputAction(UInputAction* NewAction, const EInputAxis NewAxis, const EAxisType NewScale);

protected:

	UFUNCTION()
	void InputTypeChanged(const EInputType NewInputType);

public:

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "InputDisplay")
	UImage* InputImage = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "InputDisplay")
	UTextBlock* InputText = nullptr;

protected:

	UPROPERTY(EditAnywhere, Category = "InputDisplay")
	UInputAction* InputAction = nullptr;

	UPROPERTY(EditAnywhere, Category = "InputDisplay")
	EInputAxis Axis = EInputAxis::X;
	
	UPROPERTY(EditAnywhere, Category = "InputDisplay")
	EAxisType Scale = EAxisType::None;

	UPROPERTY(EditAnywhere, Category = "InputDisplay")
	bool bMatchIconSize = false;

	UPROPERTY(EditAnywhere, Category = "InputDisplay", meta = (editcondition = "!bMatchIconSize"))
	FVector2D IconSize = FVector2D(24.0f, 24.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavComponent, meta = (editcondition = "bOverride_TextColor"))
	FSlateColor TextColorOverride;

	UPROPERTY(EditAnywhere, Category = UINavComponent, meta = (InlineEditConditionToggle))
	uint8 bOverride_TextColor : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavComponent, meta = (editcondition = "bOverride_Font"))
	FSlateFontInfo FontOverride;

	UPROPERTY(EditAnywhere, Category = UINavComponent, meta = (InlineEditConditionToggle))
	uint8 bOverride_Font : 1;

private:

	UUINavPCComponent* UINavPC = nullptr;
	
};
 