// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

UENUM(BlueprintType)
enum class EInputRestriction : uint8
{
	None UMETA(DisplayName = "None"),
	Keyboard UMETA(DisplayName = "Keyboard"),
	Mouse UMETA(DisplayName = "Mouse"),
	Keyboard_Mouse UMETA(DisplayName = "Keyboard and Mouse"),
	Gamepad UMETA(DisplayName = "Gamepad"),
};