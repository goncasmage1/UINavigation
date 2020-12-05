// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once
#include "Data/NavigationEvent.h"
#include "Data/GridButton.h"
#include "Data/NavigationDirection.h"
#include "DynamicEdgeNavigation.generated.h"

USTRUCT(BlueprintType)
struct FDynamicEdgeNavigation
{
	GENERATED_BODY()

	FDynamicEdgeNavigation() {}

	FDynamicEdgeNavigation(const int InGridIndex, ENavigationEvent InEvent, const int InTargetGridIndex, TArray<int> InTargetButtonIndices, const ENavigationDirection InDirection, const bool InbTwoWayConnection = true)
	{
		GridIndex = InGridIndex;
		Event = InEvent;
		TargetGridIndex = InTargetGridIndex;
		TargetButtonIndices = InTargetButtonIndices;
		Direction = InDirection;
		bTwoWayConnection = InbTwoWayConnection;
	}

	FDynamicEdgeNavigation(const int InGridIndex, ENavigationEvent InEvent, TArray<FGridButton> InTargetButtons, const ENavigationDirection InDirection, const bool InbTwoWayConnection = true)
	{
		GridIndex = InGridIndex;
		Event = InEvent;
		TargetButtons = InTargetButtons;
		Direction = InDirection;
		bTwoWayConnection = InbTwoWayConnection;
	}

	UPROPERTY(BlueprintReadWrite, Category = DynamicEdgeNavigation)
		int GridIndex;

	UPROPERTY(BlueprintReadWrite, Category = DynamicEdgeNavigation)
		ENavigationEvent Event;

	UPROPERTY(BlueprintReadWrite, Category = DynamicEdgeNavigation)
		int TargetGridIndex = -1;

	UPROPERTY(BlueprintReadWrite, Category = DynamicEdgeNavigation)
		TArray<int> TargetButtonIndices;

	UPROPERTY(BlueprintReadWrite, Category = DynamicEdgeNavigation)
		TArray<FGridButton> TargetButtons;

	UPROPERTY(BlueprintReadWrite, Category = DynamicEdgeNavigation)
		ENavigationDirection Direction;
	
	UPROPERTY(BlueprintReadWrite, Category = DynamicEdgeNavigation)
		bool bTwoWayConnection = true;

};