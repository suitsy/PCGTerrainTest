// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Generators/RogueQueryGenerator_Grid.h"

URogueQueryGenerator_Grid::URogueQueryGenerator_Grid()
{
	
}

void URogueQueryGenerator_Grid::GenerateItems(FRogueQueryInstance& QueryInstance)
{
	TArray<FNavLocation> GeneratedPoints;
	const int32 ItemCount = FPlatformMath::TruncToInt((GridSize * 2.0f / SpaceBetween) + 1);
	const int32 ItemCountHalf = ItemCount / 2;

	GeneratedPoints.Reserve(ItemCount * ItemCount);

	for (int32 IndexX = 0; IndexX < ItemCount; ++IndexX)
	{
		for (int32 IndexY = 0; IndexY < ItemCount; ++IndexY)
		{
			const FNavLocation TestPoint = FNavLocation(QueryInstance.SourceLocation - FVector(SpaceBetween * (IndexX - ItemCountHalf), SpaceBetween * (IndexY - ItemCountHalf), 0));
			GeneratedPoints.Add(TestPoint);
		}
	}

	FinishGeneration(QueryInstance, GeneratedPoints);
}
