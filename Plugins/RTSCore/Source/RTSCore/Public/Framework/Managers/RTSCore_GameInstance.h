// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/RTSCore_DataTypes.h"
#include "CommonGameInstance.h"
#include "RTSCore_GameInstance.generated.h"


/**
 *  RTS Core Game Instance 
 */
UCLASS()
class RTSCORE_API URTSCore_GameInstance : public UCommonGameInstance
{
	GENERATED_BODY()

public:
	URTSCore_GameInstance(const FObjectInitializer& ObjectInitializer);
	virtual void SetGamePhaseState(ERTSCore_GamePhaseState NewPhase);
	virtual ERTSCore_GamePhaseState GetGamePhaseState() const { return GamePhaseState; }

protected:	
	UPROPERTY()
	ERTSCore_GamePhaseState GamePhaseState = ERTSCore_GamePhaseState::NewGame;
};
