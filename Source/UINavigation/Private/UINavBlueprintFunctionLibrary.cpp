// Fill out your copyright notice in the Description page of Project Settings.

#include "UINavBlueprintFunctionLibrary.h"
#include "Sound/SoundClass.h"
#include "GameFramework/GameUserSettings.h"

void UUINavBlueprintFunctionLibrary::SetSoundClassVolume(USoundClass * TargetClass, float NewVolume)
{
	if (TargetClass == nullptr) return;
	TargetClass->Properties.Volume = NewVolume;
}

float UUINavBlueprintFunctionLibrary::GetSoundClassVolume(USoundClass * TargetClass)
{
	if (TargetClass == nullptr) return -1.f;
	return TargetClass->Properties.Volume;
}
