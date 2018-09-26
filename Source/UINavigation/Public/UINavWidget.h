// Copyright (C) 2018 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Engine.h"
#include "Blueprint/UserWidget.h"
#include "UINavWidget.generated.h"

#define DISPLAYERROR(Text) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("%s"), *(FString(TEXT("Error in ")).Append(GetName()).Append(TEXT(": ")).Append(Text))));
#define SELECT_INDEX -101
#define RETURN_INDEX -202

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
	Position_Right UMETA(DisplayName = "Right"),
	Position_Top_Right UMETA(DisplayName = "Top Right"),
	Position_Top_Left UMETA(DisplayName = "Top Left"),
	Position_Bottom_Right UMETA(DisplayName = "Bottom Right"),
	Position_Bottom_Left UMETA(DisplayName = "Bottom Left")
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

	FButtonNavigation()
	{

	}

	FButtonNavigation(int NewUp, int NewDown, int NewLeft, int NewRight)
	{
		UpButton = NewUp;
		DownButton = NewDown;
		LeftButton = NewLeft;
		RightButton = NewRight;
	}
	
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

	bool bCompletedSetup = false;

	bool bShouldTick = true;

	bool bMovingSelector = false;
	bool bAllowNavigation = true;

	bool bWaitForInput = false;
	bool bRemoveFocus = false;

	//Used to track when the selector's position should be updated
	int WaitForTick;

	//The index of the button that will be navigated to when movement is allowed
	int HaltedIndex = -1;

	int InputBoxIndex = -1;

	float MovementCounter;
	float MovementTime;

	FInputActionKeyMapping TempMapping;

	FVector2D SelectorOrigin;
	FVector2D SelectorDestination;
	FVector2D Distance;

	TArray<FKey> PressedKeys;

	/******************************************************************************/

	/**
	*	Configures the blueprint on Construct event
	*/
	void InitialSetup();

	/**
	*	Resets the necessary variables in order for this widget to be used again
	*/
	void CleanSetup();

	/**
	*	Sets up the UIUINavButtons and does error checking
	*/
	void FetchButtonsInHierarchy();

	/**
	*	Traverses this widget's hierarchy to setup all the UIUINavButtons
	*/
	void TraverseHierarquy();

	/**
	*	Configures the selector on event Construct
	*/
	void SetupSelector();

	/**
	*	Sets all the UTextBlocks to the default color
	*/
	void ChangeTextColorToDefault();

	/**
	*	Returns the position of the UINavButton with the specified index
	*/
	FVector2D GetButtonLocation(int Index);

	void BeginSelectorMovement(int Index);
	void HandleSelectorMovement(float DeltaTime);


public:

	//The UserWidget object that will move along the Widget
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UINavigation")
		UUserWidget* TheSelector;

	//Indicates the navigation possibilities of each button
	UPROPERTY(BlueprintReadWrite, Category = "UINavigation")
		TArray<FButtonNavigation> ButtonNavigations;

	//All the UINavButtons in this Widget
	UPROPERTY(BlueprintReadOnly, Category = "UINavigation")
		TArray<class UUINavButton*> UINavButtons;

	UPROPERTY(BlueprintReadOnly, Category = "UINavigation")
		TArray<int> UINavComponentsIndices;

	//All the UINavComponents in this Widget
	UPROPERTY(BlueprintReadOnly, Category = "UINavigation")
		TArray<class UUINavComponent*> UINavComponents;

	//The indices of all the UINavComponentBoxes in this widget
	UPROPERTY(BlueprintReadOnly, Category = "UINavigation")
		TArray<int> ComponentBoxIndices;

	//All the UINavComponentBoxes in this Widget
	UPROPERTY(BlueprintReadOnly, Category = "UINavigation")
		TArray<class UUINavComponentBox*> UINavComponentBoxes;

	//The index of the UINavInputContainer found
	int InputContainerIndex = -1;

	//The UINavInputContainer in this Widget
	UPROPERTY(BlueprintReadOnly, Category = "UINavigation")
		class UUINavInputContainer* UINavInputContainer;

	//The starting index of the UINavInputBoxes in this widget
	int InputBoxStartIndex = -1;
	//The starting index of the UINavInputBoxes in this widget
	int InputBoxEndIndex = -1;

	//All the UINavInputBoxes in this Widget
	UPROPERTY(BlueprintReadOnly, Category = "UINavigation")
		TArray<class UUINavInputBox*> UINavInputBoxes;

	//All the scrollboxes in this widget
	UPROPERTY(BlueprintReadOnly, Category = "UINavigation")
		TArray<class UScrollBox*> ScrollBoxes;

	//All the scrollboxes in this widget
	UPROPERTY(BlueprintReadWrite, Category = "UINavigation")
		TArray<class UWidgetAnimation*> UINavAnimations;

	//Indicates whether the Normal and Hovered style of a button were switched
	TArray<bool> bSwitchedStyle;

	//The index of the button that is currently the focus of navigation
	UPROPERTY(BlueprintReadOnly, Category = "UINavigation")
		int ButtonIndex = 0;

	//Reference to the parent widget that created this widget
	UPROPERTY(BlueprintReadOnly, meta = (ExposeOnSpawn = true), Category = "UINavigation")
		UUINavWidget* ParentWidget;

	//This widget's class
	TSubclassOf<UUINavWidget> WidgetClass;

	//Parent widget's class
	UPROPERTY(BlueprintReadOnly, Category = "UINavigation")
		TSubclassOf<UUINavWidget> ParentWidgetClass;

	//Current player controller
	UPROPERTY(BlueprintReadOnly)
		class AUINavController* CurrentPC;

	//Widget that created this widget (if returned from a child)
	UPROPERTY(BlueprintReadOnly, Category = "UINavigation")
		UUINavWidget* ReturnedFromWidget;

	//Should this widget remove its parent from the viewport when created?
	bool bParentRemoved = true;


	//If set to true, buttons will be navigated by switching button states (Normal and Hovered)
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation")
		bool bUseButtonStates = false;

	//If set to true, buttons will be navigated by changing the position of the selector
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation")
		bool bUseSelector = true;

	/*If set to true, buttons will be navigated by changing the text's color.
	Immediate child of UINavButton must be TextBlock */
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation")
		bool bUseTextColor = false;

	//The index of the button to be first navigated to (when the widget is added to viewport)
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation")
		int FirstButtonIndex = 0;

	/*If set to true, ButtonIndex will NOT be determined by the UINavButton's position in the
	hierarquy, but rather be specified in the Designer Tab.*/
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation")
		bool bOverrideButtonIndices = false;

	//If set to true, this widget will be removed if it has no ParentWidget and is returned from
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation")
		bool bAllowRemoveIfRoot = false;

	//The speed at which the given animations will play
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UINavigation")
		float AnimationPlaybackSpeed = 1.f;

	/*If set to true, this widget move the selector using a curve-guided movement animation.
	Otherwise it will snap the selector to its desired location*/
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Selector", meta = (EditCondition = "bUseSelector"))
		bool bUseMovementCurve = false;

	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Selector", meta = (EditCondition = "bUseMovementCurve"))
		UCurveFloat* MoveCurve;

	//The position the selector will be in relative to the button
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Selector", meta = (EditCondition = "bUseSelector"))
		ESelectorPosition SelectorPositioning = ESelectorPosition::Position_Center;

	//The offset to apply when positioning the selector on a button
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Selector", meta = (EditCondition = "bUseSelector"))
		FVector2D SelectorOffset;

	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Text", meta = (EditCondition = "bUseTextColor"))
		FLinearColor TextDefaultColor = FColor::Blue;

	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Text", meta = (EditCondition = "bUseTextColor"))
		FLinearColor TextNavigatedColor = FColor::Green;



	/*********************************************************************************/

	

	/**
	*	Reconfigures the blueprint if it has already been setup
	*/
	void ReconfigureSetup();

	/**
	*	The widget's construct event
	*/
	virtual void NativeConstruct() override;
	/**
	*	The widget's tick event (Used to make sure Geometry is updated)
	*/
	virtual void NativeTick(const FGeometry & MyGeometry, float DeltaTime) override;

	virtual FReply NativeOnMouseMove(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent);
	virtual FReply NativeOnKeyDown(const FGeometry & InGeometry, const FKeyEvent & InKeyEvent);
	virtual FReply NativeOnKeyUp(const FGeometry & InGeometry, const FKeyEvent & InKeyEvent);

	/**
	*	Appends a new array of FButtonNavigations to the already existing navigation graph with the given dimension
	*	Used for vertical grids.
	*
	*	@param	Dimension  The amount of buttons in this vertical grid (If set to -1 will match number of UIUINavButtons)
	*	@param	EdgeNavigation  The intended navigation at each of the four edges of the button grid
	*	@param  bWrap  Indicates whether navigation wraps around the grid
	*	@param	StartingButtonIndex The index of the button where the grid starts
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		void AppendVerticalNavigation(int Dimension, FButtonNavigation EdgeNavigation, bool bWrap);

	/**
	*	Appends a new array of FButtonNavigations to the already existing navigation graph with the given dimension
	*	Used for horizontal grids.
	*
	*	@param	Dimension  The amount of buttons in this vertical grid (If set to -1 will match number of UIUINavButtons)
	*	@param	EdgeNavigation  The intended navigation at each of the four edges of the button grid
	*	@param  bWrap  Indicates whether navigation wraps around the grid
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
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
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
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
	*	Navigate to the button with the specified index
	*
	*	@param	Index  The index of the button that was hovered upon
	*	@param	bHoverEvent  Was this triggered by a button hover event?
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		void NavigateTo(int Index, bool bHoverEvent = false);

	/**
	*	Changes the selector's location to that of the button with the given index in the Button's array
	*
	*	@param	Index  The new button's index in the Button's array
	*/
	void UpdateSelectorLocation(int Index);

	/**
	*	Changes the color of the text with the specified index to the specified color
	*
	*	@param	Index  The new button's index in the Button's array
	*	@param	Color  The text's new color
	*/
	void SwitchTextColorTo(int Index, FLinearColor Color);

	/**
	*	Changes the state of the current button to normal and the new button to hovered
	*
	*	@param	Index  The new button's index in the Button's array
	*	@param  bHovered  Whether the function was called due to a button hover
	*/
	void UpdateButtonsStates(int Index, bool bHovered);

	/**
	*	Plays the animations in the UINavAnimations array
	*
	*	@param	From  The index of the button that was navigated from
	*	@param	To  The index of the button that was navigated to
	*/
	void ExecuteAnimations(int From, int To);

	/**
	*	Changes the new text and previous text's colors to the desired colors
	*
	*	@param	Index  The new button's index in the Button's array
	*/
	void UpdateTextColor(int Index);

	/**
	*	Switches the button with the given index's style between normal and hovered
	*
	*	@param Index The button's index in the Button's array
	*/
	void SwitchButtonStyle(int Index);

	/**
	*	Sets this widget's selector
	*
	*	@param	NewSelector  The new selector
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		void SetSelector(class UUserWidget* NewSelector);

	/**
	*	Changes the selector's scale to the scale given
	*
	*	@param	NewScale  The selector's new scale
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		void SetSelectorScale(FVector2D NewScale);

	/**
	*	Changes the selector's visibility
	*
	*	@param	bVisible Whether the selector will be visible
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		void SetSelectorVisibility(bool bVisible);

	/**
	*	Called when the button with the specified index was navigated upon
	*
	*	@param	From  The index of the button that was navigated from
	*	@param	To  The index of the button that was navigated to
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "UINavigation")
		void OnNavigate(int From, int To);
	virtual void OnNavigate_Implementation(int From, int To);

	/**
	*	Notifies that a button was selected, and indicates its index
	*
	*	@param	Index  The index of the button that was selected
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "UINavigation")
		void OnSelect(int Index);
	virtual void OnSelect_Implementation(int Index);

	void OnPreSelect(int Index);

	/**
	*	Called when ReturnToParent is called (i.e. the player wants to exit the menu)
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "UINavigation")
		void OnReturn();
	virtual void OnReturn_Implementation();

	/**
	*	Called when the input type changed
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "UINavigation")
		void OnInputChanged(EInputType From, EInputType TO);

	/**
	*	Called when this widget completed UINavSetup
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "UINavigation")
		void OnSetupCompleted();

	/**
	*	Handles navigation according to direction
	*
	*	@param	Direction  Direction of navigation
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
	virtual void MenuNavigate(ENavigationDirection Direction);

	/**
	*	Returns the index of the button that will be navigated to according to the given direction
	*
	*	@param	Direction  Direction of navigation
	*	@return int The index of the button that will be navigated to
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		virtual int FindNextIndex(ENavigationDirection Direction);

	/**
	*	Returns the index of the next button to navigate to
	*
	*	@param	Direction  Direction of navigation
	*/
	int FetchIndexByDirection(ENavigationDirection Direction, int Index);

	/**
	*	Allows or forbids the player from using UINav Input
	*
	*	@param bAllow  Whether navigation should be allowed
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		void SetAllowNavigation(bool bAllow);

	/**
	*	Adds given widget to screen (strongly recomended over manual alternative)
	*
	*	@param	WidgetClass  The class of the widget to add to the screen
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		UWidget* GoToWidget(TSubclassOf<UUINavWidget> NewWidgetClass, bool bRemoveParent);
	
	/**
	*	Adds this widget's parent to the viewport (if applicable)
	*	and removes this widget from viewport
	*/
	UFUNCTION(BlueprintCallable, Category = "UINavigation")
		void ReturnToParent();

	/**
	*	Returns the UINavButton with the specified index
	*
	*	@return  UUINavButton  The UINavButton with the specified index
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINavigation")
		UUINavButton* GetUINavButtonAtIndex(int Index);

	/**
	*	Returns the UINavComponent with the specified index (null if that
	*	index doesn't correspond to a UINavComponent)
	*
	*	@return  UINavComponent  The UINavComponent with the specified index
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINavigation")
		UUINavComponent* GetUINavComponentAtIndex(int Index);

	/**
	*	Returns the UINavComponentBox with the specified index (null if that
	*	index doesn't correspond to a UINavComponentBox)
	*
	*	@return  UINavComponentBox  The UINavComponentBox with the specified index
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UINavigation")
		UUINavComponentBox* GetUINavComponentBoxAtIndex(int Index);

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

	void SetupUINavButtonDelegates(class UUINavButton* NewButton);

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