// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/EQS/Tests/RTSAi_GroupPositions.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"

URTSAi_GroupPositions::URTSAi_GroupPositions(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Context = UEnvQueryContext_Querier::StaticClass();
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
}

void URTSAi_GroupPositions::RunTest(FEnvQueryInstance& QueryInstance) const
{
	if(const UObject* QueryOwner = QueryInstance.Owner.Get())
	{		
		BoolValue.BindData(QueryOwner, QueryInstance.QueryID);
		const bool bWantsHit = BoolValue.GetValue();
		MaxTraceDistance.BindData(QueryOwner, QueryInstance.QueryID);		
		const float TraceDistance = MaxTraceDistance.GetValue();
		PositionSpacing.BindData(QueryOwner, QueryInstance.QueryID);		
		const float Spacing = PositionSpacing.GetValue();
		PositionsAmount.BindData(QueryOwner, QueryInstance.QueryID);		
		const float Amount = PositionsAmount.GetValue();
		
		TArray<FVector> ContextLocations;
		if (!QueryInstance.PrepareContext(Context, ContextLocations))
		{
			return;
		}

		TArray<FVector> Positions;
		if(ContextLocations.IsValidIndex(0))
		{			
			CreateSourcePositions(Positions, ContextLocations[0], Spacing, Amount);
		}
		
		
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
			AActor* ItemActor = GetItemActor(QueryInstance, It.GetIndex());

			
			
			for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
			{
				// Calculate end trace distance 
				FVector Direction = ContextLocations[ContextIndex] - ItemLocation;
				Direction.Normalize(); 
				const FVector TraceEnd = ItemLocation + TraceDistance * Direction;				

				FCollisionQueryParams TraceParams(TEXT("EnvQueryTrace"), TraceData.bTraceComplex);
				TraceParams.bTraceComplex = true;
				TraceParams.bReturnPhysicalMaterial = false;
				const ECollisionChannel TraceCollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceData.TraceChannel);

				// Perform trace
				const bool bHit = RunLineTraceTo(ItemLocation, TraceEnd, ItemActor, QueryInstance.World, TraceCollisionChannel, TraceParams);

				It.SetScore(TestPurpose, FilterType, bHit, bWantsHit);
			}
		}
	}
}

FText URTSAi_GroupPositions::GetDescriptionDetails() const
{
	return DescribeFloatTestParams();
}

bool URTSAi_GroupPositions::RunLineTraceTo(const FVector& ItemPos, const FVector& ContextPos, AActor* ItemActor,
	UWorld* World, ECollisionChannel Channel, const FCollisionQueryParams& Params) const
{
	FCollisionQueryParams TraceParams(Params);
	TraceParams.AddIgnoredActor(ItemActor);

	const bool bHit = World->LineTraceTestByChannel(ContextPos, ItemPos, Channel, TraceParams);
	return bHit;
}

void URTSAi_GroupPositions::CreateSourcePositions(TArray<FVector>& PositionsArray, const FVector& SourcePosition, const float Spacing, const int32 Count) const
{
	float SpacingLeft = 0.f, SpacingRight = 0.f;
	const bool bIsOddCount = Count % 2 != 0;
	for (int i = 0; i < Count; ++i)
	{
		FVector LocalOffset = Offset;
		if(i % 2 != 0)
		{
			if(SpacingLeft == 0.f && !bIsOddCount)
			{
				SpacingLeft += Spacing * 0.5f;
			}				
			
			LocalOffset *= SpacingLeft;
			LocalOffset.Y = LocalOffset.Y * -1;
			PositionsArray.Add(SourcePosition + LocalOffset);
			SpacingLeft += Spacing;
		}
		else
		{
			if(SpacingRight == 0.f && !bIsOddCount)
			{
				SpacingRight += Spacing * 0.5f;
			}				
			
			LocalOffset *= SpacingRight;			
			PositionsArray.Add(SourcePosition + LocalOffset);
			SpacingRight += Spacing;
		}
	}
}
