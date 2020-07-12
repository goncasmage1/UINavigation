// Copyright (C) 2019 Gon�alo Marques - All Rights Reserved


#include "UINavPCComponent.h"
#include "UINavWidget.h"
#include "UINavSettings.h"
#include "UINavPCReceiver.h"
#include "Data/AxisType.h"
#include "Data/InputIconMapping.h"
#include "Data/InputNameMapping.h"
#include "UINavBlueprintFunctionLibrary.h"

UUINavPCComponent::UUINavPCComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = true;

	bAutoActivate = true;
	bCanEverAffectNavigation = false;
}

void UUINavPCComponent::Activate(bool bReset)
{
	PC = Cast<APlayerController>(GetOwner());
	if (PC == nullptr)
	{
		DISPLAYERROR(TEXT("UINavPCComponent Owner isn't a Player Controller!"));
		return;
	}

	if (!PC->GetClass()->ImplementsInterface(UUINavPCReceiver::StaticClass()))
	{
		DISPLAYERROR(TEXT("Player Controller doesn't implement UINavPCReceiver interface!"));
		return;
	}
}

void UUINavPCComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PC != nullptr && PC->IsLocalPlayerController())
	{
		VerifyDefaultInputs();
		FetchUINavActionKeys();
	}
}

void UUINavPCComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (CountdownPhase)
	{
		case ECountdownPhase::First:
			TimerCounter += DeltaTime;
			if (TimerCounter >= InputHeldWaitTime)
			{
				TimerCallback();
				TimerCounter -= InputHeldWaitTime;
				CountdownPhase = ECountdownPhase::Looping;
			}
			break;
		case ECountdownPhase::Looping:
			TimerCounter += DeltaTime;
			if (TimerCounter >= NavigationChainFrequency)
			{
				TimerCallback();
				TimerCounter -= NavigationChainFrequency;
			}
			break;
	}

	float PosX, PosY;
	PC->GetMousePosition(PosX, PosY);
	if (CurrentInputType != EInputType::Mouse)
	{
		if ((PosX != 0.f && PosX != PreviousX) || (PosY != 0.f && PosY != PreviousY))
		{
			NotifyMouseInputType();
		}
	}
	PreviousX = PosX;
	PreviousY = PosY;
}

void UUINavPCComponent::BindMenuInputs()
{
	UInputComponent* InputComponent = PC->InputComponent;
	if (InputComponent == nullptr) return;

	FInputActionBinding& Action1_1 = InputComponent->BindAction("MenuUp", IE_Pressed, this, &UUINavPCComponent::StartMenuUp);
	Action1_1.bExecuteWhenPaused = true;
	Action1_1.bConsumeInput = false;
	FInputActionBinding& Action2_1 = InputComponent->BindAction("MenuDown", IE_Pressed, this, &UUINavPCComponent::StartMenuDown);
	Action2_1.bExecuteWhenPaused = true;
	Action2_1.bConsumeInput = false;
	FInputActionBinding& Action3_1 = InputComponent->BindAction("MenuLeft", IE_Pressed, this, &UUINavPCComponent::StartMenuLeft);
	Action3_1.bExecuteWhenPaused = true;
	Action3_1.bConsumeInput = false;
	FInputActionBinding& Action4_1 = InputComponent->BindAction("MenuRight", IE_Pressed, this, &UUINavPCComponent::StartMenuRight);
	Action4_1.bExecuteWhenPaused = true;
	Action4_1.bConsumeInput = false;
	FInputActionBinding& Action5_1 = InputComponent->BindAction("MenuSelect", IE_Pressed, this, &UUINavPCComponent::MenuSelect);
	Action5_1.bExecuteWhenPaused = true;
	Action5_1.bConsumeInput = false;
	FInputActionBinding& Action6_1 = InputComponent->BindAction("MenuReturn", IE_Pressed, this, &UUINavPCComponent::MenuReturn);
	Action6_1.bExecuteWhenPaused = true;
	Action6_1.bConsumeInput = false;
	FInputActionBinding& Action7_1 = InputComponent->BindAction("MenuNext", IE_Pressed, this, &UUINavPCComponent::MenuNext);
	Action7_1.bExecuteWhenPaused = true;
	Action7_1.bConsumeInput = false;
	FInputActionBinding& Action8_1 = InputComponent->BindAction("MenuPrevious", IE_Pressed, this, &UUINavPCComponent::MenuPrevious);
	Action8_1.bExecuteWhenPaused = true;
	Action8_1.bConsumeInput = false;

	FInputActionBinding& Action1_2 = InputComponent->BindAction("MenuUp", IE_Released, this, &UUINavPCComponent::MenuUpRelease);
	Action1_2.bExecuteWhenPaused = true;
	Action1_2.bConsumeInput = false;
	FInputActionBinding& Action2_2 = InputComponent->BindAction("MenuDown", IE_Released, this, &UUINavPCComponent::MenuDownRelease);
	Action2_2.bExecuteWhenPaused = true;
	Action2_2.bConsumeInput = false;
	FInputActionBinding& Action3_2 = InputComponent->BindAction("MenuLeft", IE_Released, this, &UUINavPCComponent::MenuLeftRelease);
	Action3_2.bExecuteWhenPaused = true;
	Action3_2.bConsumeInput = false;
	FInputActionBinding& Action4_2 = InputComponent->BindAction("MenuRight", IE_Released, this, &UUINavPCComponent::MenuRightRelease);
	Action4_2.bExecuteWhenPaused = true;
	Action4_2.bConsumeInput = false;
	FInputActionBinding& Action5_2 = InputComponent->BindAction("MenuSelect", IE_Released, this, &UUINavPCComponent::MenuSelectRelease);
	Action5_1.bExecuteWhenPaused = true;
	Action5_1.bConsumeInput = false;
	FInputActionBinding& Action6_2 = InputComponent->BindAction("MenuReturn", IE_Released, this, &UUINavPCComponent::MenuReturnRelease);
	Action6_1.bExecuteWhenPaused = true;
	Action6_1.bConsumeInput = false;

	if (CustomInputs.Num() > 0)
	{
		bAllowCustomInputs.Empty();
		bAllowCustomInputs.Init(true, CustomInputs.Num());

		int i = 0;
		for (const FName CustomInput : CustomInputs)
		{
			FInputActionHandlerSignature PressedActionHandler;
			PressedActionHandler.BindUFunction(this, TEXT("OnCustomInput"), i, true);
			FInputActionBinding PressedActionBinding(CustomInput, IE_Pressed);
			PressedActionBinding.ActionDelegate = PressedActionHandler;
			FInputActionBinding& CustomPressedInput = InputComponent->AddActionBinding(PressedActionBinding);
			CustomPressedInput.bExecuteWhenPaused = true;
			CustomPressedInput.bConsumeInput = false;

			FInputActionHandlerSignature ReleasedActionHandler;
			ReleasedActionHandler.BindUFunction(this, TEXT("OnCustomInput"), i, false);
			FInputActionBinding ReleasedActionBinding(CustomInput, IE_Released);
			ReleasedActionBinding.ActionDelegate = ReleasedActionHandler;
			FInputActionBinding& CustomReleasedInput = InputComponent->AddActionBinding(ReleasedActionBinding);
			CustomReleasedInput.bExecuteWhenPaused = true;
			CustomReleasedInput.bConsumeInput = false;

			i++;
		}
	}

}

void UUINavPCComponent::UnbindMenuInputs()
{
	UInputComponent* InputComponent = PC->InputComponent;
	if (InputComponent == nullptr) return;

	int NumActionBindings = InputComponent->GetNumActionBindings();
	for (int i = NumActionBindings - 1; i >= 0; i--)
	{
		if (InputComponent->GetActionBinding(i).ActionDelegate.IsBoundToObject(this))
		{
			InputComponent->RemoveActionBinding(i);
		}
	}
}

void UUINavPCComponent::OnCustomInput(int InputIndex, bool bPressed)
{
	CallCustomInput(CustomInputs[InputIndex], bPressed);
}

void UUINavPCComponent::CallCustomInput(FName ActionName, bool bPressed)
{
	if (ActiveWidget != nullptr && AllowsCustomInputByName(ActionName))
	{
		int CustomInputIndex = CustomInputs.Find(ActionName);
		if (CustomInputIndex < 0) return;

		uint8* Buffer = (uint8*)FMemory_Alloca(sizeof(bool));
		FMemory::Memcpy(Buffer, &bPressed, sizeof(bool));

		ActiveWidget->CallCustomInput(ActionName, Buffer);
	}
}

void UUINavPCComponent::VerifyDefaultInputs()
{
	UUINavSettings *MySettings = GetMutableDefault<UUINavSettings>();
	if (MySettings->ActionMappings.Num() == 0 && MySettings->AxisMappings.Num() == 0)
	{
		UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
		MySettings->ActionMappings = Settings->GetActionMappings();
		MySettings->AxisMappings = Settings->GetAxisMappings();
		MySettings->SaveConfig();
	}
}

void UUINavPCComponent::BindMouseWorkaround()
{
	UInputComponent* InputComponent = PC->InputComponent;
	if (InputComponent == nullptr) return;

	TArray<FKey> MouseKeys =
	{
		EKeys::LeftMouseButton,
		EKeys::RightMouseButton,
		EKeys::MiddleMouseButton,
		EKeys::MouseScrollUp,
		EKeys::MouseScrollDown,
		EKeys::ThumbMouseButton,
		EKeys::ThumbMouseButton2
	};

	for (FKey MouseKey : MouseKeys)
	{
		FInputKeyBinding NewKey;
		NewKey.KeyDelegate.BindDelegate<FMouseKeyDelegate>(this, &UUINavPCComponent::MouseKeyPressed, EKeys::AnyKey);
		NewKey.bConsumeInput = true;
		NewKey.bExecuteWhenPaused = true;
		InputComponent->KeyBindings.Add(NewKey);
	}
}

void UUINavPCComponent::UnbindMouseWorkaround()
{
	UInputComponent* InputComponent = PC->InputComponent;
	if (InputComponent == nullptr) return;

	int KeyBindingsNum = InputComponent->KeyBindings.Num();
	for (int i = KeyBindingsNum - 1; i >= 0; i--)
	{
		if (InputComponent->KeyBindings[i].KeyDelegate.IsBoundToObject(this))
		{
			InputComponent->KeyBindings.RemoveAt(i);
		}
	}
}

void UUINavPCComponent::SetActiveWidget(UUINavWidget * NewActiveWidget)
{
	if (ActiveWidget != nullptr && NewActiveWidget == nullptr)
	{
		UnbindMenuInputs();
		PressedActions.Empty();
	}
	else if (ActiveWidget == nullptr && NewActiveWidget != nullptr)
	{
		BindMenuInputs();
	}
	ActiveWidget = NewActiveWidget;
}

void UUINavPCComponent::SetAllowAllMenuInput(bool bAllowInput)
{
	bAllowDirectionalInput = bAllowInput;
	bAllowSelectInput = bAllowInput;
	bAllowReturnInput = bAllowInput;
	bAllowSectionInput = bAllowInput;
	for (bool& allowCustomInput : bAllowCustomInputs)
	{
		allowCustomInput = bAllowInput;
	}
}

void UUINavPCComponent::SetAllowDirectionalInput(bool bAllowInput)
{
	bAllowDirectionalInput = bAllowInput;
}

void UUINavPCComponent::SetAllowSelectInput(bool bAllowInput)
{
	bAllowSelectInput = bAllowInput;
}

void UUINavPCComponent::SetAllowReturnInput(bool bAllowInput)
{
	bAllowReturnInput = bAllowInput;
}

void UUINavPCComponent::SetAllowSectionInput(bool bAllowInput)
{
	bAllowSectionInput = bAllowInput;
}

void UUINavPCComponent::SetAllowCustomInputByName(FName InputName, bool bAllowInput)
{
	int CustomInputIndex = CustomInputs.Find(InputName);
	if (CustomInputIndex < 0) return;
	bAllowCustomInputs[CustomInputIndex] = bAllowInput;
}

void UUINavPCComponent::SetAllowCustomInputByIndex(int InputIndex, bool bAllowInput)
{
	if (InputIndex < 0 || InputIndex >= CustomInputs.Num()) return;
	bAllowCustomInputs[InputIndex] = bAllowInput;
}

void UUINavPCComponent::TimerCallback()
{
	MenuInput(CallbackDirection);
}

void UUINavPCComponent::SetTimer(ENavigationDirection TimerDirection)
{
	TimerCounter = 0.f;
	CallbackDirection = TimerDirection;
	CountdownPhase = ECountdownPhase::First;
}

void UUINavPCComponent::ClearTimer()
{
	if (CallbackDirection == ENavigationDirection::None) return;

	TimerCounter = 0.f;
	CallbackDirection = ENavigationDirection::None;
	CountdownPhase = ECountdownPhase::None;
}

void UUINavPCComponent::FetchUINavActionKeys()
{
	const UInputSettings* Settings = GetDefault<UInputSettings>();
	const TArray<FInputActionKeyMapping> Actions = Settings->GetActionMappings();

	const TArray<FString> MenuInputs = {
		TEXT("MenuUp"),
		TEXT("MenuDown"),
		TEXT("MenuLeft"),
		TEXT("MenuRight"),
		TEXT("MenuSelect"),
		TEXT("MenuReturn"),
		TEXT("MenuNext"),
		TEXT("MenuPrevious")
	};

	for (FInputActionKeyMapping Action : Actions)
	{
		FString NewName = Action.ActionName.ToString();
		if (NewName.Left(4).Compare(TEXT("Menu")) != 0)
			continue;

		TArray<FKey>* KeyArray = KeyMap.Find(NewName);
		if (KeyArray == nullptr)
		{
			KeyMap.Add(NewName, { Action.Key });
		}
		else
		{
			KeyArray->Add(Action.Key);
		}
	}

	TArray<FString> keys;
	KeyMap.GetKeys(keys);
	if (keys.Num() < 6)
	{
		DISPLAYERROR("Not all Menu Inputs have been setup!");
	}
	else if (keys.Num() < 8)
	{
		DISPLAYWARNING("You can add them from the UINavInput.ini file in the plugin's Content folder to your project's DefaultInput.ini file.");
		DISPLAYWARNING("Keep in mind that the MenuNext and MenuPrevious inputs have been added recently.");
		DISPLAYWARNING("Not all Menu Inputs have been setup!");
	}
}

FKey UUINavPCComponent::GetInputKey(FName InputName, EInputRestriction InputRestriction)
{
	FString InputString = InputName.ToString();
	FString AxisScale = InputString.Right(1);
	EAxisType AxisType = EAxisType::None;
	if (AxisScale.Equals(TEXT("+"))) AxisType = EAxisType::Positive;
	else if (AxisScale.Equals(TEXT("-"))) AxisType = EAxisType::Negative;
	
	const UInputSettings* Settings = GetDefault<UInputSettings>();

	if (AxisType == EAxisType::None)
	{
		TArray<FInputActionKeyMapping> ActionMappings;
		Settings->GetActionMappingByName(InputName, ActionMappings);

		int Iterations = ActionMappings.Num();
		for (int i = Iterations - 1; i >= 0; --i)
		{
			if (UUINavBlueprintFunctionLibrary::RespectsRestriction(ActionMappings[i].Key, InputRestriction))
				return ActionMappings[i].Key;
		}
		return FKey();
	}
	else
	{
		InputName = FName(*InputString.Left(InputString.Len() - 1));
		TArray<FInputAxisKeyMapping> AxisMappings;
		Settings->GetAxisMappingByName(InputName, AxisMappings);

		FKey PotentialAxisKey;
		int Iterations = AxisMappings.Num();
		for (int i = Iterations - 1; i >= 0; --i)
		{
			if (UUINavBlueprintFunctionLibrary::RespectsRestriction(AxisMappings[i].Key, InputRestriction))
			{
				if ((AxisMappings[i].Scale > 0.0f && AxisType == EAxisType::Positive) ||
					(AxisMappings[i].Scale < 0.0f && AxisType == EAxisType::Negative))
				{
					return AxisMappings[i].Key;
				}
				else
				{
					PotentialAxisKey = GetKeyFromAxis(AxisMappings[i].Key, AxisType == EAxisType::Positive);
					if (PotentialAxisKey.IsValid())
					{
						return PotentialAxisKey;
					}
				}
			}
		}
		return FKey();
	}
}

UTexture2D * UUINavPCComponent::GetKeyIcon(FKey Key)
{
	FInputIconMapping* KeyIcon = nullptr;

	if (Key.IsGamepadKey())
	{
		if (GamepadKeyIconData != nullptr && GamepadKeyIconData->GetRowMap().Contains(Key.GetFName()))
		{
			KeyIcon = (FInputIconMapping*)GamepadKeyIconData->GetRowMap()[Key.GetFName()];
		}
	}
	else
	{
		if (KeyboardMouseKeyIconData != nullptr && KeyboardMouseKeyIconData->GetRowMap().Contains(Key.GetFName()))
		{
			KeyIcon = (FInputIconMapping*)KeyboardMouseKeyIconData->GetRowMap()[Key.GetFName()];
		}
	}

	if (KeyIcon == nullptr) return nullptr;

	UTexture2D* NewTexture = KeyIcon->InputIcon.LoadSynchronous();
	return NewTexture;
}

UTexture2D * UUINavPCComponent::GetInputIcon(FName ActionName, EInputRestriction InputRestriction)
{
	FKey Key = GetInputKey(ActionName, InputRestriction);
	return GetKeyIcon(Key);
}

FText UUINavPCComponent::GetKeyText(FKey Key)
{
	if (!Key.IsValid()) return FText();

	FInputNameMapping* Keyname = nullptr;

	if (Key.IsGamepadKey())
	{
		if (GamepadKeyNameData != nullptr && GamepadKeyNameData->GetRowMap().Contains(Key.GetFName()))
		{
			Keyname = (FInputNameMapping*)GamepadKeyNameData->GetRowMap()[Key.GetFName()];
		}
	}
	else
	{
		if (KeyboardMouseKeyNameData != nullptr && KeyboardMouseKeyNameData->GetRowMap().Contains(Key.GetFName()))
		{
			Keyname = (FInputNameMapping*)KeyboardMouseKeyNameData->GetRowMap()[Key.GetFName()];
		}
	}

	if (Keyname == nullptr) return Key.GetDisplayName();

	return Keyname->InputText;
}

void UUINavPCComponent::GetInputRebindData(FName InputName, FInputRebindData& OutData, bool& bSuccess)
{
	FInputRebindData* InputRebindData;
	if (InputRebindDataTable != nullptr && InputRebindDataTable->GetRowMap().Contains(InputName))
	{
		InputRebindData = (FInputRebindData*)InputRebindDataTable->GetRowMap()[InputName];
		if (InputRebindData != nullptr)
		{
			OutData = *InputRebindData;
			bSuccess = true;
			return;
		}
	}
	bSuccess = false;
}

FText UUINavPCComponent::GetInputText(FName InputName)
{
	FInputRebindData InputRebindData;
	bool bSuccess = false;
	GetInputRebindData(InputName, InputRebindData, bSuccess);

	return bSuccess ? InputRebindData.InputText : FText();
}

TArray<FKey> UUINavPCComponent::GetInputKeysFromName(FName InputName)
{
	TArray<FKey> KeyArray = TArray<FKey>();

	const UInputSettings* Settings = GetDefault<UInputSettings>();

	TArray<FInputActionKeyMapping> ActionMappings;
	Settings->GetActionMappingByName(InputName, ActionMappings);

	if (ActionMappings.Num() > 0)
	{
		for (FInputActionKeyMapping mapping : ActionMappings)
		{
			KeyArray.Add(mapping.Key);
		}
	}
	else
	{
		TArray<FInputAxisKeyMapping> AxisMappings;
		Settings->GetAxisMappingByName(InputName, AxisMappings);
		if (AxisMappings.Num() > 0)
		{
			for (FInputAxisKeyMapping mapping : AxisMappings)
			{
				KeyArray.Add(mapping.Key);
			}
		}
	}
	return KeyArray;
}

void UUINavPCComponent::GetInputKeys(FName InputName, TArray<FKey>& OutKeys)
{
	OutKeys.Empty();
	FString InputString = InputName.ToString();
	FString AxisScale = InputString.Right(1);
	EAxisType AxisType = EAxisType::None;
	if (AxisScale.Equals(TEXT("+"))) AxisType = EAxisType::Positive;
	else if (AxisScale.Equals(TEXT("-"))) AxisType = EAxisType::Negative;

	const UInputSettings* Settings = GetDefault<UInputSettings>();

	if (AxisType == EAxisType::None)
	{
		TArray<FInputActionKeyMapping> ActionMappings;
		Settings->GetActionMappingByName(InputName, ActionMappings);

		int Iterations = ActionMappings.Num();
		for (int i = Iterations - 1; i >= 0; --i)
		{
			OutKeys.Add(ActionMappings[i].Key);
		}
	}
	else
	{
		InputName = FName(*InputString.Left(InputString.Len() - 1));
		TArray<FInputAxisKeyMapping> AxisMappings;
		Settings->GetAxisMappingByName(InputName, AxisMappings);

		FKey PotentialAxisKey;
		int Iterations = AxisMappings.Num();
		for (int i = Iterations - 1; i >= 0; --i)
		{
			if ((AxisMappings[i].Scale > 0.0f && AxisType == EAxisType::Positive) ||
				(AxisMappings[i].Scale < 0.0f && AxisType == EAxisType::Negative))
			{
				OutKeys.Add(AxisMappings[i].Key);
			}
			else
			{
				PotentialAxisKey = GetKeyFromAxis(AxisMappings[i].Key, AxisType == EAxisType::Positive);
				if (PotentialAxisKey.IsValid())
				{
					OutKeys.Add(PotentialAxisKey);
				}
			}
		}
	}
}

EInputType UUINavPCComponent::GetKeyInputType(FKey Key)
{
	if (Key.IsGamepadKey())
	{
		return EInputType::Gamepad;
	}
	else if (Key.IsMouseButton())
	{
		return EInputType::Mouse;
	}
	else
	{
		return EInputType::Keyboard;
	}
}

EInputType UUINavPCComponent::GetMenuActionInputType(FString Action)
{
	for (FKey key : KeyMap[Action])
	{
		if (PC->WasInputKeyJustPressed(key)) return GetKeyInputType(key);
	}
	return CurrentInputType;
}

FKey UUINavPCComponent::GetKeyFromAxis(FKey Key, bool bPositive)
{
	FAxis2D_Keys* Axis2DKeys = Axis2DToKeyMap.Find(Key);
	if (Axis2DKeys == nullptr) return FKey();

	return bPositive ? Axis2DKeys->PositiveKey : Axis2DKeys->NegativeKey;
}

bool UUINavPCComponent::Is2DAxis(FKey Key)
{
	return Axis2DToKeyMap.Contains(Key);
}

void UUINavPCComponent::VerifyInputTypeChangeByKey(FKey Key)
{
	EInputType NewInputType = GetKeyInputType(Key);

	if (NewInputType != CurrentInputType)
	{
		NotifyInputTypeChange(NewInputType);
	}
}

void UUINavPCComponent::VerifyInputTypeChangeByAction(FString Action)
{
	EInputType NewInputType = GetMenuActionInputType(Action);

	if (NewInputType != CurrentInputType)
	{
		NotifyInputTypeChange(NewInputType);
	}
}

void UUINavPCComponent::NotifyKeyPressed(FKey PressedKey)
{
	ExecuteActionByKey(PressedKey, true);
}

void UUINavPCComponent::NotifyKeyReleased(FKey ReleasedKey)
{
	ExecuteActionByKey(ReleasedKey, false);
}

void UUINavPCComponent::ExecuteActionByKey(FKey PressedKey, bool bPressed)
{
	FString ActionName = FindActionByKey(PressedKey);
	if (ActionName.Equals(TEXT(""))) return;

	ExecuteActionByName(ActionName, bPressed);
}

FString UUINavPCComponent::FindActionByKey(FKey ActionKey)
{
	TArray<FString> Actions;
	KeyMap.GenerateKeyArray(Actions);
	for (FString action : Actions)
	{
		for (FKey key : KeyMap[action])
		{
			if (key == ActionKey) return action;
		}
	}
	return TEXT("");
}

FReply UUINavPCComponent::OnActionPressed(FString ActionName, FKey Key)
{
	if (!PressedActions.Contains(ActionName))
	{
		PressedActions.AddUnique(ActionName);
		ExecuteActionByName(ActionName, true);
		VerifyInputTypeChangeByKey(Key);
		return FReply::Handled();
	}
	else return FReply::Unhandled();
}

FReply UUINavPCComponent::OnActionReleased(FString ActionName, FKey Key)
{
	if (PressedActions.Contains(ActionName))
	{
		PressedActions.Remove(ActionName);
		ExecuteActionByName(ActionName, false);
		return FReply::Handled();
	}
	else return FReply::Unhandled();
}

EInputMode UUINavPCComponent::GetInputMode()
{
	if (PC != nullptr)
	{
		UGameViewportClient* GameViewportClient = PC->GetWorld()->GetGameViewport();
		ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();

		bool ignore = GameViewportClient->IgnoreInput();
		EMouseCaptureMode capt = GameViewportClient->CaptureMouseOnClick();

		if (ignore == false && capt == EMouseCaptureMode::CaptureDuringMouseDown)
		{
			return EInputMode::GameUI;
		}
		else if (ignore == true && capt == EMouseCaptureMode::NoCapture)
		{
			return EInputMode::UI;
		}
		else
		{
			return EInputMode::Game;
		}
	}
	return EInputMode::None;
}

void UUINavPCComponent::ExecuteActionByName(FString Action, bool bPressed)
{
	if (Action.Equals("MenuUp"))
	{
		if (bPressed) StartMenuUp();
		else MenuUpRelease();
	}
	else if (Action.Equals("MenuDown"))
	{
		if (bPressed) StartMenuDown();
		else MenuDownRelease();
	}
	else if (Action.Equals("MenuLeft"))
	{
		if (bPressed) StartMenuLeft();
		else MenuLeftRelease();
	}
	else if (Action.Equals("MenuRight"))
	{
		if (bPressed) StartMenuRight();
		else MenuRightRelease();
	}
	else if (Action.Equals("MenuSelect"))
	{
		if (bPressed) MenuSelect();
		else
		{
			if (ActiveWidget->GetSelectCount() > 1) PressedActions.Add(Action);
			MenuSelectRelease();
		}
	}
	else if (Action.Equals("MenuReturn"))
	{
		if (bPressed) MenuReturn();
		else MenuReturnRelease();
	}
	else if (Action.Equals("MenuNext") && bPressed)
	{
		MenuNext();
	}
	else if (Action.Equals("MenuPrevious") && bPressed)
	{
		MenuPrevious();
	}
	else
	{
		CallCustomInput(FName(*Action), bPressed);
	}
}

void UUINavPCComponent::NotifyMouseInputType()
{
	if (EInputType::Mouse != CurrentInputType)
	{
		NotifyInputTypeChange(EInputType::Mouse);
	}
}

void UUINavPCComponent::NotifyInputTypeChange(EInputType NewInputType)
{
	IUINavPCReceiver::Execute_OnInputChanged(GetOwner(), CurrentInputType, NewInputType);

	EInputType OldInputType = CurrentInputType;
	CurrentInputType = NewInputType;
	if (ActiveWidget != nullptr) ActiveWidget->OnInputChanged(OldInputType, CurrentInputType);

}

void UUINavPCComponent::MenuInput(ENavigationDirection InDirection)
{
	IUINavPCReceiver::Execute_OnNavigated(GetOwner(), InDirection);

	if (ActiveWidget == nullptr || !bAllowDirectionalInput) return;

	ActiveWidget->NavigateInDirection(InDirection);
}

void UUINavPCComponent::MenuSelect()
{
	VerifyInputTypeChangeByAction(TEXT("MenuSelect"));

	if (ActiveWidget == nullptr || !bAllowSelectInput) return;

	ClearTimer();
	ActiveWidget->MenuSelectPress();
}

void UUINavPCComponent::MenuSelectRelease()
{
	IUINavPCReceiver::Execute_OnSelect(GetOwner());
	VerifyInputTypeChangeByAction(TEXT("MenuSelect"));

	if (ActiveWidget == nullptr || !bAllowSelectInput) return;

	ClearTimer();
	ActiveWidget->MenuSelectRelease();
}

void UUINavPCComponent::MenuReturn()
{
	VerifyInputTypeChangeByAction(TEXT("MenuReturn"));

	if (ActiveWidget == nullptr || !bAllowReturnInput) return;

	ClearTimer();
	ActiveWidget->MenuReturnPress();
}

void UUINavPCComponent::MenuReturnRelease()
{
	IUINavPCReceiver::Execute_OnReturn(GetOwner());

	if (ActiveWidget == nullptr || !bAllowReturnInput) return;

	ClearTimer();
	ActiveWidget->MenuReturnRelease();
}

void UUINavPCComponent::MenuNext()
{
	IUINavPCReceiver::Execute_OnNext(GetOwner());
	VerifyInputTypeChangeByAction(TEXT("MenuNext"));

	if (ActiveWidget == nullptr || !bAllowSectionInput) return;

	ClearTimer();
	ActiveWidget->OnNext();
}

void UUINavPCComponent::MenuPrevious()
{
	IUINavPCReceiver::Execute_OnPrevious(GetOwner());
	VerifyInputTypeChangeByAction(TEXT("MenuPrevious"));

	if (ActiveWidget == nullptr || !bAllowSectionInput) return;

	ClearTimer();
	ActiveWidget->OnPrevious();
}

void UUINavPCComponent::MenuUpRelease()
{
	if (Direction != ENavigationDirection::Up) return;
	Direction = ENavigationDirection::None;

	ClearTimer();
}

void UUINavPCComponent::MenuDownRelease()
{
	if (Direction != ENavigationDirection::Down) return;
	Direction = ENavigationDirection::None;

	ClearTimer();
}

void UUINavPCComponent::MenuLeftRelease()
{
	if (Direction != ENavigationDirection::Left) return;
	Direction = ENavigationDirection::None;

	ClearTimer();
}

void UUINavPCComponent::MenuRightRelease()
{
	if (Direction != ENavigationDirection::Right) return;
	Direction = ENavigationDirection::None;

	ClearTimer();
}

void UUINavPCComponent::MouseKeyPressed(FKey MouseKey)
{
	if (ActiveWidget != nullptr && ActiveWidget->bWaitForInput)
	{
		ActiveWidget->ProcessKeybind(MouseKey);
	}
}

void UUINavPCComponent::StartMenuUp()
{
	if (Direction == ENavigationDirection::Up) return;

	MenuInput(ENavigationDirection::Up);
	VerifyInputTypeChangeByAction(TEXT("MenuUp"));
	Direction = ENavigationDirection::Up;

	if (!bChainNavigation || !bAllowDirectionalInput) return;

	SetTimer(ENavigationDirection::Up);
}

void UUINavPCComponent::StartMenuDown()
{
	if (Direction == ENavigationDirection::Down) return;

	MenuInput(ENavigationDirection::Down);
	VerifyInputTypeChangeByAction(TEXT("MenuDown"));
	Direction = ENavigationDirection::Down;

	if (!bChainNavigation || !bAllowDirectionalInput) return;

	SetTimer(ENavigationDirection::Down);
}

void UUINavPCComponent::StartMenuLeft()
{
	if (Direction == ENavigationDirection::Left) return;

	MenuInput(ENavigationDirection::Left);
	VerifyInputTypeChangeByAction(TEXT("MenuLeft"));
	Direction = ENavigationDirection::Left;

	if (!bChainNavigation || !bAllowDirectionalInput) return;

	SetTimer(ENavigationDirection::Left);
}

void UUINavPCComponent::StartMenuRight()
{
	if (Direction == ENavigationDirection::Right) return;

	MenuInput(ENavigationDirection::Right);
	VerifyInputTypeChangeByAction(TEXT("MenuRight"));
	Direction = ENavigationDirection::Right;

	if (!bChainNavigation || !bAllowDirectionalInput) return;

	SetTimer(ENavigationDirection::Right);
}
