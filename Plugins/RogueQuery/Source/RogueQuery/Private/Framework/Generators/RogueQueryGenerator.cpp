// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Generators/RogueQueryGenerator.h"
#include "AI/Navigation/NavigationTypes.h"
#include "AI/Navigation/NavAgentInterface.h"
#include "Framework/Data/RogueQuery.h"
#include "Framework/Queries/RogueQueryTraceHelpers.h"
#include "NavMesh/RecastNavMesh.h"

URogueQueryGenerator::URogueQueryGenerator()
{
	PathToItem.DefaultValue = true;
	ScanRangeMultiplier.DefaultValue = 1.5f;
}

void URogueQueryGenerator::PostInitProperties()
{
	Super::PostInitProperties();

	GeneratorName = GetFName();
}

void URogueQueryGenerator::PostLoad()
{
	Super::PostLoad();
	
	if (GeneratorName == NAME_None || GeneratorName.IsValid() == false)
	{
		GeneratorName = GetFName();
	}
}

#if WITH_EDITOR
void URogueQueryGenerator::PostRename(UObject* OldOuter, const FName OldName)
{
	Super::PostRename(OldOuter, OldName);

	GeneratorName = GetFName();
}

void URogueQueryGenerator::PostDuplicate(bool bDuplicateForPIE)
{
	if (bDuplicateForPIE == false)
	{
		GeneratorName = GetFName();
	}

	Super::PostDuplicate(bDuplicateForPIE);
}
#endif


void URogueQueryGenerator::FinishGeneration(FRogueQueryInstance& QueryInstance, TArray<FNavLocation>& GeneratedPoints) const
{
	// Project points to navigation if required
	if(NavPointsOnly)
	{
		FRogueQueryHelpers::ProjectAndFilterNavPoints(GeneratedPoints, QueryInstance, ProjectionData);
	}

	// Init items on query instance
	QueryInstance.InitItems(GeneratedPoints);	

	// Prepare item details for testing
	if (QueryInstance.NumValidItems > 0)
	{
		// Reset test details
		QueryInstance.ItemDetails.Reserve(QueryInstance.NumValidItems);
		for (int32 ItemIndex = 0; ItemIndex < QueryInstance.NumValidItems; ItemIndex++)
		{
			QueryInstance.ItemDetails.Add(FRogueQueryItemDetails(QueryInstance.Tests.Num(), ItemIndex));
		}
	}
}