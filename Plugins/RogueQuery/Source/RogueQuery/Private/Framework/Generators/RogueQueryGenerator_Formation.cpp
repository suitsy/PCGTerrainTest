// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Generators/RogueQueryGenerator_Formation.h"

URogueQueryGenerator_Formation::URogueQueryGenerator_Formation(): Offset(FVector(0.f, -1.f, 0.f)), SpaceBetween(200.f), NumPositions(6)
{}

void URogueQueryGenerator_Formation::SetCustomData(const FRogueQueryCustomData& Data)
{
}

void URogueQueryGenerator_Formation::GenerateItems(FRogueQueryInstance& QueryInstance)
{
	TArray<FNavLocation> GeneratedPoints;

	if(QueryInstance.CustomData.IsValid())
	{
		FVector2D Spacing = FVector2D(0.f,0.f);	
	
		const bool bIsOddCount = QueryInstance.CustomData.NumPositions % 2 != 0;
		const FRotator Rotation = FRotator::ZeroRotator;
	
		for (int i = 0; i < QueryInstance.CustomData.NumPositions; ++i)
		{
			FVector LocalOffset = QueryInstance.CustomData.Offset;
		
			if(i == 0 && !bIsOddCount)
			{
				Spacing.X = QueryInstance.CustomData.SpaceBetween * 0.5f;
				Spacing.Y = QueryInstance.CustomData.SpaceBetween * 0.5f;
			}
		
			if(i % 2 != 0)
			{
				// Left
				if(Spacing.X == 0.f && !bIsOddCount)
				{
					Spacing.X += QueryInstance.CustomData.SpaceBetween;
				}				

				LocalOffset.Y = LocalOffset.Y * -1;
				LocalOffset *= Spacing.X;
				Spacing.X += QueryInstance.CustomData.SpaceBetween;
			}
			else
			{
				// Right
				if(Spacing.Y == 0.f && !bIsOddCount)
				{
					Spacing.Y += QueryInstance.CustomData.SpaceBetween;
				}				

				LocalOffset *= Spacing.Y;
				Spacing.Y += QueryInstance.CustomData.SpaceBetween;
			}
		
			LocalOffset = Rotation.RotateVector(LocalOffset);
			GeneratedPoints.Add(FNavLocation(QueryInstance.SourceLocation + LocalOffset));
		}
	}

	FinishGeneration(QueryInstance, GeneratedPoints);
}
