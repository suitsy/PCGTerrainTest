// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Managers/RTSCore_GameInstance.h"

URTSCore_GameInstance::URTSCore_GameInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void URTSCore_GameInstance::SetGamePhaseState(const ERTSCore_GamePhaseState NewPhase)
{
	// Check the new game phase is not the current or a lower phase than the current phase
	if(GamePhaseState != NewPhase && GamePhaseState < NewPhase)
	{
		GamePhaseState = NewPhase;
	}
}
