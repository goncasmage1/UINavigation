// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

#pragma once

#include "Engine.h"

#define DISPLAYERROR(Text) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("%s"), *(FString(TEXT("Error in ")).Append(GetName()).Append(TEXT(": ")).Append(Text))))
#define DISPLAYWARNING(Text) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Orange, FString::Printf(TEXT("%s"), *(FString(TEXT("Warning in ")).Append(GetName()).Append(TEXT(": ")).Append(Text))))