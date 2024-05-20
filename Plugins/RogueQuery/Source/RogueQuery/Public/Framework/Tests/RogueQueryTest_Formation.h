// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueQueryTest.h"
#include "Framework/Queries/RogueQueryTraceHelpers.h"
#include "RogueQueryTest_Formation.generated.h"

/**
 * 
 */
UCLASS()
class ROGUEQUERY_API URogueQueryTest_Formation : public URogueQueryTest
{
	GENERATED_BODY()

public:
	URogueQueryTest_Formation(const FObjectInitializer& ObjectInitializer);
	virtual void RunTest(FRogueQueryInstance& QueryInstance) const override;
	//void ProjectToNavigation(const FRogueQueryInstance& QueryInstance, TArray<FNavLocation>& PointsToProject, const FRogueQueryHelpers::ERogueTraceMode Mode = FRogueQueryHelpers::ERogueTraceMode::Discard) const;
	static bool IsValidSourcePoint(const TArray<FNavLocation>& PointsToProject) { return PointsToProject.Num() > 0; }
	static void CreateItemGrid(TArray<FNavLocation>& GridPoints, const FRogueQueryInstance& QueryInstance, const FVector& Position, FVector2D& GridData);
	static void CreateGridQueryItems(TArray<FNavLocation>& GridPoints, TArray<FRogueQueryItem>& GridItems);

private:
	/** trace params */
	UPROPERTY()
	FRogueQueryTraceData ProjectionData;
};
