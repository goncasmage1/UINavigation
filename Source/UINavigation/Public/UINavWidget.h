// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "Blueprint/UserWidget.h"
#include "UINavWidget.generated.h"

#define DISPLAYERROR(Text) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("%s"), *(FString(TEXT("Error in ")).Append(GetName()).Append(TEXT(": ")).Append(Text))));

/**
* This class contains the logic for UserWidget navigation
*/

UENUM(BlueprintType)
enum class ESelectorPosition : uint8
{
	Position_Center UMETA(DisplayName = "Center"),
	Position_Top UMETA(DisplayName = "Top"),
	Position_Bottom UMETA(DisplayName = "Bottom"),
	Position_Left UMETA(DisplayName = "Left"),
	Position_Right UMETA(DisplayName = "Right")
};

UENUM(BlueprintType)
enum class ENavigationDirection : uint8
{
	Nav_UP,
	Nav_DOWN,
	Nav_LEFT,
	Nav_RIGHT
};

USTRUCT(BlueprintType)
struct FButtonNavigation
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
		int UpButton = -1;
	UPROPERTY(BlueprintReadWrite)
		int DownButton = -1;
	UPROPERTY(BlueprintReadWrite)
		int LeftButton = -1;
	UPROPERTY(BlueprintReadWrite)
		int RightButton = -1;
};

UCLASS()
class UINAVIGATION_API UUINavWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	bool bShouldTick = true;

	bool bMovingSelector = false;

	//Used to track when the selector's position should be updated
	int WaitForTick;

	float MovementCounter;
	float MovementTime;

	FVector2D InitialOffset;

	FVector2D SelectorOrigin;
	FVector2D SelectorDestination;
	FVector2D Distance;


	/******************************************************************************/


	/**
	*	Returns the position of the UINavButton with the specified index
	*/
	FVector2D GetButtonLocation(int Index);

	/**
	*	Creates the selector on event Construct
	*/
	void CreateSelector();

	/**
	*	Sets up the UINavButtons and does error checking
	*/
	void FetchButtonsInHierarchy();

	/**
	*	Traverses this widget's hierarchy to setup all the UINavButtons
	*/
	void TraverseHierarquy();

	/**
	*	Sets all the UTextBlocks to the default color
	*/
	void ChangeTextColorToDefault();

	void BeginSelectorMovement(int Index);
	void HandleSelectorMovement(float DeltaTime);


public:

	//The image object that will move around the UI
	UPROPERTY(BlueprintReadWrite, Category = "UINavigation")
		UImage* TheSelector;

	//Indicates the navigation possibilities of each button
	UPROPERTY(BlueprintReadWrite, Category = "UINavigation")
		TArray<FButtonNavigation> ButtonNavigations;

	//All the UINavButtons in this Widget
	UPROPERTY(BlueprintReadOnly, Category = "UINavigation")
		TArray<class UUINavButton*> NavButtons;

	UPROPERTY(BlueprintReadOnly, Category = "UINavigation")
		TArray<int> NavComponentsIndices;

	UPROPERTY(BlueprintReadOnly, Category = "UINavigation")
		TArray<class UUINavComponent*> NavComponents;

	//The indices of all the UINavOptionBoxes in this widget
	UPROPERTY(BlueprintReadOnly, Category = "UINavigation")
		TArray<int> SliderIndices;

	//An array with the indices of all the UINavOptionBoxes in this widget
	UPROPERTY(BlueprintReadOnly, Category = "UINavigation")
		TArray<class UUINavOptionBox*> Sliders;

	//All the scrollboxes in this widget
	UPROPERTY(BlueprintReadOnly, Category = "UINavigation")
		TArray<class UScrollBox*> ScrollBoxes;

	//Indicates whether the Normal and Hovered style of a button were switched
	TArray<bool> bSwitchedStyle;

	//The index of the button that is currently the focus of navigation
	int ButtonIndex = 0;

	//Reference to the parent widget that created this widget
	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "UINavigation")
		UUINavWidget* ParentWidget;

	//Reference to this widget's class
	TSubclassOf<UUINavWidget> WidgetClass;

	//Reference to the parent widget's class
	UPROPERTY(BlueprintReadWrite, Category = "UINavigation")
		TSubclassOf<UUINavWidget> ParentWidgetClass;

	//Reference to the current player controller
	class AUINavController* CurrentPC;

	//Reference to the widget that created this widget (if returned from a child)
	UPROPERTY(BlueprintReadWrite, Category = "UINavigation")
		UUINavWidget* ReturnedFromWidget;

	//Indicates whether this widget should remove its parent from the viewport when created
	bool bParentRemoved = true;


	//If set to true, buttons will be navigated using states (Normal and Hovered)
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation")
		bool bUseButtonStates = false;

	//If set to true, buttons will be navigated by changing the position of the selector
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation")
		bool bUseSelector = true;

	/*If set to true, buttons will be navigated by changing the text's color
	TEXT HAS TO BE INSIDE A UINavButton*/
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation")
		bool bUseTextColor = false;

	//The index of the button to be first navigated to (when the widget is added to viewport)
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation")
		int FirstButtonIndex = 0;

	/*If set to true, ButtonIndex will not be determined by the UINavButton's position in the
	hierarquy and remain the same, but rather be specified in the Designer Tab.*/
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation")
		bool bOverrideButtonIndeces = false;

	//If set to true, this widget will be removed if it has no ParentWidget and is returned from
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation")
		bool bAllowRemoveIfRoot = false;

	/*If set to true, this widget move the selector using a curve guiding movement animation.
	Otherwise it will snap the selector to its desired location*/
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Selector", meta = (EditCondition = "bUseSelector"))
		bool bUseMovementCurve = false;

	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Selector", meta = (EditCondition = "bUseMovementCurve"))
		UCurveFloat* MoveCurve;

	//The image of the selector
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Selector", meta = (EditCondition = "bUseSelector"))
		UTexture2D* SelectorImage;

	//The position the selector will be in relative to the button
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Selector", meta = (EditCondition = "bUseSelector"))
		ESelectorPosition SelectorPositioning = ESelectorPosition::Position_Center;

	//The scale of the selector's image
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Selector", meta = (EditCondition = "bUseSelector"))
		FVector2D SelectorScale = FVector2D(1.f, 1.f);

	//The offset to apply when positioning the selector on a button
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Selector", meta = (EditCondition = "bUseSelector"))
		FVector2D SelectorOffset;

	//The depth priority of the selector
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Selector", meta = (EditCondition = "bUseSelector"))
		int32 SelectorZOrder = -2;

	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Text", meta = (EditCondition = "bUseTextColor"))
		FLinearColor TextDefaultColor = FColor::Blue;

	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Text", meta = (EditCondition = "bUseTextColor"))
		FLinearColor TextNavigatedColor = FColor::Green;


	/*********************************************************************************/

	
	/**
	*	The widget's construct event
	*/
	virtual void NativeConstruct() override;
	/**
	*	The widget's tick event (Used to make sure Geometry is updated)
	*/
	virtual void NativeTick(const FGeometry & MyGeometry, float DeltaTime) override;

	/**
	*	Appends a new array of FButtonNavigations to the already existing navigation graph with the given dimension
	*	Used for vertical grids.
	*
	*	@param	Dimension  The amount of buttons in this vertical grid (If set to -1 will match number of UINavButtons)
	*	@param	EdgeNavigation  The intended navigation at each of the four edges of the button grid
	*	@param  bWrap  Indicates whether navigation wraps around the grid
	*	@param	StartingButtonIndex The index of the button where the grid starts
	*/
	UFUNCTION(BlueprintCallable)
		void AppendVerticalNavigation(int Dimension, FButtonNavigation EdgeNavigation, bool bWrap);

	/**
	*	Appends a new array of FButtonNavigations to the already existing navigation graph with the given dimension
	*	Used for horizontal grids.
	*
	*	@param	Dimension  The amount of buttons in this vertical grid (If set to -1 will match number of UINavButtons)
	*	@param	EdgeNavigation  The intended navigation at each of the four edges of the button grid
	*	@param  bWrap  Indicates whether navigation wraps around the grid
	*/
	UFUNCTION(BlueprintCallable)
		void AppendHorizontalNavigation(int Dimension, FButtonNavigation EdgeNavigation, bool bWrap);

	/**
	*	Appends a new array of FButtonNavigations to the already existing navigation graph with the given dimension
	*	Used for two-dimensional grids of buttons
	*	
	*	@param	DimensionX  The horizontal dimension of the grid
	*	@param	DimensionY  The vertical dimension of the grid
	*	@param	EdgeNavigation  The intended navigation at each of the four edges of the grid
	*	@param  bWrap  Indicates whether navigation wraps around the grid
	*/
	UFUNCTION(BlueprintCallable)
		void AppendGridNavigation(int DimensionX, int DimensionY, FButtonNavigation EdgeNavigation, bool bWrap);


	/**
	*	Called manually to setup all the elements in the Widget
	*
	*	@param  FirstButton The index of the button that will be the focus of navigation
	*/
	virtual void UINavSetup();

	/**
	*	Called when geometry is updated after 1st tick (ready for SetupUI)
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "UINavigation")
		void ReadyForSetup();

	/**
	*	Called when the input type changed
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "UINavigation")
		void OnInputChanged(EInputType From, EInputType TO);

	/**
	*	Changes the selector's location to that of the button with the given index in the Button's array
	*
	*	@param	Index  The new button's index in the Button's array
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		void UpdateSelectorLocation(int Index);

	/**
	*	Changes the new text and previous text's colors to the desired colors
	*
	*	@param	Index  The new button's index in the Button's array
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		void UpdateTextColor(int Index);

	/**
	*	Changes the state of the current button to normal and the new button to hovered
	*
	*	@param	Index  The new button's index in the Button's array
	*	@param  bHovered  Whether the function was called due to a button hover
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		void UpdateButtonsStates(int Index, bool bHovered);

	/**
	*	Switches the button with the given index's style between normal and hovered
	*
	*	@param Index The button's index in the Button's array
	*/
	void SwitchButtonStyle(int Index);

	/**
	*	Changes the selector's image scale to the scale given
	*
	*	@param	NewScale  The selector's image new scale
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		void ChangeSelectorScale(FVector2D NewScale);

	/**
	*	Sets this widget's selector
	*
	*	@param	NewSelector  The new selector
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		void SetSelector(class UImage* NewSelector);

	/**
	*	Changes the selector's texture
	*
	*	@param	NewBrush  The new brush
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		void SetSelectorBrush(UTexture2D* NewBrush);

	/**
	*	Navigate to the button with the specified index
	*
	*	@param	Index  The index of the button that was hovered upon
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		void NavigateTo(int Index, bool bHoverEvent = false);

	/**
	*	Called when the button with the specified index was navigated upon
	*
	*	@param	Index  The index of the button that was hovered upon
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "UINavigation")
		void OnNavigate(int From, int To);
	virtual void OnNavigate_Implementation(int From, int To);

	/**
	*	Called when ReturnToParent is called (i.e. the player wants to exit the menu)
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "UINavigation")
		void OnReturnToParent();
	virtual void OnReturnToParent_Implementation();

	/**
	*	Button Hover event
	*
	*	@param	Index  The index of the button that was hovered upon
	*/
	UFUNCTION(Category = "UINavigation")
		void HoverEvent(int Index);
	/**
	*	Button UnHover event
	*
	*	@param	Index  The index of the button that was unhovered upon
	*/
	UFUNCTION(Category = "UINavigation")
		void UnhoverEvent(int Index);
	/**
	*	Button Click event
	*
	*	@param	Index  The index of the button that was clicked upon
	*/
	UFUNCTION(Category = "UINavigation")
		void ClickEvent(int Index);

	/**
	*	Button Release event
	*
	*	@param	Index  The index of the button that was released
	*/
	UFUNCTION(Category = "UINavigation")
		void ReleaseEvent(int Index);

	/**
	*	Called in blueprint (or C++) to indicate what happens with each button's click event
	*
	*	@param	Index  The index of the button that was selected
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "UINavigation")
		void OnSelect(int Index);
	virtual void OnSelect_Implementation(int Index);

	/**
	*	Handles navigation according to direction
	*
	*	@param	Direction  Direction of navigation
	*/
	virtual void MenuNavigate(ENavigationDirection Direction);
	/**
	*	Returns the index of the next button to navigate to
	*
	*	@param	Direction  Direction of navigation
	*/
	int FetchDirection(ENavigationDirection Direction);
	
	/**
	*	Adds this widget's parent to the viewport (if applicable)
	*	and removes this widget from viewport
	*/
	void ReturnToParent();

	/**
	*	Adds given widget to screen (strongly recomended over manual alternative)
	*
	*	@param	WidgetClass  The class of the widget to add to the screen
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		UWidget* GoToWidget(TSubclassOf<UUINavWidget> NewWidgetClass, bool bRemoveParent);


	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		virtual void MenuUp();
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		virtual void MenuDown();
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		virtual void MenuLeft();
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		virtual void MenuRight();
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		virtual void MenuSelect();
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		virtual void MenuReturn();

};