// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Managers/RTSEntities_GameState.h"
#include "PTT_GameState.generated.h"

class URTSEquipment_ManagerComponent;
class URTSCore_GamePhaseManager;
class URTSEntities_LoadoutManager;
class URTSTeams_Manager;

UCLASS(Abstract)
class PCGTERRAINTEST_API APTT_GameState : public ARTSEntities_GameState
{
	GENERATED_BODY()

public:
	APTT_GameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	URTSEquipment_ManagerComponent* EquipmentManagerComponent;
};
