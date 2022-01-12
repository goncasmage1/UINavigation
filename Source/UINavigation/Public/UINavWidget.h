// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "Data/NavigationDirection.h"
#include "Data/InputType.h"
#include "Data/ReceiveInputType.h"
#include "Data/SelectorPosition.h"
#include "Data/Grid.h"
#include "Data/NavigationEvent.h"
#include "Data/GridButton.h"
#include "Data/DynamicEdgeNavigation.h"
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

	bool bMovingSelector = false;
	bool bIgnoreMouseEvent = false;
	bool bReturning = false;
		
	bool bShouldTickUINavSetup = false;
	int UINavSetupWaitForTick=0;

	bool bShouldTickUpdateSelector = false;
	int UpdateSelectorPrevButtonIndex;
	int UpdateSelectorNextButtonIndex;
	int UpdateSelectorWaitForTick=0;

	bool bReturningToParent = false;

	bool bAutoAppended = false;
	bool bDestroying = false;
	bool bHasNavigation = false;
	bool bForcingNavigation = true;
	bool bRestoreNavigation = false;
	int HoveredButtonIndex = -1;

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

	TArray<int> UINavWidgetPath;

	//This widget's class
	TSubclassOf<UUINavWidget> WidgetClass;

	bool bUsingSplitScreen = false;

	/******************************************************************************/

	UUINavWidget(const FObjectInitializer& ObjectInitializer);

	/**
	*	Configures the blueprint on Construct event
	*/
	void InitialSetup(const bool bRebuilding = false);

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

	void SetEnableUINavButtons(const bool bEnable, const bool bRecursive);

	/**
	*	Rebuilds all of the widget's navigation and navigates to the button at the specified index
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void RebuildNavigation(const int NewButtonIndex = -1);

	/**
	*	Returns the position of the UINavButton with the specified index
	*/
	FVector2D GetButtonLocation(const int Index);

	void BeginSelectorMovement(const int PrevButtonIndex, const int NextButtonIndex);
	void HandleSelectorMovement(const float DeltaTime);

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

	//All the child UINavWidgets in this Widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		TArray<UUINavWidget*> ChildUINavWidgets;

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

	//Reference to the widget that encapsulates this widget
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		UUINavWidget* OuterUINavWidget;

	//Current player controller
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		class UUINavPCComponent* UINavPC;

	//Widget that created this widget (if returned from a child)
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		UUINavWidget* ReturnedFromWidget;

	//Nested widget that created this widget (if returned from a child)
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		UUINavWidget* PreviousNestedWidget;

	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		class UUINavWidgetComponent* WidgetComp;

	//Should this widget remove its parent from the viewport when created?
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		bool bParentRemoved = false;

	//Should this widget destroy its parent
	UPROPERTY(BlueprintReadOnly, Category = UINavWidget)
		bool bShouldDestroyParent = false;

	/*If set to true, the hovered/selected button styles will be always forced even when using the mouse,
	  otherwise, the button style won't be forced whenever the player is using the mouse.*/
	UPROPERTY(EditDefaultsOnly, Category = UINavWidget)
		bool bShouldForceNavigation = true;
	
	//If set to true, buttons will be navigated by switching button states (Normal and Hovered)
	UPROPERTY(EditDefaultsOnly, Category = UINavWidget)
		bool bUseButtonStates = false;

	/*If set to true, buttons will be navigated by changing the text's color.
	Immediate child of UINavButton must be TextBlock */
	UPROPERTY(EditDefaultsOnly, Category = UINavWidget)
		bool bUseTextColor = false;

	/*If set to true, the UINavWidget will maintain its navigated state when navigation moves to a child nested widget,
	 otherwise, the button being navigated to at that moment will be navigated out of */
	UPROPERTY(EditDefaultsOnly, Category = UINavWidget)
		bool bMaintainNavigationForChild = true;

	/*If set to true, the UINavWidget will maintain its navigated state when navigation moves to a parent nested widget,
	otherwise, the button being navigated to at that moment will be navigated out of */
	UPROPERTY(EditDefaultsOnly, Category = UINavWidget)
		bool bMaintainNavigationForParent = false;

	/*If set to true, the gamepad's left thumbstick will be used to move the mouse */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UINavWidget)
		bool bUseLeftThumbstickAsMouse = false;

	//The index of the button to be first navigated to (when the widget is added to viewport)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UINavWidget)
		int FirstButtonIndex = 0;

	//If set to true, this widget will be removed if it has no ParentWidget and is returned from
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UINavWidget)
		bool bAllowRemoveIfRoot = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UINavWidget)
		bool bAnimateScrollBoxes = false;

	//UINavAnimations Playback Speed
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UINavWidget)
		float AnimationPlaybackSpeed = 1.f;
		
    /*If set to true, the widget will be set to fullscreen even when using split screen */
    UPROPERTY(EditDefaultsOnly, Category = UINavWidget)
        bool bUseFullscreenWhenSplitscreen = false;

	// If set to true, will always use AddToPlayerScreen instead of AddToViewport, even if not in split screen
	UPROPERTY(EditDefaultsOnly, Category = UINavWidget)
		bool bForceUsePlayerScreen = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UINavigation Selector")
		UCurveFloat* MoveCurve;

	//The position the selector will be in relative to the button
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UINavigation Selector")
		ESelectorPosition SelectorPositioning = ESelectorPosition::Position_Center;

	//The offset to apply when positioning the selector on a button
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UINavigation Selector")
		FVector2D SelectorOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UINavigation Text", meta = (EditCondition = "bUseTextColor"))
		FLinearColor TextDefaultColor = FColor::Blue;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UINavigation Text", meta = (EditCondition = "bUseTextColor"))
		FLinearColor TextNavigatedColor = FColor::Green;



	/*********************************************************************************/

	
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry & MyGeometry, float DeltaTime) override;

	virtual void RemoveFromParent() override;
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

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
		void AppendNavigationGrid1D(const EGridType GridType, int Dimension, const FButtonNavigation EdgeNavigation, const bool bWrap);

	/**
	*	Appends a new navigation grid to the widget. Used for 2-dimensional grids.
	*	
	*	@param	DimensionX  The horizontal dimension of the grid
	*	@param	DimensionY  The vertical dimension of the grid
	*	@param	EdgeNavigation  The intended navigation at each of the four edges of the grid
	*	@param  bWrap  Indicates whether navigation wraps around the grid
	*	@param	ButtonsInGrid  The number of buttons in this grid
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta=(AdvancedDisplay=4))
		void AppendNavigationGrid2D(const int DimensionX, int DimensionY, const FButtonNavigation EdgeNavigation, const bool bWrap, const int ButtonsInGrid = -1);

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
		void AddSingleGridDynamicEdgeNavigation(const int GridIndex, const int TargetGridIndex, TArray<int> TargetButtonIndices, const ENavigationEvent Event, const ENavigationDirection Direction, const bool bTwoWayConnection = true);

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
		void AddMultiGridDynamicEdgeNavigation(const int GridIndex, TArray<FGridButton> TargetButtons, const ENavigationEvent Event, const ENavigationDirection Direction, const bool bTwoWayConnection = true);

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
		void SetEdgeNavigation(const int GridIndex, const FButtonNavigation NewEdgeNavigation);

	/**
	*	Replaces the edge navigation of the grids at the specified indices with the given edge navigation
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void SetBulkEdgeNavigation(const TArray<int>& GridIndices, const FButtonNavigation NewEdgeNavigation);

	/**
	*	Replaces the edge navigation of the grid at the specified index with the non null buttons of the given edge navigation
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void SetEdgeNavigationByButton(const int GridIndex, const FButtonNavigation NewEdgeNavigation);

	/**
	*	Replaces the edge navigation of the grids at the specified indices with the non null buttons of the given edge navigation
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void SetBulkEdgeNavigationByButton(const TArray<int>& GridIndices, const FButtonNavigation NewEdgeNavigation);

	/**
	*	Determines whether the navigation wraps around the specified grid
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void SetWrap(const int GridIndex, const bool bWrap);

	//Helper function to add a new 1D grid
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void Add1DGrid(const EGridType GridType, UUINavButton* FirstButton, const int StartingIndex, const int Dimension, const FButtonNavigation EdgeNavigation, const bool bWrap);

	/**
	*	Called manually to setup all the elements in the Widget
	*/
	virtual void UINavSetup();

	void AddParentToPath(const int IndexInParent);

	void PropagateGainNavigation(UUINavWidget* PreviousActiveWidget, UUINavWidget* NewActiveWidget, const UUINavWidget* const CommonParent);

	virtual void GainNavigation(UUINavWidget* PreviousActiveWidget);

	/**
	*	Called when navigation is gained
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
        void OnGainedNavigation(UUINavWidget* PreviousActiveWidget, const bool bFromChild);
	virtual void OnGainedNavigation_Implementation(UUINavWidget* PreviousActiveWidget, const bool bFromChild);

	void PropagateLoseNavigation(UUINavWidget* NewActiveWidget, UUINavWidget* PreviousActiveWidget, const UUINavWidget* const CommonParent);
	
	virtual void LoseNavigation(UUINavWidget* NewActiveWidget);

	/**
	*	Called when navigation is lost
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
        void OnLostNavigation(UUINavWidget* NewActiveWidget, const bool bToChild);
	virtual void OnLostNavigation_Implementation(UUINavWidget* NewActiveWidget, const bool bToChild);

	/**
	*	Called when geometry is updated after 1st tick (ready for SetupUI)
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void ReadyForSetup();
	virtual void ReadyForSetup_Implementation();

	/**
	*	Navigate to the button with the specified index
	*
	*	@param	Index  The index of the button to be navigated to
	*	@param	bHoverEvent  Was this triggered by a button hover event?
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta = (AdvancedDisplay=1))
		void NavigateTo(const int Index, const bool bHoverEvent = false, const bool bBypassChecks = false);

	/**
	*	Navigate to the button with the specified index at the specified grid
	*
	*	@param	GridIndex  The index of the grid to be navigated to
	*	@param	IndexInGrid  The index in the grid to be navigated to
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void NavigateToGrid(const int GridIndex, const int IndexInGrid = 0);

	void CollectionNavigateTo(const int Index);

	void CallCustomInput(const FName ActionName, uint8* Buffer);

	void OnPromptDecided(const TSubclassOf<class UUINavPromptWidget> PromptClass, const int Index);

	void ProcessDynamicEdgeNavigation(FDynamicEdgeNavigation& DynamicEdgeNavigation);

	void UpdateEdgeNavigation(const int GridIndex, UUINavButton* TargetButton, const ENavigationDirection Direction, const bool bInverted);

	void DispatchNavigation(const int Index, const bool bBypassForcedNavigation = false);

	/**
	*	Changes the selector's location to that of the button with the given index in the Button's array
	*
	*	@param	Index  The new button's index in the Button's array
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void UpdateSelectorLocation(const int Index);

	/**
	*	Changes the color of the text with the specified index to the specified color
	*
	*	@param	Index  The new button's index in the Button's array
	*	@param	Color  The text's new color
	*/
	void SwitchTextColorTo(const int Index, FLinearColor Color);

	/**
	*	Changes the state of the current button to normal and the new button to hovered
	*
	*	@param	Index  The new button's index in the Button's array
	*/
	void UpdateHoveredButtonStates(const int Index);

	/**
	*	Plays the animations in the UINavAnimations array
	*
	*	@param	From  The index of the button that was navigated from
	*	@param	To  The index of the button that was navigated to
	*/
	void ExecuteAnimations(const int From, const int To);

	/**
	*	Changes the new text and previous text's colors to the desired colors
	*
	*	@param	Index  The new button's index in the Button's array
	*/
	void UpdateTextColor(const int Index);

	/**
	*	Switches the button with the given index's style
	*
	*	@param NewStyle The desired style
	*	@param Index The button's index in the Button's array
	*	@param bRevertStyle Whether to revert the button's style to normal before switching
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
	void SwitchButtonStyle(const EButtonStyle NewStyle, const int Index, const bool bRevertStyle = true);

	void RevertButtonStyle(const int Index);

	static void SwapStyle(UUINavButton* TargetButton, EButtonStyle Style1, EButtonStyle Style2);

	static void SwapPadding(UUINavButton* TargetButton);

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
		void SetSelectorVisibility(const bool bVisible);

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
		void OnNavigate(const int From, const int To);
	virtual void OnNavigate_Implementation(const int From, const int To);

	/**
	*	Notifies that the player navigated in the specified direction
	*
	*	@param	Direction  The direction of navigation
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnNavigatedDirection(const ENavigationDirection Direction);
	virtual void OnNavigatedDirection_Implementation(const ENavigationDirection Direction);

	/**
	*	Notifies that a button was selected, and indicates its index
	*
	*	@param	Index  The index of the button that was selected
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnSelect(const int Index);
	virtual void OnSelect_Implementation(const int Index);

	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnStartSelect(const int Index);
	virtual void OnStartSelect_Implementation(const int Index);

	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnStopSelect(const int Index);
	virtual void OnStopSelect_Implementation(const int Index);

	void CollectionOnSelect(const int Index);
	void CollectionOnStartSelect(const int Index);
	void CollectionOnStopSelect(const int Index);

	void OnPreSelect(const int Index, const bool bMouseClick = false);

	void AttemptUnforceNavigation(const EInputType NewInputType);

	/**
	*	Called when ReturnToParent is called (i.e. the player wants to exit the menu)
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnReturn();
	virtual void OnReturn_Implementation();

	void CollectionOnReturn();

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
		void OnInputChanged(const EInputType From, const EInputType To);
	virtual void OnInputChanged_Implementation(const EInputType From, const EInputType To);

	/**
	*	Called before this widget is setup for UINav logic
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void PreSetup(const bool bFirstSetup);
	virtual void PreSetup_Implementation(const bool bFirstSetup);

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
		void OnHorizCompNavigateLeft(const int Index);
	virtual void OnHorizCompNavigateLeft_Implementation(const int Index);

	/**
	*	Called when the user navigates right on a UINavComponentBox
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnHorizCompNavigateRight(const int Index);
	virtual void OnHorizCompNavigateRight_Implementation(const int Index);

	/**
	* Called when a HorizontalComponent was updated
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
		void OnHorizCompUpdated(const int Index);
	virtual void OnHorizCompUpdated_Implementation(const int Index);

	virtual void MenuNavigate(const ENavigationDirection Direction);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
	UUINavWidget* GetMostOuterUINavWidget();

	UUINavWidget* GetChildUINavWidget(const int ChildIndex);
	
	FORCEINLINE TArray<int> GetUINavWidgetPath() const { return UINavWidgetPath; }


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		bool IsSelectorValid();

	FORCEINLINE uint8 GetSelectCount() const { return SelectCount; }

	/**
	*	Returns the button that will be navigated to according to the given direction, starting at the given button
	*
	*	@param	Direction  Direction of navigation
	*	@return UUINavButton* The button that will be navigated to
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		virtual class UUINavButton* FindNextButton(class UUINavButton* Button, const ENavigationDirection Direction);

	/**
	*	Returns the next button to navigate to
	*
	*	@param	Direction  Direction of navigation
	*/
	class UUINavButton* FetchButtonByDirection(const ENavigationDirection Direction, UUINavButton* Button);

	/**
	*	Adds given widget to screen (strongly recomended over manual alternative)
	*
	*	@param	NewWidgetClass  The class of the widget to add to the screen
	*	@param	bRemoveParent  Whether to remove the parent widget (this widget) from the viewport
	*	@param  bDestroyParent  Whether to destruct the parent widget (this widget)
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta = (AdvancedDisplay=2))
		UUINavWidget* GoToWidget(TSubclassOf<UUINavWidget> NewWidgetClass, const bool bRemoveParent, const bool bDestroyParent = false, const int ZOrder = 0);

	/**
	*	Adds given widget to screen (strongly recomended over manual alternative)
	*
	*	@param	NewWidget  Object instance of the UINavWidget to add to the screen
	*	@param	bRemoveParent  Whether to remove the parent widget (this widget) from the viewport
	*	@param  bDestroyParent  Whether to destruct the parent widget (this widget)
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta = (AdvancedDisplay=2))
		UUINavWidget* GoToBuiltWidget(UUINavWidget* NewWidget, const bool bRemoveParent, const bool bDestroyParent = false, const int ZOrder = 0);

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
		void DeleteUINavElement(const int Index, const bool bAutoNavigate = true);

	/**
	*	Removes the UINav element at the index in the specified grid
	*	Set IndexInGrid to -1 or to a number greater than the dimension of the grid to
	*	delete the element at the end of the grid.
	*	AutoNavigate indicates whether the plugin will try to find the
	*	next button to be navigated to automatically if the deleted button
	*	is being navigated upon
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta=(AdvancedDisplay=2))
		void DeleteUINavElementFromGrid(const int GridIndex, int IndexInGrid, const bool bAutoNavigate = true);

	void IncrementGrid(class UUINavButton* NewButton, FGrid& TargetGrid, int& IndexInGrid);
	void DecrementGrid(FGrid& TargetGrid, const int IndexInGrid = -1);
	void IncrementUINavButtonIndices(const int StartingIndex, const int GridIndex);
	void DecrementUINavButtonIndices(const int StartingIndex, const int GridIndex);

	/**
	*	Moves a UINavButton or UINavComponent to the specified grid and its index.
	*	Set IndexInGrid to -1 or to a number greater than the dimension of the grid to
	*	move the element to the end of the grid.
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void MoveUINavElementToGrid(const int Index, const int TargetGridIndex, int IndexInGrid = -1);

	/**
	*	Moves a UINavButton or UINavComponent to the specified grid and its index.
	*	Set IndexInGrid to -1 or to a number greater than the dimension of the grid to
	*	move the element to the end of the grid.
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		void MoveUINavElementToGrid2(const int FromGridIndex, const int FromIndexInGrid, const int TargetGridIndex, const int TargetIndexInGrid = -1);

	void UpdateArrays(const int From, const int To, const int OldGridIndex, const int OldIndexInGrid);
	void UpdateButtonArray(const int From, int To, const int OldGridIndex, const int OldIndexInGrid);
	void UpdateCollectionLastIndex(const int ButtonIndex, const bool bAdded);
	void ReplaceButtonInNavigationGrid(class UUINavButton* ButtonToReplace, const int GridIndex, const int IndexInGrid);

	void UpdateCurrentButton(class UUINavButton* NewCurrentButton);

	/**
	*	Deletes the UINavElements in the grid at the specified index
	*	NOTE: You must remove the UINavButtons and UINavComponents manually
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta = (AdvancedDisplay = 1))
		void ClearGrid(const int GridIndex, const bool bAutoNavigate = true);

	void DeleteButtonEdgeNavigationRefs(class UUINavButton* Button);

	void DeleteGridEdgeNavigationRefs(const int GridIndex);

	/**
	*	Adds this widget's parent to the viewport (if applicable)
	*	and removes this widget from viewport
	*/
	UFUNCTION(BlueprintCallable, Category = UINavWidget, meta = (AdvancedDisplay = 1))
		virtual void ReturnToParent(const bool bRemoveAllParents = false, const int ZOrder = 0);

	void RemoveAllParents();

	int GetWidgetHierarchyDepth(UWidget* Widget) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		class UUINavButton* GetButtonAtIndex(const int InButtonIndex);

	static EButtonStyle GetStyleFromButtonState(UButton* Button);
	
	FORCEINLINE bool HasNavigation() const { return bHasNavigation; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		void GetGridAtIndex(const int GridIndex, FGrid& Grid, bool& IsValid);

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
		void GetButtonGrid(const int InButtonIndex, FGrid& ButtonGrid, bool &IsValid);

	/**
	*	Returns the given button's index in its grid
	*	-1 if the index is invalid
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		int GetButtonIndexInGrid(const int InButtonIndex);

	/**
	*	Returns the index of the grid associated with the given button
	*	-1 if the index is invalid
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		int GetButtonGridIndex(const int InButtonIndex);

	/**
	*	Returns the index of the first button in this grid.
	*	-1 if the index is invalid for some reason.
	*
	*	@return ButtonGrid The button's associated grid
	*	@return IsValid Whether the returned grid is valid
	*/
	int GetGridStartingIndex(const int GridIndex);

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

	static int GetCollectionFirstButtonIndex(UUINavCollection* Collection, const int Index);

	/**
	*	Returns the UINavComponent with the specified index (null if that
	*	index doesn't correspond to a UINavComponent)
	*
	*	@return  UINavComponent  The UINavComponent with the specified index
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		UUINavComponent* GetUINavComponentAtIndex(const int Index);

	/**
	*	Returns the UINavComponentBox with the specified index (null if that
	*	index doesn't correspond to a UINavComponentBox)
	*
	*	@return  UINavComponentBox  The UINavComponentBox with the specified index
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UINavWidget)
		UUINavHorizontalComponent* GetUINavHorizontalCompAtIndex(const int Index);

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
		virtual void NavigateInDirection(const ENavigationDirection Direction);

	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		virtual void MenuSelect();
	UFUNCTION(BlueprintCallable, Category = UINavWidget)
		virtual void MenuReturn();

	virtual void MenuSelectPress();
	virtual void MenuSelectRelease();
	virtual void MenuReturnPress();
	virtual void MenuReturnRelease();

	void FinishPress(const bool bMouse);

};