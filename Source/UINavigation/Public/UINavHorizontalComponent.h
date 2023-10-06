// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UINavComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "UINavHorizontalComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnValueChangedEvent);

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavHorizontalComponent : public UUINavComponent
{
	GENERATED_BODY()

protected:
	virtual FNavigationReply NativeOnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply) override;

public:

	//Indicates the option that should appear first in the slider
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavHorizontalComponent)
	int OptionIndex = 0;

	//If set to true, will loop between options (won't disable buttons, even if DisableButtons is set to true)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavComponentBox)
	bool bLoopOptions = false;

	int LastOptionIndex = -1;

	UPROPERTY(BlueprintAssignable, Category = "Appearance|Event")
	FOnValueChangedEvent OnValueChanged;
	DECLARE_EVENT(UUserWidget, FNativeOnClickedEvent);
	FOnValueChangedEvent OnNativeValueChanged;

	UFUNCTION(BlueprintCallable, Category = UINavHorizontalComponent)
	virtual bool Update(const bool bNotify = true);

	virtual void NotifyUpdated();

	//Changes the text displayed to match the specified option index
	UFUNCTION(BlueprintCallable, Category = UINavComponentBox)
	virtual bool SetOptionIndex(int NewIndex);

	UFUNCTION(BlueprintCallable, Category = UINavComponentBox)
	virtual FORCEINLINE int GetMaxOptionIndex() const { return 0; }

	UFUNCTION(BlueprintCallable, Category = UINavHorizontalComponent)
	virtual void NavigateLeft();

	UFUNCTION(BlueprintCallable, Category = UINavHorizontalComponent)
	virtual void NavigateRight();

	virtual void NotifyNavigateLeft();
	virtual void NotifyNavigateRight();

	UFUNCTION(BlueprintNativeEvent, Category = UINavHorizontalComponent)
	void OnNavigateLeft();

	virtual void OnNavigateLeft_Implementation();
	
	UFUNCTION(BlueprintNativeEvent, Category = UINavHorizontalComponent)
	void OnNavigateRight();
	
	virtual void OnNavigateRight_Implementation();
	
	UFUNCTION(BlueprintNativeEvent, Category = UINavHorizontalComponent)
	void OnUpdated();

	virtual void OnUpdated_Implementation();
	
};
