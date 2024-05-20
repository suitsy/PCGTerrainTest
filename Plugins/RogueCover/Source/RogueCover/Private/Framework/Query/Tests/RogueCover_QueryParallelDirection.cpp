// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Query/Tests/RogueCover_QueryParallelDirection.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "Framework/CoverSystem/RogueCover_Point.h"
#include "Framework/Query/RogueCover_EnvQueryCover.h"


URogueCover_QueryParallelDirection::URogueCover_QueryParallelDirection(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = URogueCover_EnvQueryCover::StaticClass();
	SetWorkOnFloatValues(false);

	Context = UEnvQueryContext_Querier::StaticClass();
}

void URogueCover_QueryParallelDirection::RunTest(FEnvQueryInstance& QueryInstance) const
{
	const UObject* DataOwner = QueryInstance.Owner.Get();
	BoolValue.BindData(DataOwner, QueryInstance.QueryID);
	MaxEpsilon.BindData(DataOwner, QueryInstance.QueryID);

	const bool bWantsHit = BoolValue.GetValue();
	const float MaxEps = MaxEpsilon.GetValue();

	TArray<FVector> ContextLocations;
	if (!QueryInstance.PrepareContext(Context, ContextLocations))
	{
		return;
	}

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		if (const URogueCover_Point* CoverPoint = URogueCover_EnvQueryCover::GetValue(QueryInstance.RawData.GetData() + QueryInstance.Items[It.GetIndex()].DataOffset))
		{
			const FVector CoverDirection = CoverPoint->GetDirectionToWall();
			const float CoverDirectionSize = CoverDirection.Size(); 
			const FVector CoverPosition = CoverPoint->Location; 

			for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
			{
				const FVector ContextLocation = ContextLocations[ContextIndex]; 
				FVector DirectionCoverToContext = ContextLocation - CoverPosition; 

				// Check if parallel
				const bool bParallel = FVector::DotProduct(CoverDirection, DirectionCoverToContext) / (CoverDirectionSize*DirectionCoverToContext.Size()) > 1 - MaxEps;
				It.SetScore(TestPurpose, FilterType, bParallel, bWantsHit);
			}
		}		
	}
}
