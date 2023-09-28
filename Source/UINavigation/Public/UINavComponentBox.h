// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UINavHorizontalComponent.h"
#include "UINavComponentBox.generated.h"

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavComponentBox : public UUINavHorizontalComponent
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(BlueprintReadWrite, Category = UINavComponentBox, meta = (BindWidget))
	class UButton* LeftButton = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category = UINavComponentBox, meta = (BindWidget))
	class UButton* RightButton = nullptr;

	virtual void CheckLeftLimit();
	virtual void CheckRightLimit();

public:

	virtual void NativeConstruct() override;

	virtual bool SetOptionIndex(const int NewIndex) override;

	void BaseConstruct();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavComponentBox)
	int MinRange = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavComponentBox)
	int MaxRange = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavComponentBox, meta = (ClampMin="1"))
	int Interval = 1;

	//If set to true, will disable buttons if the slider runs out of options on either side
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavComponentBox)
	bool bDisableButtons = true;

	//Returns the currently selected number resulting from the number range
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = UINavComponentBox)
	FORCEINLINE int GetCurrentNumber() const { return (MinRange + OptionIndex * Interval); }

	virtual void NavigateLeft() override;
	virtual void NavigateRight() override;

};
