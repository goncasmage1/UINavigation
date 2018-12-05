// Copyright (C) 2018 Gonçalo Marques - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UINavComponent.h"
#include "UINavComponentBox.generated.h"

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavComponentBox : public UUINavComponent
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(BlueprintReadOnly, Category = UINavComponentBox)
		int OptionIndex = 0;

	UPROPERTY(BlueprintReadWrite, Category = UINavComponentBox, meta = (BindWidget))
		class UButton* LeftButton;
	UPROPERTY(BlueprintReadWrite, Category = UINavComponentBox, meta = (BindWidget))
		class UButton* RightButton;

	virtual void CheckLeftLimit();
	virtual void CheckRightLimit();

	virtual void UpdateTextBlock();
	UFUNCTION(BlueprintCallable, Category = UINavComponentBox)
		virtual void UpdateTextToIndex(int NewIndex);

public:

	virtual void NativeConstruct() override;

	void BaseConstruct();

	//Indicates the option that should appear first in the slider
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavComponentBox)
		int DefaultOptionIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavComponentBox)
		int MinRange = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavComponentBox)
		int MaxRange = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavComponentBox)
		int Interval = 1;

	//If set to true, will disable buttons if the slider runs out of options on either side
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavComponentBox)
		bool bDisableButtons = true;
	
	UFUNCTION(BlueprintCallable, Category = UINavComponentBox)
		virtual void NavigateLeft();
	UFUNCTION(BlueprintCallable, Category = UINavComponentBox)
		virtual void NavigateRight();

	UFUNCTION(BlueprintNativeEvent, Category = UINavComponentBox)
		void OnNavigateLeft();
	void OnNavigateLeft_Implementation();
	UFUNCTION(BlueprintNativeEvent, Category = UINavComponentBox)
		void OnNavigateRight();
	void OnNavigateRight_Implementation();
	
};
