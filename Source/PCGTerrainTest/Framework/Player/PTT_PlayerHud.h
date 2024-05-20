// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Player/RTSEntities_Hud.h"
#include "PTT_PlayerHud.generated.h"

/**
 * 
 */
UCLASS()
class PCGTERRAINTEST_API APTT_PlayerHud : public ARTSEntities_Hud
{
	GENERATED_BODY()

public:
	explicit APTT_PlayerHud(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
