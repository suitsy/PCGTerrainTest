// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Player/RTSEntities_PlayerControllerCommand.h"
#include "PTT_PlayerController.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class PCGTERRAINTEST_API APTT_PlayerController : public ARTSEntities_PlayerControllerCommand
{
	GENERATED_BODY()

public:
	APTT_PlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
