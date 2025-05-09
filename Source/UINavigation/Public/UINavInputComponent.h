// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UINavComponent.h"
#include "UINavInputComponent.generated.h"

class UTextBlock;
class URichTextBlock;
class FText;

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavInputComponent : public UUINavComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
	class UImage* InputImage = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "UINav Input")
	class UTextBlock* LeftText = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "UINav Input")
	class UTextBlock* RightText = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "UINav Input")
	class URichTextBlock* LeftRichText = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "UINav Input")
	class URichTextBlock* RightRichText = nullptr;

public:
	void SetIsHold(const FText& HoldText, const bool bIsHold);

};
