// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Query/Tests/RogueCover_QueryCoverOccupied.h"
#include "Framework/CoverSystem/RogueCover_Subsystem.h"
#include "Framework/Query/RogueCover_EnvQueryCover.h"

URogueCover_QueryCoverOccupied::URogueCover_QueryCoverOccupied(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = URogueCover_EnvQueryCover::StaticClass();
	SetWorkOnFloatValues(false);
}

void URogueCover_QueryCoverOccupied::RunTest(FEnvQueryInstance& QueryInstance) const
{	
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		// Get current CoverPoint
		if(const URogueCover_Point* CurrentCoverPoint = URogueCover_EnvQueryCover::GetValue(QueryInstance.RawData.GetData() + QueryInstance.Items[It.GetIndex()].DataOffset))
		{
			It.SetScore(TestPurpose, FilterType, CurrentCoverPoint->IsOccupied(), true);
		}
	}	
}
