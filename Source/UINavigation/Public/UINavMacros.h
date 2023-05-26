// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Engine/Engine.h"

#define DISPLAYERROR(Text) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("%s"), *(FString(TEXT("Error in ")).Append(GetName()).Append(TEXT(": ")).Append(Text))))
#define DISPLAYERROR_STATIC(Widget, Text) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("%s"), *(FString(TEXT("Error in ")).Append(Widget->GetName()).Append(TEXT(": ")).Append(Text))))
#define DISPLAYWARNING(Text) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Orange, FString::Printf(TEXT("%s"), *(FString(TEXT("Warning in ")).Append(GetName()).Append(TEXT(": ")).Append(Text))))
#define IS_VR_PLATFORM !PLATFORM_SWITCH
