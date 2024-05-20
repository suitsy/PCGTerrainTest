// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Managers/RTSCore_GameInstance.h"

#include "PTT_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PCGTERRAINTEST_API UPTT_GameInstance : public URTSCore_GameInstance
{
	GENERATED_BODY()

public:
	UPTT_GameInstance(const FObjectInitializer& ObjectInitializer);
};
