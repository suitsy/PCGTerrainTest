// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Debug/RTSCore_DebugGamePhase.h"
#include "CommonTextBlock.h"
#include "Framework/Managers/RTSCore_GameState.h"


void URTSCore_DebugGamePhase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if(GamePhaseText && GetWorld())
	{		
		if(const ARTSCore_GameState* RTSGameState = GetWorld()->GetGameState<ARTSCore_GameState>())
		{
			GamePhaseText->SetText(UEnum::GetDisplayValueAsText(RTSGameState->GetGamePhaseState()));
		}			
	}
}
