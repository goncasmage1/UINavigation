// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "UINavSectionsWidget.generated.h"

class UPanelWidget;

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavSectionsWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UINavSectionsWidget")
	UPanelWidget* SectionButtonsPanel = nullptr;

};
 
