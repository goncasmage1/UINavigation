// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Delegates/DelegateCombinations.h"
#include "Fonts/SlateFontInfo.h"
#include "Animation/WidgetAnimation.h"
#include "ComponentActions/UINavComponentAction.h"
#include "Sound/SoundBase.h"
#include "UINavComponent.generated.h"

class UUINavWidget;
class UTextBlock;
class URichTextBlock;
class UScrollBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClickedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPressedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReleasedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNavigatedToEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNavigatedFromEvent);

UENUM(BlueprintType, meta = (ScriptName = "UINavButtonStyle"))
enum class EButtonStyle : uint8
{
	None UMETA(DisplayName = "None"),
	Normal UMETA(DisplayName = "Normal"),
	Hovered UMETA(DisplayName = "Hovered"),
	Pressed UMETA(DisplayName = "Pressed")
};

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavComponent : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UUINavComponent(const FObjectInitializer& ObjectInitializer);

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual bool Initialize() override;

	virtual void SetFocusable(const bool bNewIsFocusable);

	UFUNCTION(BlueprintNativeEvent, Category = UINavComponent)
	void OnNavigatedTo();

	virtual void OnNavigatedTo_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = UINavComponent)
	void OnNavigatedFrom();

	virtual void OnNavigatedFrom_Implementation();

	void HandleFocusReceived();
	void HandleFocusLost();

	UFUNCTION()
	void OnButtonClicked();
	UFUNCTION()
	void OnButtonPressed();
	UFUNCTION()
	void OnButtonReleased();
	UFUNCTION()
	void OnButtonHovered();
	UFUNCTION()
	void OnButtonUnhovered();

	UFUNCTION(BlueprintCallable, Category = UINavComponent)
	void SetText(const FText& Text);
	
	UFUNCTION(BlueprintCallable, Category = UINavComponent)
	void SwitchButtonStyle(const EButtonStyle NewStyle, const bool bRevertStyle = true);

	void RevertButtonStyle();

	/**
	*	Changes the color of the text with the specified index to the specified color
	*
	*	@param	Index  The new button's index in the Button's array
	*	@param	Color  The text's new color
	*/
	UFUNCTION(BlueprintCallable, Category = UINavComponent)
	void SwitchTextColorTo(FLinearColor Color);

	UFUNCTION(BlueprintCallable, Category = UINavComponent)
	void SwitchTextColorToDefault();

	UFUNCTION(BlueprintCallable, Category = UINavComponent)
	void SwitchTextColorToNavigated();

	USoundBase* GetOnNavigatedSound() const { return Cast<USoundBase>(NavigatedSlateSound.GetResourceObject()); }

	void ExecuteComponentActions(const EComponentAction Action);

	UWidgetAnimation* GetComponentAnimation() const { return ComponentAnimation; }

	bool UseComponentAnimation() const { return bUseComponentAnimation; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavComponent)
	bool CanBeNavigated() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavComponent)
	bool IsBeingNavigated() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavComponent)
	UScrollBox* GetParentScrollBox() const { return ParentScrollBox; }

protected:

	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnFocusLost(const FFocusEvent& InFocusEvent) override;

	virtual void NativeOnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) override;
	virtual FNavigationReply NativeOnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply) override;

	virtual void NativePreConstruct() override;

	void SwapStyle(EButtonStyle Style1, EButtonStyle Style2);

	EButtonStyle GetStyleFromButtonState();

public:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = UINavComponent)
	UButton* NavButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = UINavComponent)
	UTextBlock* NavText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = UINavComponent)
	URichTextBlock* NavRichText = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = UINavComponent)
	UUINavWidget* ParentWidget = nullptr;

	UPROPERTY(BlueprintAssignable, Category = "Appearance|Event")
	FOnClickedEvent OnClicked;
	DECLARE_EVENT(UUserWidget, FNativeOnClickedEvent);
	FNativeOnClickedEvent OnNativeClicked;

	UPROPERTY(BlueprintAssignable, Category = "Appearance|Event")
	FOnPressedEvent OnPressed;
	DECLARE_EVENT(UUserWidget, FNativeOnPressedEvent);
	FNativeOnPressedEvent OnNativePressed;

	UPROPERTY(BlueprintAssignable, Category = "Appearance|Event")
	FOnReleasedEvent OnReleased;
	DECLARE_EVENT(UUserWidget, FNativeOnReleasedEvent);
	FNativeOnReleasedEvent OnNativeReleased;

	UPROPERTY(BlueprintAssignable, Category = "Appearance|Event")
	FOnNavigatedToEvent OnNavigatedToEvent;
	DECLARE_EVENT(UUserWidget, FNativeOnNavigatedToEvent);
	FNativeOnNavigatedToEvent OnNativeNavigatedToEvent;

	UPROPERTY(BlueprintAssignable, Category = "Appearance|Event")
	FOnNavigatedFromEvent OnNavigatedFromEvent;
	DECLARE_EVENT(UUserWidget, FNativeOnNavigatedFromEvent);
	FNativeOnNavigatedFromEvent OnNativeNavigatedFromEvent;

	EButtonStyle CurrentStyle = EButtonStyle::Normal;

	EButtonStyle ForcedStyle = EButtonStyle::None;

	bool bIgnoreDueToRebind = false;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavComponent)
	FText ComponentText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavComponent)
	bool bUseTextColor = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavComponent, meta = (editcondition = "bUseTextColor"))
	FLinearColor TextDefaultColor = FColor::Blue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavComponent, meta = (editcondition = "bUseTextColor"))
	FLinearColor TextNavigatedColor = FColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavComponent)
	bool bUseComponentAnimation = true;

	// Applied to the optional NavRichText text block if it exists and if this string isn't empty
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavComponent)
	FString NormalStyleRowName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavComponent, meta = (editcondition = "bUseNavigatedStyleRow"))
	FString NavigatedStyleRowName;

	UPROPERTY(EditAnywhere, Category = UINavComponent, meta = (InlineEditConditionToggle))
	uint8 bUseNavigatedStyleRow : 1;

	UPROPERTY(BlueprintReadOnly, Transient, Category = UINavComponent, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* ComponentAnimation = nullptr;

	UPROPERTY()
	UScrollBox* ParentScrollBox = nullptr;

	/**
	 * The sound the button should play when initially Navigated over
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UINavComponent, meta = (DisplayName = "Navigated Sound"))
	FSlateSound NavigatedSlateSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavComponent, meta = (editcondition = "bOverride_Font"))
	FSlateFontInfo FontOverride;

	UPROPERTY(EditAnywhere, Category = UINavComponent, meta = (InlineEditConditionToggle))
	uint8 bOverride_Font : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavComponent, meta = (editcondition = "bOverride_Style"))
	FButtonStyle StyleOverride;

	UPROPERTY(EditAnywhere, Category = UINavComponent, meta = (InlineEditConditionToggle))
	uint8 bOverride_Style : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UINavComponent)
	TMap<EComponentAction, FComponentActions> ComponentActions;

};
