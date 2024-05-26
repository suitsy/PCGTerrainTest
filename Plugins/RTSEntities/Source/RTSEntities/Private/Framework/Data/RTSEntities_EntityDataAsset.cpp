// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Data/RTSEntities_EntityDataAsset.h"

void URTSEntities_EntityDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Initialize SpeedStates array with default values if it's empty
	if (SpeedStates.Num() == 0)
	{
		for (int32 i = 0; i < static_cast<int32>(ERTSCore_SpeedState::Sprint) + 1; ++i)
		{
			FRTSEntities_Speeds NewState;
			NewState.SpeedState = static_cast<ERTSCore_SpeedState>(i);
			NewState.Speed = 0.f; // Default speed
			SpeedStates.Add(NewState);
		}
	}
}

float URTSEntities_EntityDataAsset::GetStateSpeed(const ERTSCore_SpeedState& SpeedState) const
{
	for (int i = 0; i < SpeedStates.Num(); ++i)
	{
		if(SpeedStates[i].SpeedState == SpeedState)
		{
			return SpeedStates[i].Speed;
		}
	}

	return 0.f;
}
