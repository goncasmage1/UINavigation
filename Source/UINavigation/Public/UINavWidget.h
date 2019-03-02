// Copyright (C) 2018 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Engine.h"
#include "Blueprint/UserWidget.h"
#include "NavigationDirection.h"
#include "UINavWidget.generated.h"

#define DISPLAYERROR(Text) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("%s"), *(FString(TEXT("Error in ")).Append(GetName()).Append(TEXT(": ")).Append(Text))));
#define SELECT_INDEX -101
#define RETURN_INDEX -202

/**
* This class contains the logic for UserWidget navigation
*/

UENUM(BlueprintType)
enum class EReceiveInputType : uint8
{
	None UMETA(DisplayName = "None"),
	Action UMETA(DisplayName = "Action"),
	Axis UMETA(DisplayName = "Axis")
};

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

UCLASS()
class UINAVIGATION_API UUINavWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	bool bCompletedSetup = false;
	bool bSetupStarted = false;

	bool bShouldTick = true;

	bool bMovingSelector = false;

	//Used to track when the selector's position should be updated
	int WaitForTick;

	//The index of the button that will be navigated to when movement is allowed
	int HaltedIndex = -1;

	int InputBoxIndex = -1;
	int NumberOfButtonsInGrids = 0;

	float MovementCounter;
	float MovementTime;

	FVector2D SelectorOrigin;
	FVector2D SelectorDestination;
	FVector2D Distance;

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
	inline void ChangeTextColorToDefault();

	/**
	*	Returns the position of the UINavButton with the specified index
	*/
	FVector2D GetButtonLocation(int Index);

	void BeginSelectorMovement(int Index);
	void HandleSelectorMovement(float DeltaTime);

public:

	bool bWaitForInput = false;

	EReceiveInputType ReceiveInputType = EReceiveInputType::None;

	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<FGrid> NavigationGrids;

	//The UserWidget object that will move along the Widget
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = UINavWidget)
		UUserWidget* TheSelector;

	//All the UINavButtons in this Widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<class UUINavButton*> UINavButtons;

	//The indices of all the UINavComponents in this widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<int> UINavComponentsIndices;

	//All the UINavComponents in this Widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<class UUINavComponent*> UINavComponents;

	//The indices of all the UINavComponentBoxes in this widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<int> ComponentBoxIndices;

	//All the UINavComponentBoxes in this Widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<class UUINavComponentBox*> UINavComponentBoxes;

	//The index of the UINavInputContainer found
	int InputContainerIndex = -1;

	//The UINavInputContainer in this Widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		class UUINavInputContainer* UINavInputContainer;

	//All the UINavInputBoxes in this Widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<class UUINavInputBox*> UINavInputBoxes;

	//All the scrollboxes in this widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<class UScrollBox*> ScrollBoxes;

	//All the scrollboxes in this widget
	UPROPERTY(BlueprintReadWrite, Category = UINavWidget)
		TArray<class UWidgetAnimation*> UINavAnimations;

	//The index of the button that was last navigated upon
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		int ButtonIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		class UUINavButton* CurrentButton = nullptr;

	//Reference to the parent widget that created this widget
	UPROPERTY(BlueprintReadOnly, meta = (ExposeOnSpawn = true), Category = UINavWidget)
		UUINavWidget* ParentWidget;

	//This widget's class
	TSubclassOf<UUINavWidget> WidgetClass;

	//Current player controller
	UPROPERTY(BlueprintReadWrite, Category = UINavWidget)
		class AUINavController* CurrentPC;

	//Widget that created this widget (if returned from a child)
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		UUINavWidget* ReturnedFromWidget;

	//Should this widget remove its parent from the viewport when created?
	bool bParentRemoved = true;


	//If set to true, buttons will be navigated by switching button states (Normal and Hovered)
	UPROPERTY(EditDefaultsOnly, Category = UINavWidget)
		bool bUseButtonStates = false;

	/*If set to true, buttons will be navigated by changing the text's color.
	Immediate child of UINavButton must be TextBlock */
	UPROPERTY(EditDefaultsOnly, Category = UINavWidget)
		bool bUseTextColor = false;

	//The index of the button to be first navigated to (when the widget is added to viewport)
	UPROPERTY(EditDefaultsOnly, Category = UINavWidget)
		int FirstButtonIndex = 0;

	//If set to true, this widget will be removed if it has no ParentWidget and is returned from
	UPROPERTY(EditDefaultsOnly, Category = UINavWidget)
		bool bAllowRemoveIfRoot = false;

	//The speed at which the given animations will play
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UINavWidget)
		float AnimationPlaybackSpeed = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Selector", meta = (EditCondition = "bUseMovementCurve"))
		UCurveFloat* MoveCurve;

	//The position the selector will be in relative to the button
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Selector")
		ESelectorPosition SelectorPositioning = ESelectorPosition::Position_Center;

	//The offset to apply when positioning the selector on a button
	UPROPERTY(EditDefaultsOnly, Category = "UINavigation Selector")
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

	virtual FReply NativeOnMouseWheel(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry & InGeometry, const FKeyEvent & InKeyEvent) override;
	virtual FReply NativeOnKeyUp(const FGeometry & InGeometry, const FKeyEvent & InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent) override;

	FReply OnKeyPressed(FKey PressedKey);
	FReply OnKeyReleased(FKey PressedKey);

	/**
	*	Appends a new navigation grid to the widget. Used for horizontal and vertical grids.
	*
	*	@param	GridType  The type of grid to be appended (horizontal or vertical)
	*	@param	Dimension  The amount of buttons in this vertical grid (If set to -1 will match number of UIUINavButtons)
	*	@param	EdgeNavigation  The intended navigation at each of the four edges of the button grid
	*	@param  bWrap  Indicates whether navigation wraps around the grid
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void AppendNavigationGrid1D(EGridType GridType, int Dimension, FButtonNavigation EdgeNavigation, bool bWrap);

	/**
	*	Appends a new navigation grid to the widget. Used for 2-dimensional grids.
	*	
	*	@param	DimensionX  The horizontal dimension of the grid
	*	@param	DimensionY  The vertical dimension of the grid
	*	@param	EdgeNavigation  The intended navigation at each of the four edges of the grid
	*	@param  bWrap  Indicates whether navigation wraps around the grid
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void AppendNavigationGrid2D(int DimensionX, int DimensionY, FButtonNavigation EdgeNavigation, bool bWrap);

	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta = (DeprecatedFunction, DeprecationMessage="This function has been replaced by Append Navigation Grid 1D."))
		void AppendHorizontalNavigation(int Dimension, FButtonNavigation EdgeNavigation, bool bWrap);

	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta = (DeprecatedFunction, DeprecationMessage="This function has been replaced by Append Navigation Grid 1D."))
		void AppendVerticalNavigation(int Dimension, FButtonNavigation EdgeNavigation, bool bWrap);

	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta = (DeprecatedFunction, DeprecationMessage="This function has been replaced by Append Navigation Grid 2D."))
		void AppendGridNavigation(int DimensionX, int DimensionY, FButtonNavigation EdgeNavigation, bool bWrap);
	/**
	*	Called manually to setup all the elements in the Widget
	*/
	virtual void UINavSetup();

	/**
	*	Called when geometry is updated after 1st tick (ready for SetupUI)
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void ReadyForSetup();
	virtual void ReadyForSetup_Implementation();

	/**
	*	Navigate to the button with the specified index
	*
	*	@param	Index  The index of the button that was hovered upon
	*	@param	bHoverEvent  Was this triggered by a button hover event?
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta = (AdvancedDisplay=1))
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
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void SetSelector(class UUserWidget* NewSelector);

	/**
	*	Changes the selector's scale to the scale given
	*
	*	@param	NewScale  The selector's new scale
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void SetSelectorScale(FVector2D NewScale);

	/**
	*	Changes the selector's visibility
	*
	*	@param	bVisible Whether the selector will be visible
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void SetSelectorVisibility(bool bVisible);

	/**
	*	Called when the button with the specified index was navigated upon
	*
	*	@param	From  The index of the button that was navigated from
	*	@param	To  The index of the button that was navigated to
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnNavigate(int From, int To);
	virtual void OnNavigate_Implementation(int From, int To);

	/**
	*	Notifies that the player navigated in the specified direction
	*
	*	@param	Direction  The direction of navigation
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnNavigatedDirection(ENavigationDirection Direction);
	virtual void OnNavigatedDirection_Implementation(ENavigationDirection Direction);

	/**
	*	Notifies that a button was selected, and indicates its index
	*
	*	@param	Index  The index of the button that was selected
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnSelect(int Index);
	virtual void OnSelect_Implementation(int Index);

	void OnPreSelect(int Index);

	/**
	*	Called when ReturnToParent is called (i.e. the player wants to exit the menu)
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnReturn();
	virtual void OnReturn_Implementation();

	/**
	*	Called when the input type changed
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnInputChanged(EInputType From, EInputType To);
	virtual void OnInputChanged_Implementation(EInputType From, EInputType To);

	/**
	*	Called before this widget is setup for UINav logic
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void PreSetup();
	virtual void PreSetup_Implementation();

	/**
	*	Called when this widget completed UINavSetup
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnSetupCompleted();
	virtual void OnSetupCompleted_Implementation();

	/**
	*	Called when the user navigates left on a UINavComponentBox
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnComponentBoxNavigateLeft(int Index);
	virtual void OnComponentBoxNavigateLeft_Implementation(int Index);

	/**
	*	Called when the user navigates right on a UINavComponentBox
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnComponentBoxNavigateRight(int Index);
	virtual void OnComponentBoxNavigateRight_Implementation(int Index);

	virtual void MenuNavigate(ENavigationDirection Direction);

	/**
	*	Returns the index of the button that will be navigated to according to the given direction, starting at the given button
	*
	*	@param	Direction  Direction of navigation
	*	@return int The index of the button that will be navigated to
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		virtual class UUINavButton* FindNextButton(class UUINavButton* Button, ENavigationDirection Direction);

	/**
	*	Returns the next button to navigate to
	*
	*	@param	Direction  Direction of navigation
	*/
	class UUINavButton* FetchButtonByDirection(ENavigationDirection Direction, UUINavButton* Button);

	/**
	*	Adds given widget to screen (strongly recomended over manual alternative)
	*
	*	@param	WidgetClass  The class of the widget to add to the screen
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta = (AdvancedDisplay=2))
		UWidget* GoToWidget(TSubclassOf<UUINavWidget> NewWidgetClass, bool bRemoveParent, int ZOrder = 0);

	/**
	*	Setup a new UINavButton added at runtime (must be added to viewport manually)
	*/
	//UFUNCTION(BlueprintCallable, Category = UINavWidget)
		//void AddUINavButton(class UUINavButton* NewButton, FGrid& TargetGrid, int IndexInGrid = -1);

	/**
	*	Setup a new UINavButton added at runtime (must be added to viewport manually)
	*/
	//UFUNCTION(BlueprintCallable, Category = UINavWidget)
		//void AddUINavComponent(class UUINavComponent* NewButton, FGrid& TargetGrid, int IndexInGrid = -1);

	/**
	*	Adds this widget's parent to the viewport (if applicable)
	*	and removes this widget from viewport
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void ReturnToParent();

	/**
	*	Returns the grid associated with the given button
	*
	*	@return ButtonGrid The button's associated grid
	*	@return IsValid Whether the returned grid is valid
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		void GetButtonGrid(class UUINavButton* Button, FGrid& ButtonGrid, bool &IsValid);

	// Returns the last button in the given grid
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		class UUINavButton* GetLastButtonInGrid(const FGrid Grid);

	// Returns the button at the specified index of the given grid
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		class UUINavButton* GetButtonAtGridIndex(const FGrid ButtonGrid, const int GridIndex);

	// Checks whether the given button is in the specified grid
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		bool IsButtonInGrid(const FGrid ButtonGrid, class UUINavButton* Button);

	// Checks whether the given button index is in the specified grid
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		bool IsButtonIndexInGrid(const FGrid ButtonGrid, const int Index);

	// Returns the given button's index inside the specified grid (-1 if the button isn't inside the grid)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		 int GetIndexInGridFromButton(const FGrid ButtonGrid, class UUINavButton* Button);

	// Returns the given button's index inside the specified grid (-1 if the button isn't inside the grid)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		int GetIndexInGridFromButtonIndex(const FGrid ButtonGrid, const int Index);

	/**
	*	Returns the UINavComponent with the specified index (null if that
	*	index doesn't correspond to a UINavComponent)
	*
	*	@return  UINavComponent  The UINavComponent with the specified index
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		UUINavComponent* GetUINavComponentAtIndex(int Index);

	/**
	*	Returns the UINavComponentBox with the specified index (null if that
	*	index doesn't correspond to a UINavComponentBox)
	*
	*	@return  UINavComponentBox  The UINavComponentBox with the specified index
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		UUINavComponentBox* GetUINavComponentBoxAtIndex(int Index);

	/**
	*	Button Hover event
	*
	*	@param	Index  The index of the button that was hovered upon
	*/
	UFUNCTION(Category = UINavWidget)
		void HoverEvent(int Index);
	/**
	*	Button UnHover event
	*
	*	@param	Index  The index of the button that was unhovered upon
	*/
	UFUNCTION(Category = UINavWidget)
		void UnhoverEvent(int Index);
	/**
	*	Button Click event
	*
	*	@param	Index  The index of the button that was clicked upon
	*/
	UFUNCTION(Category = UINavWidget)
		void ClickEvent(int Index);
	/**
	*	Button Release event
	*
	*	@param	Index  The index of the button that was released
	*/
	UFUNCTION(Category = UINavWidget)
		void ReleaseEvent(int Index);

	void SetupUINavButtonDelegates(class UUINavButton* NewButton);
	void ProcessNonMouseKeybind(FKey PressedMouseKey);
	void ProcessMouseKeybind(FKey PressedMouseKey);
	void CancelRebind();

	/**
	*	Notifies this widget to navigate in the specified direction
	*
	*	@param	Direction  The direction of navigation
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		virtual void NavigateInDirection(ENavigationDirection Direction);

	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		virtual void MenuSelect();
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		virtual void MenuReturn();

};