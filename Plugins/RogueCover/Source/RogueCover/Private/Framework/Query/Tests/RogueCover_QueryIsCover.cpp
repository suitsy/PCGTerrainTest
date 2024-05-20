// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Query/Tests/RogueCover_QueryIsCover.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "Framework/CoverSystem/RogueCover_Point.h"
#include "Framework/Query/RogueCover_EnvQueryCover.h"

URogueCover_QueryIsCover::URogueCover_QueryIsCover(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Cost = EEnvTestCost::High;
	ValidItemType = URogueCover_EnvQueryCover::StaticClass();
	SetWorkOnFloatValues(false);

	Context = UEnvQueryContext_Querier::StaticClass();
	TraceData.SetGeometryOnly();
}

void URogueCover_QueryIsCover::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* DataOwner = QueryInstance.Owner.Get();
	ContextHorizontalDistanceOffset.BindData(DataOwner, QueryInstance.QueryID);
	ContextVerticalDistanceOffset.BindData(DataOwner, QueryInstance.QueryID);
	DebugData.BindData(DataOwner, QueryInstance.QueryID);

	float HorizontalOffset = ContextHorizontalDistanceOffset.GetValue();
	float VerticalOffset = ContextVerticalDistanceOffset.GetValue();
	bool Debug = DebugData.GetValue(); 

	TArray<FVector> ContextLocations;
	if (!QueryInstance.PrepareContext(Context, ContextLocations))
	{
		return;
	}

	FCollisionQueryParams TraceParams(TEXT("EnvQueryTrace"), TraceData.bTraceComplex);
	TraceParams.bTraceComplex = true;

	TArray<AActor*> IgnoredActors;
	if (QueryInstance.PrepareContext(Context, IgnoredActors))
	{
		TraceParams.AddIgnoredActors(IgnoredActors);
	}

	ECollisionChannel TraceCollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceData.TraceChannel);

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		
		// Get current CoverPoint
		URogueCover_Point* CurrentCoverPoint = URogueCover_EnvQueryCover::GetValue(QueryInstance.RawData.GetData() + QueryInstance.Items[It.GetIndex()].DataOffset);
		// Generate Directions
		const FVector Front	= CurrentCoverPoint->GetDirectionToWall();
		const FVector Back	= -Front;
		const FVector Right	= PerpendicularClockwise(Front);
		const FVector Left = -Right;
		// Get point location
		const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex()) + FVector(0, 0, VerticalOffset);

		//AActor* ItemActor = GetItemActor(QueryInstance, It.GetIndex());

		for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
		{
			//const bool bHitCenter = RunLineTraceTo(ItemLocation, ContextLocations[ContextIndex], NULL, QueryInstance.World, TraceCollisionChannel, TraceParams);
			/*const bool bHitLeft = RunLineTraceTo(ItemLocation + Left, ContextLocations[ContextIndex], NULL, QueryInstance.World, TraceCollisionChannel, TraceParams);
			const bool bHitRight = RunLineTraceTo(ItemLocation + Right, ContextLocations[ContextIndex], NULL, QueryInstance.World, TraceCollisionChannel, TraceParams);
			const bool bHitFront = RunLineTraceTo(ItemLocation + Front, ContextLocations[ContextIndex], NULL, QueryInstance.World, TraceCollisionChannel, TraceParams);
			const bool bHitBack = RunLineTraceTo(ItemLocation + Back, ContextLocations[ContextIndex], NULL, QueryInstance.World, TraceCollisionChannel, TraceParams);
			*/
			const bool bHitLeftUp = RunLineTraceTo(ItemLocation + HorizontalOffset*Left, ContextLocations[ContextIndex], NULL, QueryInstance.World, TraceCollisionChannel, TraceParams, Debug);
			const bool bHitRightUp = RunLineTraceTo(ItemLocation + HorizontalOffset*Right, ContextLocations[ContextIndex], NULL, QueryInstance.World, TraceCollisionChannel, TraceParams, Debug);
			/*const bool bHitFrontUp = RunLineTraceTo(ItemLocation + Front, ContextLocations[ContextIndex], NULL, QueryInstance.World, TraceCollisionChannel, TraceParams);
			const bool bHitBackUp = RunLineTraceTo(ItemLocation + Back, ContextLocations[ContextIndex], NULL, QueryInstance.World, TraceCollisionChannel, TraceParams);
			*/
			//CurrentCoverPoint->bRightCover = !bHitRightUp;
			//CurrentCoverPoint->bLeftCover = !bHitLeftUp;
			

			bool bHit = bHitLeftUp && bHitRightUp; 

			It.SetScore(TestPurpose, FilterType, bHit, false);
		}
	}
}

bool URogueCover_QueryIsCover::RunLineTraceTo(const FVector& ItemPos, const FVector& ContextPos, const AActor* ItemActor,
const UWorld* World, const ECollisionChannel Channel, const FCollisionQueryParams& Params, const bool Debug) const
{
	FCollisionQueryParams TraceParams(Params);

	if (Debug && World)
	{
		DrawDebugLine(World, ContextPos, ItemPos, FColor::Yellow, false, 5.0f, 1, 3);
	}

	TraceParams.AddIgnoredActor(ItemActor);

	const bool bHit = World->LineTraceTestByChannel(ContextPos, ItemPos, Channel, TraceParams);
	return bHit;
}

FVector URogueCover_QueryIsCover::PerpendicularClockwise(FVector V)
{
	return FVector(-V.Y, V.X, 0.f);
}
