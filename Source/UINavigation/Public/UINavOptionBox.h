// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UINavComponentBox.h"
#include "UINavOptionBox.generated.h"

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavOptionBox : public UUINavComponentBox
{
	GENERATED_BODY()
	
protected:

	virtual bool Update(const bool bNotify = true) override;

public:

	virtual void NativeConstruct() override;

	virtual int GetMaxOptionIndex() const override;

	/*If set to false, will use StringOptions, otherwise will use
	all integers in designated range (from MinRange to MaxRange, inclusive)*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavOptionBox)
	bool bUseNumberRange = false;

	//The list of Names to display as options in this slider
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavOptionBox, meta = (EditCondition = "!bUseNumberRange"))
	TArray<FText> StringOptions;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavOptionBox)
	FORCEINLINE FText GetCurrentString() const { return StringOptions.IsValidIndex(OptionIndex) ? StringOptions[OptionIndex] : FText(); }

};
