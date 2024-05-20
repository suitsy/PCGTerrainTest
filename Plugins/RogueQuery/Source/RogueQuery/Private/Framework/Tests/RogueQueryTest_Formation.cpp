// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Tests/RogueQueryTest_Formation.h"
#include "Framework/Generators/RogueQueryGenerator_Formation.h"

URogueQueryTest_Formation::URogueQueryTest_Formation(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ProjectionData = FRogueQueryTraceData();
	ProjectionData.bTraceComplex = true;
	ProjectionData.TraceMode = ERogueQueryTrace::Navigation;
}

void URogueQueryTest_Formation::RunTest(FRogueQueryInstance& QueryInstance) const
{
	if (QueryInstance.Owner == nullptr)
	{
		return;
	}

	TArray<FNavLocation> SourcePosition;
	SourcePosition.Add(FNavLocation(QueryInstance.SourceLocation));
	FRogueQueryHelpers::ProjectToNavigation(QueryInstance, SourcePosition, ProjectionData, FRogueQueryHelpers::ERogueTraceMode::Keep);
	if(SourcePosition.IsValidIndex(0))
	{
		QueryInstance.SourceLocation = SourcePosition[0];
	}
	
	for (int i = 0; i < QueryInstance.Items.Num(); ++i)
	{
		TArray<FNavLocation> PointsToProject;
		PointsToProject.Add(FNavLocation(QueryInstance.Items[i].Location));
		FRogueQueryHelpers::ProjectAndFilterNavPoints(PointsToProject, QueryInstance, ProjectionData);
		
		if(!IsValidSourcePoint(PointsToProject))
		{
			// Create grid around formation position
			TArray<FNavLocation> GridPoints;
			FVector2D GridData = FVector2D(0.f, 0.f);
			CreateItemGrid(GridPoints, QueryInstance, QueryInstance.Items[i].Location, GridData);

			// Remove non navigation points
			FRogueQueryHelpers::ProjectAndFilterNavPoints(GridPoints, QueryInstance, ProjectionData);

			// Create query items
			TArray<FRogueQueryItem> GridItems;
			CreateGridQueryItems(GridPoints, GridItems);

			// Score each grid location
			float MaxScore = 0.f;
			float MinScore = MAX_FLT;
			for (int j = 0; j < GridItems.Num(); ++j)
			{
				float Score = 0.f;
				const float MinDistance = GridData.X;
				const float MaxDistance = GridData.Y;

				check(!FMath::IsNearlyEqual(MinDistance, MaxDistance));
				
				// Calculate distance to source position
				const float DistanceToSource = static_cast<float>(FVector::Distance(QueryInstance.Items[i].Location, GridItems[j].Location));
				const float Weight = 1.0f - ((DistanceToSource - MinDistance) / (MaxDistance - MinDistance));
								
				if (DistanceToSource > MinDistance)
				{
					// Created weighted score for distance to other formation positions
					float WeightFormationPos = 0.f;
					for (int k = 0; k < QueryInstance.Items.Num(); ++k)
					{
						const float DistanceToPosition = static_cast<float>(FVector::Distance(QueryInstance.Items[k].Location, GridItems[j].Location));
						if(DistanceToPosition > MinDistance * 2.f)
						{
							WeightFormationPos += ((DistanceToPosition - MinDistance) / (MaxDistance - MinDistance));
							//Score += DistanceToPosition;
						}
						else
						{
							WeightFormationPos += -1.f;
						}
					}

					// Create weighted score for distance of navigation from formation center
					float WeightNav = 0.f;
					const float DistanceToNav = FRogueQueryHelpers::GetPathDistance(QueryInstance.Owner, QueryInstance.World, QueryInstance.SourceLocation, GridItems[j].Location, ProjectionData.NavigationFilter);
					if(DistanceToNav > MinDistance * 2.f)
					{
						WeightNav = 1.f - ((DistanceToNav - MinDistance) / (MaxDistance - MinDistance));
					}
					else
					{
						WeightNav = 1.f;
					}

					ensure(QueryInstance.Items.Num() > 0);
					WeightFormationPos /= QueryInstance.Items.Num();
					const float FinalWeight = (Weight + WeightFormationPos + WeightNav) / 3.f;
					//Score /= QueryInstance.Items.Num();		

					//Score += DistanceToSource;
					Score = (FinalWeight * MinDistance) - Score / MinDistance;			
				}

				// Ensure the score is non-negative
				if (Score < 0.0f)
				{
					Score = 0.0f;
				}					

				if(Score > MaxScore)
				{
					MaxScore = Score;
				}				

				if(Score < MinScore)
				{
					MinScore = Score;
				}

				GridItems[j].Score = Score;				
			}

			// Normalise scores
			for (int n = 0; n < GridItems.Num(); ++n)
			{
				GridItems[n].Score = (GridItems[n].Score - MinScore) / (MaxScore - MinScore);
			}

			FRogueQueryItem BestItem;
			float BestScore = 0.f;
			for (int s = 0; s < GridItems.Num(); ++s)
			{
				/*if(i == 0)
				{
					const FVector Location = GridItems[s].Location;
					UWorld* World = QueryInstance.World;
					const float Score = GridItems[s].Score;
					AsyncTask(ENamedThreads::GameThread, [this, Location, World, Score]()
					{
						if(World != nullptr)
						{
							FActorSpawnParameters SpawnParams;
							SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
							if(AActor* NewActor = World->SpawnActor<ARogueQueryDebugActor>(ARogueQueryDebugActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams))
							{
								if(ARogueQueryDebugActor* QueryActor = Cast<ARogueQueryDebugActor>(NewActor))
								{
									QueryActor->SetScale(0.5f);
									QueryActor->SetColor(FMath::Lerp(FLinearColor::Red, FLinearColor::Green, Score));
									UE_LOG(LogRogueQuery, Log, TEXT("[%s] Grid Point Score: %f"), *GetClass()->GetName(), Score);
								}
							}
						}
					});
					
				}*/
				
				if(GridItems[s].Score > BestScore)
				{
					BestScore = GridItems[s].Score;
					BestItem = GridItems[s];
				}
			}
			
			QueryInstance.Items[i].Score = BestScore;
			QueryInstance.Items[i].Location = BestItem.Location;
		}
		else
		{
			QueryInstance.Items[i].Score = 1.f;
		}
	}

	// Project final points to navigation
	for (int j = 0; j < QueryInstance.Items.Num(); ++j)
	{
		TArray<FNavLocation> PointsToProject;
		PointsToProject.Add(FNavLocation(QueryInstance.Items[j].Location));
		FRogueQueryHelpers::ProjectToNavigation(QueryInstance, PointsToProject, ProjectionData, FRogueQueryHelpers::ERogueTraceMode::Keep);

		if(PointsToProject.IsValidIndex(0))
		{
			QueryInstance.Items[j].Location = PointsToProject[0].Location;
		}
	}
}

/*void URogueQueryTest_Formation::ProjectToNavigation(const FRogueQueryInstance& QueryInstance, TArray<FNavLocation>& PointsToProject, const FRogueQueryHelpers::ERogueTraceMode Mode) const
{	
	if(const ANavigationData* QueryNavData = FRogueQueryHelpers::FindNavigationDataForQuery(QueryInstance.Owner, QueryInstance.World))
	{
		FRogueQueryHelpers::RunNavProjection(*QueryNavData, *QueryInstance.Owner, FRogueQueryTraceData(), PointsToProject, Mode);
	}	

#if WITH_RECAST	
	PathToItem.BindData(QueryInstance.Owner, QueryInstance.QueryId);
	ScanRangeMultiplier.BindData(QueryInstance.Owner, QueryInstance.QueryId);
	const float RangeMultiplierValue = ScanRangeMultiplier.GetValue();

	const ANavigationData* QueryNavData = FRogueQueryHelpers::FindNavigationDataForQuery(QueryInstance.Owner, QueryInstance.World);
	const ARecastNavMesh* NavMeshData = Cast<const ARecastNavMesh>(QueryNavData);
	if (NavMeshData == nullptr || QueryInstance.Owner == nullptr)
	{
		return;
	}

	const FSharedConstNavQueryFilter NavQueryFilter = ProjectionData.NavigationFilter ? UNavigationQueryFilter::GetQueryFilter(*NavMeshData, QueryInstance.Owner, ProjectionData.NavigationFilter) : NavMeshData->GetDefaultQueryFilter();
	if (!NavQueryFilter.IsValid())
	{
		UE_LOG(LogRogueQuery, Error, TEXT("Query:%d can't obtain navigation filter! NavData:%s FilterClass:%s"),
			QueryInstance.QueryId, *GetNameSafe(NavMeshData), *GetNameSafe(ProjectionData.NavigationFilter.Get()));
		return;
	}

	FSharedNavQueryFilter NavigationFilterCopy = NavQueryFilter->GetCopy();
	if (NavigationFilterCopy.IsValid())
	{
		TArray<NavNodeRef> Polys;
		TArray<FNavLocation> HitLocations;
		const FVector ProjectionExtent(ProjectionData.ExtentX, ProjectionData.ExtentX, (ProjectionData.ProjectDown + ProjectionData.ProjectUp) / 2);

		for (int32 ContextIdx = 0; ContextIdx < PointsToProject.Num(); ContextIdx++)
		{
			FVector::FReal CollectDistanceSq = 0.0f;
			for (int32 Idx = 0; Idx < PointsToProject.Num(); Idx++)
			{
				const FVector::FReal TestDistanceSq = FVector::DistSquared(PointsToProject[Idx].Location, QueryInstance.SourceLocation);
				CollectDistanceSq = FMath::Max(CollectDistanceSq, TestDistanceSq);
			}

			const FVector::FReal MaxPathDistance = FMath::Sqrt(CollectDistanceSq) * RangeMultiplierValue;

			Polys.Reset();

			FRecastDebugPathfindingData NodePoolData;
			NodePoolData.Flags = ERecastDebugPathfindingFlags::Basic;

			NavMeshData->GetPolysWithinPathingDistance(QueryInstance.SourceLocation, MaxPathDistance, Polys, NavigationFilterCopy, nullptr, &NodePoolData);

			for (int32 Idx = PointsToProject.Num() - 1; Idx >= 0; Idx--)
			{
				bool bHasPath = FRogueQueryHelpers::HasPath(NodePoolData, PointsToProject[Idx].NodeRef);
				if (!bHasPath && PointsToProject[Idx].NodeRef != INVALID_NAVNODEREF)
				{
					// try projecting it again, maybe it will match valid poly on different height
					HitLocations.Reset();
					FVector TestPt(PointsToProject[Idx].Location.X, PointsToProject[Idx].Location.Y, QueryInstance.SourceLocation.Z);

					NavMeshData->ProjectPointMulti(TestPt, HitLocations, ProjectionExtent, TestPt.Z - ProjectionData.ProjectDown, TestPt.Z + ProjectionData.ProjectUp, NavigationFilterCopy, nullptr);
					for (int32 HitIdx = 0; HitIdx < HitLocations.Num(); HitIdx++)
					{
						const bool bHasPathTest = FRogueQueryHelpers::HasPath(NodePoolData, HitLocations[HitIdx].NodeRef);
						if (bHasPathTest)
						{
							PointsToProject[Idx] = HitLocations[HitIdx];
							PointsToProject[Idx].Location.Z += ProjectionData.PostProjectionVerticalOffset;
							bHasPath = true;
							break;
						}
					}
				}

				if (!bHasPath)
				{
					GeneratedPoints.RemoveAt(Idx);
				}
			}
		}
	}
#endif
}*/

void URogueQueryTest_Formation::CreateItemGrid(TArray<FNavLocation>& GridPoints, const FRogueQueryInstance& QueryInstance, const FVector& Position, FVector2D& GridData)
{
	if(const URogueQueryGenerator_Formation* FormationGenerator = Cast<URogueQueryGenerator_Formation>(QueryInstance.Generator))
	{		
		const float SpaceBetween = FormationGenerator->SpaceBetween * 0.25f;
		const int32 ItemCount = FPlatformMath::TruncToInt((FormationGenerator->SpaceBetween * 5.f / SpaceBetween) + 1);
		const int32 ItemCountHalf = ItemCount / 2;
		GridData.X = FormationGenerator->SpaceBetween;
		GridData.Y = ItemCount * SpaceBetween * 1.5;
		GridPoints.Reserve(ItemCount * ItemCount);

		for (int32 IndexX = 0; IndexX < ItemCount; ++IndexX)
		{
			for (int32 IndexY = 0; IndexY < ItemCount; ++IndexY)
			{
				FNavLocation NewItem = FNavLocation(Position - FVector(SpaceBetween * (IndexX - ItemCountHalf), SpaceBetween * (IndexY - ItemCountHalf), 0));
				GridPoints.Add(NewItem);
			}
		}
	}
	
}

void URogueQueryTest_Formation::CreateGridQueryItems(TArray<FNavLocation>& GridPoints,
	TArray<FRogueQueryItem>& GridItems)
{
	for (int i = 0; i < GridPoints.Num(); ++i)
	{
		GridItems.Add(FRogueQueryItem(GridPoints[i]));
	}
}
