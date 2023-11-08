// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "GenericPlatform/ICursor.h"
#include "UINavGameViewportClient.generated.h"

class FViewport;

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()
	
public:

	virtual EMouseCursor::Type GetCursor(FViewport* InViewport, int32 X, int32 Y) override;
};
