// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Blueprint/UserWidget.h"
#include "UINavInputContainer.generated.h"

UENUM(BlueprintType)
enum class EInputRestriction : uint8
{
	None UMETA(DisplayName = "None"),
	Keyboard UMETA(DisplayName = "Keyboard"),
	Mouse UMETA(DisplayName = "Mouse"),
	Keyboard_Mouse UMETA(DisplayName = "Keyboard and Mouse"),
	Gamepad UMETA(DisplayName = "Gamepad"),
};

USTRUCT(Blueprintable, BlueprintType)
struct FInputIconMapping : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UINav Input")
		TAssetPtr<class UTexture2D> InputIcon;
};

/**
* This class contains the logic for aggregating several input boxes
*/
UCLASS()
class UINAVIGATION_API UUINavInputContainer : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	void CreateInputBoxes();

	//-----------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, Category = "UINav Input")
		int FirstButtonIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = "UINav Input")
		int NumberOfActions = -1;

	/*
	The desired InputBox widget blueprint
	*/
	UPROPERTY(EditDefaultsOnly, Category = "UINav Input")
		TSubclassOf<class UUINavInputBox> InputBox_BP;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
		class UPanelWidget* Panel;
	
	class UUINavWidget* ParentWidget;

public:

	void SetParentWidget(class UUINavWidget* NewParent);

	UFUNCTION(BlueprintCallable, Category = "UINav Input")
		void ResetKeyMappings();

	UFUNCTION(BlueprintCallable, Category = "UINav Input")
		bool IsKeyBeingUsed(FKey CompareKey) const;

	UFUNCTION(BlueprintCallable, Category = "UINav Input")
		bool RespectsRestriction(FKey CompareKey, int Index);

	class UTexture2D* LoadTexture2D(const FString& FullFilePath, bool& IsValid, int32& Width, int32& Height);

	//-----------------------------------------------------------------------

	/*
	Holds the key icons for the gamepad
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		UDataTable* GamepadKeyData;

	/*
	Holds the key icons for mouse and keyboard
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		UDataTable* KeyboardMouseKeyData;

	UPROPERTY(BlueprintReadOnly, Category = "UINav Input")
		int InputsPerAction = 0;

	/*
	The names of the desired actions to allow for rebinding
	Can be left empty if all actions are to be rebindable
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		TArray<FName> ActionNames;

	/*
	The restrictions for the type of input associated with each column
	in the Input Container
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
		TArray<EInputRestriction> InputRestrictions;

	FStreamableManager AssetLoader;

};
