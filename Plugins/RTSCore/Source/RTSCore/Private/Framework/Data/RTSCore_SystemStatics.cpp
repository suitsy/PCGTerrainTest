// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Data/RTSCore_SystemStatics.h"
#include "Framework/Data/RTSCore_StaticGameData.h"

void URTSCore_SystemStatics::GetTerrainPosition(UWorld* WorldContext, FVector& TerrainPosition)
{
	FHitResult Hit;
	FCollisionQueryParams CollisionParams;
	FVector TraceOrigin = TerrainPosition;
	TraceOrigin.Z += 10000.f;
	FVector TraceEnd = TerrainPosition;
	TraceEnd.Z -= 10000.f;

	if(WorldContext)
	{
		if(WorldContext->LineTraceSingleByChannel(Hit, TraceOrigin, TraceEnd, RTS_TRACE_CHANNEL_TERRAIN, CollisionParams))
		{
			TerrainPosition = Hit.ImpactPoint;
		}
	}	
}
