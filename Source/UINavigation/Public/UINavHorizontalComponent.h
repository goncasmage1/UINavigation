// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UINavComponent.h"
#include "UINavHorizontalComponent.generated.h"

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavHorizontalComponent : public UUINavComponent
{
	GENERATED_BODY()

public:

	//Indicates the option that should appear first in the slider
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavHorizontalComponent)
	int OptionIndex = 0;

	//If set to true, will loop between options (won't disable buttons, even if DisableButtons is set to true)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavComponentBox)
	bool bLoopOptions = false;

	int LastOptionIndex = -1;

	UPROPERTY()
	class UUINavWidget* ParentWidget = nullptr;

	UFUNCTION(BlueprintCallable, Category = UINavHorizontalComponent)
	virtual void Update();

	//Changes the text displayed to match the specified option index
	UFUNCTION(BlueprintCallable, Category = UINavComponentBox, meta = (DisplayName = "Set Option Index"))
	virtual void UpdateTextToIndex(int NewIndex);

	//Changes the text displayed in the NavText element
	UFUNCTION(BlueprintCallable, Category = UINavComponentBox)
	void ChangeText(const FText NewText);

	UFUNCTION(BlueprintCallable, Category = UINavComponentBox)
	virtual FORCEINLINE int GetMaxOptionIndex() const { return 0; }

	UFUNCTION(BlueprintCallable, Category = UINavHorizontalComponent)
	virtual void NavigateLeft();

	UFUNCTION(BlueprintCallable, Category = UINavHorizontalComponent)
	virtual void NavigateRight();

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
