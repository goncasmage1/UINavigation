// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavSlider.h"
#include "UINavController.h"
#include "Button.h"
#include "Image.h"
#include "CanvasPanelSlot.h"
#include "Blueprint/SlateBlueprintLibrary.h"

void UUINavSlider::NativeConstruct()
{
	Super::NativeConstruct();

	SliderHandle->SetClickMethod(EButtonClickMethod::MouseDown);
	SliderHandle->OnClicked.AddDynamic(this, &UUINavSlider::OnClick);
	SliderHandle->OnReleased.AddDynamic(this, &UUINavSlider::OnRelease);
	CurrentValue = DefaultValue;
}

void UUINavSlider::NativeTick(const FGeometry & MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	if (!bDraggingSlider) return;

	MoveHandle();
	CalculateValue();
}

void UUINavSlider::SetPC(AUINavController * NewPC)
{
	UINavPC = NewPC;
}

//void UUINavSlider::OnCaptureBegin()
//{
//	AttemptedValue = DefaultValue;
//	SnapToValue();
//}
//
//void UUINavSlider::OnValueUpdate(float NewValue)
//{
//	AttemptedValue = NewValue;
//	SnapToValue();
//}

void UUINavSlider::OnClick()
{
	bDraggingSlider = true;
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Clicked"));
}

void UUINavSlider::OnRelease()
{
	bDraggingSlider = false;
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Released"));
}

void UUINavSlider::MoveHandle()
{

}

void UUINavSlider::CalculateValue()
{

	float LocationX, LocationY;
	if (UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetMousePosition(LocationX, LocationY))
	{
		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot);
		if (CanvasSlot == nullptr)
		{
			CurrentValue = -1.f;
			return;
		}
		FVector2D Pos = CanvasSlot->GetPosition();
		FVector2D Size = GetDesiredSize();

		GEngine->AddOnScreenDebugMessage(1, 4.f, FColor::Green, FString::Printf(TEXT("Position: %s"), *Pos.ToString()));
		GEngine->AddOnScreenDebugMessage(2, 4.f, FColor::Green, FString::Printf(TEXT("Size: %s"), *Size.ToString()));
	}
	else
	{
		CurrentValue = -1.f;
	}
}

void UUINavSlider::SnapToValue()
{
	int Count = 0;
	float Division = 1.f / (Steps);
	float NewValue = 0.f;
	NewValue += Division;

	while (NewValue < 1.f)
	{
		if (AttemptedValue < NewValue)
		{
			break;
		}
		NewValue += Division;
		Count++;
	}

	StepIndex = Count;
	//SetValue(1.f / (Steps - 1)*StepIndex);
}

void UUINavSlider::IncreaseStepIndex()
{
	if (StepIndex < Steps)
	{
		StepIndex++;
		//SetValue(1.f / (Steps - 1)*StepIndex);
	}
}

void UUINavSlider::DecreaseStepIndex()
{
	if (StepIndex > 0)
	{
		StepIndex--;
		//SetValue(1.f / (Steps - 1)*StepIndex);
	}
}

