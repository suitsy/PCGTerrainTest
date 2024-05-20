// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Player/RTSTeams_PlayerState.h"
#include "PTT_PlayerState.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class PCGTERRAINTEST_API APTT_PlayerState : public ARTSTeams_PlayerState
{
	GENERATED_BODY()

public:
	APTT_PlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
