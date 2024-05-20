// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Managers/RTSCore_GameMode.h"
#include "PTT_GameMode.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class PCGTERRAINTEST_API APTT_GameMode : public ARTSCore_GameMode
{
	GENERATED_BODY()

public:
	APTT_GameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
