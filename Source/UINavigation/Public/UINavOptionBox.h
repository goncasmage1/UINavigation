// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UINavComponentBox.h"
#include "UINavOptionBox.generated.h"

#define DISPLAYERROR(Text) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("%s"), *(FString(TEXT("Error in ")).Append(GetName()).Append(TEXT(": ")).Append(Text))));

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavOptionBox : public UUINavComponentBox
{
	GENERATED_BODY()
	
protected:

	virtual void CheckRightLimit() override;

	virtual void Update() override;

public:

	virtual void NativeConstruct() override;

	virtual int GetLastOptionIndex() override;

	/*If set to false, will use StringOptions, otherwise will use
	all integers in designated range (from MinRange to MaxRange, inclusive)*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavOptionBox)
		bool bUseNumberRange = false;

	//The list of Names to display as options in this slider
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavOptionBox, meta = (EditCondition = "!bUseNumberRange"))
		TArray<FText> StringOptions;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavOptionBox)
		FORCEINLINE FText GetCurrentString() const { return StringOptions[OptionIndex]; }

	virtual void NavigateRight() override;

};
