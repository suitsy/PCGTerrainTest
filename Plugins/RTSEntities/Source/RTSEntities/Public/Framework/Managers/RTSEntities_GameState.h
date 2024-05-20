// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Managers/RTSCore_GameState.h"
#include "RTSEntities_GameState.generated.h"

class URTSEntities_LoadoutManager;


UCLASS(Abstract)
class RTSENTITIES_API ARTSEntities_GameState : public ARTSCore_GameState
{
	GENERATED_BODY()

public:
	ARTSEntities_GameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	URTSEntities_LoadoutManager* LoadoutManager;
};
