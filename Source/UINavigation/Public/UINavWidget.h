// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "Data/NavigationDirection.h"
#include "Data/ReceiveInputType.h"
#include "Data/SelectorPosition.h"
#include "Data/Grid.h"
#include "Data/NavigationEvent.h"
#include "Data/GridButton.h"
#include "Data/DynamicEdgeNavigation.h"
#include "UINavMacros.h"
#include "UINavWidget.generated.h"

#define SELECT_INDEX -101
#define RETURN_INDEX -202

enum class EButtonStyle : uint8;

/**
* This class contains the logic for UserWidget navigation
*/

UCLASS()
class UINAVIGATION_API UUINavWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	bool bCompletedSetup = false;
	bool bSetupStarted = false;

	bool bShouldTick = true;
	bool bMovingSelector = false;
	bool bIgnoreMouseEvent = false;
	bool bReturning = false;
	bool bReturningToParent = false;

	//Used to track when the selector's position should be updated
	int WaitForTick;

	//The index of the button that will be navigated to when movement is allowed
	int HaltedIndex = -1;

	//The index of the UINavInputContainer found
	int InputContainerIndex = -1;

	int CollectionIndex = 0;

	//The index of the button that was selected
	int SelectedButtonIndex = -1;
	uint8 SelectCount = 0;

	int InputBoxIndex = -1;
	int NumberOfButtonsInGrids = 0;

	float MovementCounter;
	float MovementTime;

	FVector2D SelectorOrigin;
	FVector2D SelectorDestination;
	FVector2D Distance;

	TArray<FDynamicEdgeNavigation> DynamicEdgeNavigations;

	TMap<class UWidget*, int> GridIndexMap;

	//This widget's class
	TSubclassOf<UUINavWidget> WidgetClass;

	bool bUsingSplitScreen = false;

	/******************************************************************************/

	UUINavWidget(const FObjectInitializer& ObjectInitializer);

	/**
	*	Configures the blueprint on Construct event
	*/
	void InitialSetup(bool bRebuilding = false);

	/**
	*	Resets the necessary variables in order for this widget to be used again
	*/
	void CleanSetup();

	/**
	*	Sets up the UIUINavButtons and does error checking
	*/
	void FetchButtonsInHierarchy();

	/**
	*	Configures the UINavPC
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
	void ConfigureUINavPC();

	/**
	*	Configures the selector on event Construct
	*/
	void SetupSelector();

	/**
	*	Sets all the UTextBlocks to the default color
	*/
	inline void ChangeTextColorToDefault();

	/**
	*	Rebuilds all of the widget's navigation and navigates to the button at the specified index
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void RebuildNavigation(int NewButtonIndex = -1);

	/**
	*	Returns the position of the UINavButton with the specified index
	*/
	FVector2D GetButtonLocation(int Index);

	void BeginSelectorMovement(int Index);
	void HandleSelectorMovement(float DeltaTime);

public:

	EReceiveInputType ReceiveInputType = EReceiveInputType::None;

	TSubclassOf<class UUINavPromptWidget> PromptWidgetClass;
	int PromptSelectedIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<FGrid> NavigationGrids;

	//The UserWidget object that will move along the Widget
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = UINavWidget)
		UUserWidget* TheSelector;

	//All the animation in this widget
	UPROPERTY(BlueprintReadWrite, Category = UINavWidget)
		TArray<class UWidgetAnimation*> UINavAnimations;

	//All the UINavButtons in this Widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<class UUINavButton*> UINavButtons;

	//All the UINavComponents in this Widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<class UUINavComponent*> UINavComponents;

	//All the UINavComponentBoxes in this Widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<class UUINavHorizontalComponent*> UINavHorizontalComps;

	//The UINavInputContainer in this Widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		class UUINavInputContainer* UINavInputContainer;

	//All the UINavInputBoxes in this Widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<class UUINavInputBox*> UINavInputBoxes;

	//All the UINavCollections in this widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<class UUINavCollection*> UINavCollections;

	//All the scrollboxes in this widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<class UScrollBox*> ScrollBoxes;

	//The index of the button that was last navigated upon
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		int ButtonIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		class UUINavButton* CurrentButton = nullptr;

	//Reference to the parent widget that created this widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		UUINavWidget* ParentWidget;

	//Current player controller
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		class UUINavPCComponent* UINavPC;

	//Widget that created this widget (if returned from a child)
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		UUINavWidget* ReturnedFromWidget;

	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		class UUINavWidgetComponent* WidgetComp;

	//Should this widget remove its parent from the viewport when created?
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		bool bParentRemoved = true;

	//Should this widget destroy its parent
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		bool bShouldDestroyParent = true;


	//If set to true, buttons will be navigated by switching button states (Normal and Hovered)
	UPROPERTY(EditDefaultsOnly, Category = UINavWidget)
		bool bUseButtonStates = false;

	/*If set to true, buttons will be navigated by changing the text's color.
	Immediate child of UINavButton must be TextBlock */
	UPROPERTY(EditDefaultsOnly, Category = UINavWidget)
		bool bUseTextColor = false;

	/*If set to true, the gamepad's left thumbstick will be used to move the mouse */
	UPROPERTY(EditDefaultsOnly, Category = UINavWidget)
		bool bUseLeftThumbstickAsMouse = false;

	//The index of the button to be first navigated to (when the widget is added to viewport)
	UPROPERTY(EditDefaultsOnly, Category = UINavWidget)
		int FirstButtonIndex = 0;

	//If set to true, this widget will be removed if it has no ParentWidget and is returned from
	UPROPERTY(EditDefaultsOnly, Category = UINavWidget)
		bool bAllowRemoveIfRoot = true;

	UPROPERTY(EditDefaultsOnly, Category = UINavWidget)
		bool bAnimateScrollBoxes = false;

	//UINavAnimations Playback Speed
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

	
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry & MyGeometry, float DeltaTime) override;

	virtual void RemoveFromParent() override;

	/**
	*	Traverses this widget's hierarchy to setup all the UIUINavButtons
	*/
	static void TraverseHierarquy(UUINavWidget* UINavWidget, UUserWidget* WidgetToTraverse);

	/**
	*	Reconfigures the blueprint if it has already been setup
	*/
	void ReconfigureSetup();

	virtual FReply NativeOnMouseWheel(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry & InGeometry, const FKeyEvent & InKeyEvent) override;
	virtual FReply NativeOnKeyUp(const FGeometry & InGeometry, const FKeyEvent & InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent) override;

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
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta=(AdvancedDisplay=4))
		void AppendNavigationGrid2D(int DimensionX, int DimensionY, FButtonNavigation EdgeNavigation, bool bWrap, int ButtonsInGrid = -1);

	/**
	*	Adds an edge navigation connection between 2 grids
	*	@param	Direction  The direction in which the edge navigation should go
	*   @param	bTwoWayConnection  Whether the edge connection should go both from the first grid to the target grid and vice-versa
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void AddEdgeNavigation(const int GridIndex1, const int TargetIndexInGrid1, const int GridIndex2, const int TargetIndexInGrid2, const ENavigationDirection Direction, const bool bTwoWayConnection = true);

	/**
	*	Adds edge navigation that changes through navigation between 2 grids
	*	Useful for connecting 1 vertical grid and 1 2D grid, for example
	* 
	*   @param	GridIndex  The index of the grid the connection belongs to
	*   @param	TargetGridIndex  The index of the grid to connect to
	*   @param	TargetButtonIndices  Leave empty for auto-fill. The indices of the buttons to set as the edge navigation for each index in grid
	*   @param	Event  The event that triggers the updating of the edge navigation
	*   @param	Direction  The direction in which the edge navigation should go
	*   @param	bTwoWayConnection  Whether the edge connection should go both from the first grid to the target grid and vice-versa
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta = (AutoCreateRefTerm = "ButtonIndices, TargetButtonIndices"))
		void AddSingleGridDynamicEdgeNavigation(const int GridIndex, const int TargetGridIndex, TArray<int> TargetButtonIndices, ENavigationEvent Event, const ENavigationDirection Direction, const bool bTwoWayConnection = true);

	/**
	*	Adds edge navigation that changes through navigation between 1 grid and multiple other grids
	*	Useful for a grid that changes a widget switcher index, for example
	* 
	*   @param	GridIndex  The index of the grid the connection belongs to
	*   @param	TargetButtons  The indices of the buttons to set as the edge navigation for each index in grid
	*   @param	Event  The event that triggers the updating of the edge navigation
	*   @param	Direction  The direction in which the edge navigation should go
	*   @param	bTwoWayConnection  Whether the edge connection should go both from the first grid to the target grid and vice-versa
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void AddMultiGridDynamicEdgeNavigation(const int GridIndex, TArray<FGridButton> TargetButtons, ENavigationEvent Event, const ENavigationDirection Direction, const bool bTwoWayConnection = true);

	void UpdateDynamicEdgeNavigations(const int UpdatedGridIndex);

	/**
	*	Appends a new navigation grid to the widget. Used to setup UINavCollections.
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta = (AutoCreateRefTerm = "EdgeNavigations"))
		void AppendCollection(const TArray<FButtonNavigation>& EdgeNavigations);

	/**
	*	Replaces the edge navigation of the grid at the specified index with the given edge navigation
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void SetEdgeNavigation(int GridIndex, FButtonNavigation NewEdgeNavigation);

	/**
	*	Replaces the edge navigation of the grids at the specified indices with the given edge navigation
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void SetBulkEdgeNavigation(const TArray<int>& GridIndices, FButtonNavigation NewEdgeNavigation);

	/**
	*	Replaces the edge navigation of the grid at the specified index with the non null buttons of the given edge navigation
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void SetEdgeNavigationByButton(int GridIndex, FButtonNavigation NewEdgeNavigation);

	/**
	*	Replaces the edge navigation of the grids at the specified indices with the non null buttons of the given edge navigation
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void SetBulkEdgeNavigationByButton(const TArray<int>& GridIndices, FButtonNavigation NewEdgeNavigation);

	/**
	*	Determines whether the navigation wraps around the specified grid
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void SetWrap(int GridIndex, bool bWrap);

	//Helper function to add a new 1D grid
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void Add1DGrid(EGridType GridType, UUINavButton* FirstButton, int StartingIndex, int Dimension, FButtonNavigation EdgeNavigation, bool bWrap);

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
	*	Navigate to the button with the specified index at the specified grid
	*
	*	@param	Index  The index of the button that was hovered upon
	*	@param	bHoverEvent  Was this triggered by a button hover event?
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void NavigateToGrid(int GridIndex, int IndexInGrid = 0);

	void CollectionNavigateTo(int Index);

	void CallCustomInput(FName ActionName, uint8* Buffer);

	void OnPromptDecided(TSubclassOf<class UUINavPromptWidget> PromptClass, int Index);

	void ProcessDynamicEdgeNavigation(FDynamicEdgeNavigation& DynamicEdgeNavigation);

	void UpdateEdgeNavigation(const int GridIndex, UUINavButton* TargetButton, ENavigationDirection Direction, bool bInverted);

	void DispatchNavigation(int Index, bool bHoverEvent = false);

	/**
	*	Changes the selector's location to that of the button with the given index in the Button's array
	*
	*	@param	Index  The new button's index in the Button's array
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
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
	*	@param  bHovered  Whether the function was called due to a mouse hover
	*/
	void UpdateHoveredButtonStates(int Index, bool bHovered);

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
	*	Switches the button with the given index's style
	*
	*	@param NewStyle The desired style
	*	@param Index The button's index in the Button's array
	*	@param bRevertStyle Whether to revert the button's style to normal before switching
	*/
	void SwitchButtonStyle(EButtonStyle NewStyle, int Index, bool bRevertStyle = true);

	void RevertButtonStyle(int Index);

	void SwapStyle(UUINavButton* TargetButton, EButtonStyle Style1, EButtonStyle Style2);

	void SwapPadding(UUINavButton* TargetButton);

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

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		bool IsSelectorVisible();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		FORCEINLINE bool IsRebindingInput() const { return ReceiveInputType != EReceiveInputType::None; }

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

	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnStartSelect(int Index);
	virtual void OnStartSelect_Implementation(int Index);

	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnStopSelect(int Index);
	virtual void OnStopSelect_Implementation(int Index);

	void CollectionOnSelect(int Index);
	void CollectionOnStartSelect(int Index);
	void CollectionOnStopSelect(int Index);

	void OnPreSelect(int Index, bool bMouseClick = false);

	/**
	*	Called when ReturnToParent is called (i.e. the player wants to exit the menu)
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnReturn();
	virtual void OnReturn_Implementation();

	/**
	*	Called when player navigates to the next section
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnNext();
	virtual void OnNext_Implementation();

	/**
	*	Called when player navigates to the previous section
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnPrevious();
	virtual void OnPrevious_Implementation();

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
		void PreSetup(bool bFirstSetup);
	virtual void PreSetup_Implementation(bool bFirstSetup);

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
		void OnHorizCompNavigateLeft(int Index);
	virtual void OnHorizCompNavigateLeft_Implementation(int Index);

	/**
	*	Called when the user navigates right on a UINavComponentBox
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnHorizCompNavigateRight(int Index);
	virtual void OnHorizCompNavigateRight_Implementation(int Index);

	/**
	* Called when a HorizontalComponent was updated
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnHorizCompUpdated(int Index);
	virtual void OnHorizCompUpdated_Implementation(int Index);

	virtual void MenuNavigate(ENavigationDirection Direction);

	int GetLocalComponentIndex(int Index);
	int GetLocalHorizontalCompIndex(int Index);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		bool IsSelectorValid();

	bool IsButtonIndexValid(const int InButtonIndex);

	bool IsGridIndexValid(const int GridIndex);

	FORCEINLINE uint8 GetSelectCount() const { return SelectCount; }

	/**
	*	Returns the button that will be navigated to according to the given direction, starting at the given button
	*
	*	@param	Direction  Direction of navigation
	*	@return UUINavButton* The button that will be navigated to
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
	*	@param	RemoveParent  Whether to remove the parent widget (this widget) from the viewport
	*	@param  DestroyParent  Whether to destruct the parent widget (this widget)
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta = (AdvancedDisplay=2))
		UWidget* GoToWidget(TSubclassOf<UUINavWidget> NewWidgetClass, bool bRemoveParent, bool bDestroyParent = false, int ZOrder = 0);

	/**
	*	Adds given widget to screen (strongly recomended over manual alternative)
	*
	*	@param	Widget  Object instance of the UINavWidget to add to the screen
	*	@param	RemoveParent  Whether to remove the parent widget (this widget) from the viewport
	*	@param  DestroyParent  Whether to destruct the parent widget (this widget)
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta = (AdvancedDisplay=2))
		UWidget* GoToBuiltWidget(UUINavWidget* NewWidget, bool bRemoveParent, bool bDestroyParent = false, int ZOrder = 0);

	/**
	*	Setup a new UINavButton added at runtime (must be added to viewport manually)
	*	Set IndexInGrid to -1 or to a number greater than the dimension of the grid to
	*	add the button to the end of the grid.
	*	Note: The plugin doesn't support adding buttons at runtime while navigating with animations!
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void AddUINavButton(class UUINavButton* NewButton, int TargetGridIndex, int IndexInGrid = -1);

	/**
	*	Setup several new UINavButtons added at runtime (must be added to viewport manually)
	*	Set IndexInGrid to -1 or to a number greater than the dimension of the grid to
	*	add the button to the end of the grid.
	*	Note: The plugin doesn't support adding buttons at runtime while navigating with animations!
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
        void AddUINavButtons(TArray<class UUINavButton*> NewButtons, int TargetGridIndex, int IndexInGrid = -1);

	/**
	*	Setup a new UINavComponent added at runtime (must be added to viewport manually)
	*	Set IndexInGrid to -1 or to a number greater than the dimension of the grid to
	*	add the component to the end of the grid.
	*	Note: The plugin doesn't support adding buttons at runtime while navigating with animations!
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void AddUINavComponent(class UUINavComponent* NewComponent, int TargetGridIndex, int IndexInGrid = -1);

	/**
	*	Setup several new UINavComponents added at runtime (must be added to viewport manually)
	*	Set IndexInGrid to -1 or to a number greater than the dimension of the grid to
	*	add the component to the end of the grid.
	*	Note: The plugin doesn't support adding buttons at runtime while navigating with animations!
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
        void AddUINavComponents(TArray<class UUINavComponent*> NewComponents, int TargetGridIndex, int IndexInGrid = -1);

	/**
	*	Removes the UINav element at the specified index from the widget
	*	AutoNavigate indicates whether the plugin will try to find the
	*	next button to be navigated to automatically if the deleted button
	*	is being navigated upon
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta=(AdvancedDisplay=1))
		void DeleteUINavElement(int Index, bool bAutoNavigate = true);

	/**
	*	Removes the UINav element at the index in the specified grid
	*	Set IndexInGrid to -1 or to a number greater than the dimension of the grid to
	*	delete the element at the end of the grid.
	*	AutoNavigate indicates whether the plugin will try to find the
	*	next button to be navigated to automatically if the deleted button
	*	is being navigated upon
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta=(AdvancedDisplay=2))
		void DeleteUINavElementFromGrid(int GridIndex, int IndexInGrid, bool bAutoNavigate = true);

	void IncrementGrid(class UUINavButton* NewButton, FGrid& TargetGrid, int& IndexInGrid);
	void DecrementGrid(FGrid& TargetGrid, int IndexInGrid = -1);
	void IncrementUINavButtonIndices(int StartingIndex, int GridIndex);
	void IncrementUINavComponentIndices(int StartingIndex);
	void DecrementUINavButtonIndices(int StartingIndex, int GridIndex);
	void DecrementUINavComponentIndices(int StartingIndex);

	/**
	*	Moves a UINavButton or UINavComponent to the specified grid and its index.
	*	Set IndexInGrid to -1 or to a number greater than the dimension of the grid to
	*	move the element to the end of the grid.
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void MoveUINavElementToGrid(int Index, int TargetGridIndex, int IndexInGrid = -1);

	/**
	*	Moves a UINavButton or UINavComponent to the specified grid and its index.
	*	Set IndexInGrid to -1 or to a number greater than the dimension of the grid to
	*	move the element to the end of the grid.
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void MoveUINavElementToGrid2(int FromGridIndex, int FromIndexInGrid, int TargetGridIndex, int TargetIndexInGrid = -1);

	void InsertNewComponent(class UUINavComponent* NewComponent, int ComponentIndex);
	void UpdateArrays(int From, int To, int OldGridIndex, int OldIndexInGrid);
	void UpdateButtonArray(int From, int To, int OldGridIndex, int OldIndexInGrid);
	void UpdateComponentArray(int From, int To);
	void UpdateCollectionLastIndex(int ButtonIndex, bool bAdded);
	void ReplaceButtonInNavigationGrid(class UUINavButton* ButtonToReplace, int GridIndex, int IndexInGrid);

	void UpdateCurrentButton(class UUINavButton* NewCurrentButton);

	/**
	*	Deletes the UINavElements in the grid at the specified index
	*	NOTE: You must remove the UINavButtons and UINavComponents manually
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta = (AdvancedDisplay = 1))
		void ClearGrid(int GridIndex, bool bAutoNavigate = true);

	void DeleteButtonEdgeNavigationRefs(class UUINavButton* Button);

	void DeleteGridEdgeNavigationRefs(int GridIndex);

	/**
	*	Adds this widget's parent to the viewport (if applicable)
	*	and removes this widget from viewport
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta = (AdvancedDisplay = 1))
		virtual void ReturnToParent(bool bRemoveAllParents = false, int ZOrder = 0);

	void RemoveAllParents();

	int GetWidgetHierarchyDepth(UWidget* Widget);


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		class UUINavButton* GetButtonAtIndex(int InButtonIndex);

	EButtonStyle GetStyleFromButtonState(UButton* Button);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		void GetGridAtIndex(int GridIndex, FGrid& Grid, bool& IsValid);

	// Returns the grid index of a panel widget object (Vertical Box, Horizontal Box or Uniform Grid)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		int GetGridIndexFromWidgetObject(class UWidget* Widget);

	/**
	*	Returns the grid associated with the given button
	*
	*	@return ButtonGrid The button's associated grid
	*	@return IsValid Whether the returned grid is valid
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		void GetButtonGrid(int InButtonIndex, FGrid& ButtonGrid, bool &IsValid);

	/**
	*	Returns the given button's index in its grid
	*	-1 if the index is invalid
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		int GetButtonIndexInGrid(int InButtonIndex);

	/**
	*	Returns the index of the grid associated with the given button
	*	-1 if the index is invalid
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		int GetButtonGridIndex(int InButtonIndex);

	/**
	*	Returns the index of the first button in this grid.
	*	-1 if the index is invalid for some reason.
	*
	*	@return ButtonGrid The button's associated grid
	*	@return IsValid Whether the returned grid is valid
	*/
	int GetGridStartingIndex(int GridIndex);

	/**
	*	Returns the button at the specified index of the given grid.
	*	Pass -1 for IndexInGrid for last button in grid
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		class UUINavButton* GetButtonAtGridIndex(const int GridIndex, int IndexInGrid);

	// Checks whether the given button is in the specified grid
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		bool IsButtonInGrid(const int InButtonIndex, const int GridIndex);

	// Returns the button's coordinates in a 2D grid
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		void GetButtonCoordinatesInGrid2D(const int InButtonIndex, int& XCoord, int& YCoord);

	// Returns the button in the specified coordinates of a 2D grid
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		class UUINavButton* GetButtonFromCoordinatesInGrid2D(const int GridIndex, const int XCoord, const int YCoord);

	int GetCollectionFirstButtonIndex(UUINavCollection* Collection, int Index);

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
		UUINavHorizontalComponent* GetUINavHorizontalCompAtIndex(int Index);

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
	*	Button Press event
	*
	*	@param	Index  The index of the button that was pressed
	*/
	UFUNCTION(Category = UINavWidget)
		void PressEvent(int Index);

	/**
	*	Button Release event
	*
	*	@param	Index  The index of the button that was released
	*/
	UFUNCTION(Category = UINavWidget)
		void ReleaseEvent(int Index);

	void SetupUINavButtonDelegates(class UUINavButton* NewButton);
	void ProcessKeybind(FKey PressedMouseKey);
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

	virtual void MenuSelectPress();
	virtual void MenuSelectRelease();
	virtual void MenuReturnPress();
	virtual void MenuReturnRelease();

	void FinishPress(bool bMouse);

};