// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Query/Tests/RogueCover_QueryTraceDistance.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"


URogueCover_QueryTraceDistance::URogueCover_QueryTraceDistance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Cost = EEnvTestCost::High;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
	SetWorkOnFloatValues(false);

	Context = UEnvQueryContext_Querier::StaticClass();
	TraceData.SetGeometryOnly();
}

void URogueCover_QueryTraceDistance::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* DataOwner = QueryInstance.Owner.Get();
	BoolValue.BindData(DataOwner, QueryInstance.QueryID);
	MaxTraceDistance.BindData(DataOwner, QueryInstance.QueryID);
	VerticalOffset.BindData(DataOwner, QueryInstance.QueryID);

	const bool bWantsHit = BoolValue.GetValue();
	const float TraceDistance = MaxTraceDistance.GetValue();
	const float VerticalOff = VerticalOffset.GetValue(); 

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
		const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
		AActor* ItemActor = GetItemActor(QueryInstance, It.GetIndex());

		for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
		{
			// Calculate end trace distance 
			FVector Direction = ContextLocations[ContextIndex] - ItemLocation;
			Direction.Normalize(); 
			const FVector StarTrace = ItemLocation + FVector(0, 0, VerticalOff); 
			const FVector EndTrace = StarTrace + TraceDistance * Direction;

			// Perform trace
			const bool bHit = RunLineTraceTo(StarTrace, EndTrace, ItemActor, QueryInstance.World, TraceCollisionChannel, TraceParams);

			It.SetScore(TestPurpose, FilterType, bHit, bWantsHit);
		}
	}
}

bool URogueCover_QueryTraceDistance::RunLineTraceTo(const FVector& ItemPos, const FVector& ContextPos,
const AActor* ItemActor, const UWorld* World, const ECollisionChannel Channel, const FCollisionQueryParams& Params) const
{
	FCollisionQueryParams TraceParams(Params);
	TraceParams.AddIgnoredActor(ItemActor);
	return World->LineTraceTestByChannel(ContextPos, ItemPos, Channel, TraceParams);
}
