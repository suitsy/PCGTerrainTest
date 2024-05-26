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

void URTSCore_SystemStatics::GetTerrainTransform(UWorld* WorldContext, FTransform& TerrainTransform)
{
	FHitResult Hit;
	FCollisionQueryParams CollisionParams;
	FVector TraceOrigin = TerrainTransform.GetLocation();
	TraceOrigin.Z += 10000.f;
	FVector TraceEnd = TerrainTransform.GetLocation();
	TraceEnd.Z -= 10000.f;

	if(WorldContext)
	{
		if(WorldContext->LineTraceSingleByChannel(Hit, TraceOrigin, TraceEnd, RTS_TRACE_CHANNEL_TERRAIN, CollisionParams))
		{
			if(Hit.GetActor() != nullptr)
			{
				TerrainTransform = Hit.GetActor()->GetTransform();
			}
		}
	}	
}

void URTSCore_SystemStatics::GetTerrainLocationAndNormal(UWorld* WorldContext, FVector& Location, FVector& Normal)
{
	FHitResult Hit;
	FCollisionQueryParams CollisionParams;
	FVector TraceOrigin = Location;
	TraceOrigin.Z += 10000.f;
	FVector TraceEnd = Location;
	TraceEnd.Z -= 10000.f;

	if(WorldContext)
	{
		if(WorldContext->LineTraceSingleByChannel(Hit, TraceOrigin, TraceEnd, RTS_TRACE_CHANNEL_TERRAIN, CollisionParams))
		{
			if(Hit.bBlockingHit)
			{
				Location = Hit.ImpactPoint;
				Normal = Hit.ImpactNormal;
			}
		}
	}
}
