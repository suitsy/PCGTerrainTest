// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Queries/RogueQueryTraceHelpers.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "Algo/RemoveIf.h"
#include "NavMesh/RecastNavMesh.h"

template<>
void FRogueQueryHelpers::FRogueQueryBatchTrace::DoSingleSourceMultiDestinations<ERogueQueryTraceShape::Line>(const FVector& Source, TArray<FNavLocation>& Points)
{
	FVector HitPos(FVector::ZeroVector);
	for (int32 Idx = Points.Num() - 1; Idx >= 0; Idx--)
	{
		const bool bHit = RunLineTrace(Source, Points[Idx].Location, HitPos);
		if (bHit)
		{
			Points[Idx] = FNavLocation(HitPos);
		}
		else if (TraceMode == ERogueTraceMode::Discard)
		{
			Points.RemoveAt(Idx, 1, false);
		}
	}
}

template<>
void FRogueQueryHelpers::FRogueQueryBatchTrace::DoSingleSourceMultiDestinations<ERogueQueryTraceShape::Box>(const FVector& Source, TArray<FNavLocation>& Points)
{
	FVector HitPos(FVector::ZeroVector);
	for (int32 Idx = Points.Num() - 1; Idx >= 0; Idx--)
	{
		const bool bHit = RunBoxTrace(Source, Points[Idx].Location, HitPos);
		if (bHit)
		{
			Points[Idx] = FNavLocation(HitPos);
		}
		else if (TraceMode == ERogueTraceMode::Discard)
		{
			Points.RemoveAt(Idx, 1, false);
		}
	}
}

template<>
void FRogueQueryHelpers::FRogueQueryBatchTrace::DoSingleSourceMultiDestinations<ERogueQueryTraceShape::Sphere>(const FVector& Source, TArray<FNavLocation>& Points)
{
	FVector HitPos(FVector::ZeroVector);
	for (int32 Idx = Points.Num() - 1; Idx >= 0; Idx--)
	{
		const bool bHit = RunSphereTrace(Source, Points[Idx].Location, HitPos);
		if (bHit)
		{
			Points[Idx] = FNavLocation(HitPos);
		}
		else if (TraceMode == ERogueTraceMode::Discard)
		{
			Points.RemoveAt(Idx, 1, false);
		}
	}
}

template<>
void FRogueQueryHelpers::FRogueQueryBatchTrace::DoSingleSourceMultiDestinations<ERogueQueryTraceShape::Capsule>(const FVector& Source, TArray<FNavLocation>& Points)
{
	FVector HitPos(FVector::ZeroVector);
	for (int32 Idx = Points.Num() - 1; Idx >= 0; Idx--)
	{
		const bool bHit = RunCapsuleTrace(Source, Points[Idx].Location, HitPos);
		if (bHit)
		{
			Points[Idx] = FNavLocation(HitPos);
		}
		else if (TraceMode == ERogueTraceMode::Discard)
		{
			Points.RemoveAt(Idx, 1, false);
		}
	}
}

template<>
void FRogueQueryHelpers::FRogueQueryBatchTrace::DoMultiSourceMultiDestinations2D<ERogueQueryTraceShape::Line>(const TArray<FRayStartEnd>& Rays, TArray<FNavLocation>& OutPoints)
{
	FVector HitPos(FVector::ZeroVector);

	for (int32 RayIndex = 0; RayIndex < Rays.Num(); ++RayIndex)
	{
		const FVector StartLocation = Rays[RayIndex].RayStart;
		// adjusted end to make this a 2D trace
		FVector EndLocation = Rays[RayIndex].RayEnd;
		EndLocation.Z = StartLocation.Z;
		const bool bHit = RunLineTrace(StartLocation, Rays[RayIndex].RayEnd, HitPos);
		if (bHit || TraceMode != ERogueTraceMode::Discard)
		{
			OutPoints.Add(FNavLocation(bHit ? HitPos : EndLocation));
		}
	}
}

template<>
void FRogueQueryHelpers::FRogueQueryBatchTrace::DoMultiSourceMultiDestinations2D<ERogueQueryTraceShape::Box>(const TArray<FRayStartEnd>& Rays, TArray<FNavLocation>& OutPoints)
{
	FVector HitPos(FVector::ZeroVector);

	for (int32 RayIndex = 0; RayIndex < Rays.Num(); ++RayIndex)
	{
		const FVector StartLocation = Rays[RayIndex].RayStart;
		// adjusted end to make this a 2D trace
		FVector EndLocation = Rays[RayIndex].RayEnd;
		EndLocation.Z = StartLocation.Z;
		const bool bHit = RunBoxTrace(StartLocation, Rays[RayIndex].RayEnd, HitPos);
		if (bHit || TraceMode != ERogueTraceMode::Discard)
		{
			OutPoints.Add(FNavLocation(bHit ? HitPos : EndLocation));
		}
	}
}

template<>
void FRogueQueryHelpers::FRogueQueryBatchTrace::DoMultiSourceMultiDestinations2D<ERogueQueryTraceShape::Sphere>(const TArray<FRayStartEnd>& Rays, TArray<FNavLocation>& OutPoints)
{
	FVector HitPos(FVector::ZeroVector);

	for (int32 RayIndex = 0; RayIndex < Rays.Num(); ++RayIndex)
	{
		const FVector StartLocation = Rays[RayIndex].RayStart;
		// adjusted end to make this a 2D trace
		FVector EndLocation = Rays[RayIndex].RayEnd;
		EndLocation.Z = StartLocation.Z;
		const bool bHit = RunSphereTrace(StartLocation, Rays[RayIndex].RayEnd, HitPos);
		if (bHit || TraceMode != ERogueTraceMode::Discard)
		{
			OutPoints.Add(FNavLocation(bHit ? HitPos : EndLocation));
		}
	}
}

template<>
void FRogueQueryHelpers::FRogueQueryBatchTrace::DoMultiSourceMultiDestinations2D<ERogueQueryTraceShape::Capsule>(const TArray<FRayStartEnd>& Rays, TArray<FNavLocation>& OutPoints)
{
	FVector HitPos(FVector::ZeroVector);

	for (int32 RayIndex = 0; RayIndex < Rays.Num(); ++RayIndex)
	{
		const FVector StartLocation = Rays[RayIndex].RayStart;
		// adjusted end to make this a 2D trace
		FVector EndLocation = Rays[RayIndex].RayEnd;
		EndLocation.Z = StartLocation.Z;
		const bool bHit = RunCapsuleTrace(StartLocation, Rays[RayIndex].RayEnd, HitPos);
		if (bHit || TraceMode != ERogueTraceMode::Discard)
		{
			OutPoints.Add(FNavLocation(bHit ? HitPos : EndLocation));
		}
	}
}

template<>
void FRogueQueryHelpers::FRogueQueryBatchTrace::DoProject<ERogueQueryTraceShape::Line>(TArray<FNavLocation>& PointsToProject, float StartOffsetZ, float EndOffsetZ, float HitOffsetZ)
{
	FVector HitPos(FVector::ZeroVector);
	for (int32 Idx = PointsToProject.Num() - 1; Idx >= 0; Idx--)
	{
		const bool bHit = RunLineTrace(PointsToProject[Idx].Location + FVector(0, 0, StartOffsetZ), PointsToProject[Idx].Location + FVector(0, 0, EndOffsetZ), HitPos);

		if (bHit)
		{
			PointsToProject[Idx] = FNavLocation(HitPos + FVector(0, 0, HitOffsetZ));
		}
		else if (TraceMode == ERogueTraceMode::Discard)
		{
			PointsToProject.RemoveAt(Idx, 1, false);
		}

		if (TraceHits.IsValidIndex(Idx))
		{
			TraceHits[Idx] = bHit ? 1 : 0;
		}
	}
}

template<>
void FRogueQueryHelpers::FRogueQueryBatchTrace::DoProject<ERogueQueryTraceShape::Box>(TArray<FNavLocation>& PointsToProject, float StartOffsetZ, float EndOffsetZ, float HitOffsetZ)
{
	FVector HitPos(FVector::ZeroVector);
	for (int32 Idx = PointsToProject.Num() - 1; Idx >= 0; Idx--)
	{
		const bool bHit = RunBoxTrace(PointsToProject[Idx].Location + FVector(0, 0, StartOffsetZ), PointsToProject[Idx].Location + FVector(0, 0, EndOffsetZ), HitPos);
		if (bHit)
		{
			PointsToProject[Idx] = FNavLocation(HitPos + FVector(0, 0, HitOffsetZ));
		}
		else if (TraceMode == ERogueTraceMode::Discard)
		{
			PointsToProject.RemoveAt(Idx, 1, false);
		}

		if (TraceHits.IsValidIndex(Idx))
		{
			TraceHits[Idx] = bHit ? 1 : 0;
		}
	}
}

template<>
void FRogueQueryHelpers::FRogueQueryBatchTrace::DoProject<ERogueQueryTraceShape::Sphere>(TArray<FNavLocation>& PointsToProject, float StartOffsetZ, float EndOffsetZ, float HitOffsetZ)
{
	FVector HitPos(FVector::ZeroVector);
	for (int32 Idx = PointsToProject.Num() - 1; Idx >= 0; Idx--)
	{
		const bool bHit = RunSphereTrace(PointsToProject[Idx].Location + FVector(0, 0, StartOffsetZ), PointsToProject[Idx].Location + FVector(0, 0, EndOffsetZ), HitPos);
		if (bHit)
		{
			PointsToProject[Idx] = FNavLocation(HitPos + FVector(0, 0, HitOffsetZ));
		}
		else if (TraceMode == ERogueTraceMode::Discard)
		{
			PointsToProject.RemoveAt(Idx, 1, false);
		}

		if (TraceHits.IsValidIndex(Idx))
		{
			TraceHits[Idx] = bHit ? 1 : 0;
		}
	}
}

template<>
void FRogueQueryHelpers::FRogueQueryBatchTrace::DoProject<ERogueQueryTraceShape::Capsule>(TArray<FNavLocation>& PointsToProject, float StartOffsetZ, float EndOffsetZ, float HitOffsetZ)
{
	FVector HitPos(FVector::ZeroVector);
	for (int32 Idx = PointsToProject.Num() - 1; Idx >= 0; Idx--)
	{
		const bool bHit = RunCapsuleTrace(PointsToProject[Idx].Location + FVector(0, 0, StartOffsetZ), PointsToProject[Idx].Location + FVector(0, 0, EndOffsetZ), HitPos);
		if (bHit)
		{
			PointsToProject[Idx] = FNavLocation(HitPos + FVector(0, 0, HitOffsetZ));
		}
		else if (TraceMode == ERogueTraceMode::Discard)
		{
			PointsToProject.RemoveAt(Idx, 1, false);
		}

		if (TraceHits.IsValidIndex(Idx))
		{
			TraceHits[Idx] = bHit ? 1 : 0;
		}
	}
}


ANavigationData* FRogueQueryHelpers::FindNavigationDataForQuery(const TObjectPtr<UObject> Owner, const UWorld* World)
{
	if(World == nullptr)
	{
		return nullptr;
	}
	const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (NavSys == nullptr)
	{
		return nullptr;
	}

	// try to match navigation agent for owner
	if (const INavAgentInterface* NavAgent = Owner != nullptr ? Cast<INavAgentInterface>(Owner.Get()) : nullptr)
	{
		const FNavAgentProperties& NavAgentProps = NavAgent->GetNavAgentPropertiesRef();
		if (NavAgentProps.IsValid() || NavAgentProps.PreferredNavData.IsValid())
		{
			return NavSys->GetNavDataForProps(NavAgentProps, NavAgent->GetNavAgentLocation());
		}
	}

	return NavSys->GetDefaultNavDataInstance();
}

void FRogueQueryHelpers::ProjectToNavigation(const FRogueQueryInstance& QueryInstance,
	TArray<FNavLocation>& PointsToProject, const FRogueQueryTraceData& ProjectionData, const ERogueTraceMode Mode)
{
	UObject* DataOwner = QueryInstance.Owner.Get();	
	if(DataOwner == nullptr || QueryInstance.World == nullptr)
	{
		return;
	}

	const ANavigationData* QueryNavData = FindNavigationDataForQuery(DataOwner, QueryInstance.World);
	const ARecastNavMesh* NavMeshData = Cast<const ARecastNavMesh>(QueryNavData);	
	if (NavMeshData == nullptr || DataOwner == nullptr)
	{
		return;
	}
	
	if (ProjectionData.TraceMode != ERogueQueryTrace::None)
	{
		if(QueryNavData != nullptr)
		{
			if (ProjectionData.TraceMode == ERogueQueryTrace::Navigation)
			{
				RunNavProjection(*QueryNavData, *DataOwner, ProjectionData, PointsToProject, Mode);
			}
			if (ProjectionData.TraceMode == ERogueQueryTrace::GeometryByProfile)
			{
				RunPhysProjection(QueryInstance.World, ProjectionData, PointsToProject);
			}
		}
	}
}

void FRogueQueryHelpers::RunNavProjection(const ANavigationData& NavData, const UObject& Querier, const FRogueQueryTraceData& TraceData,
	TArray<FNavLocation>& PointsToProject, ERogueTraceMode TraceMode)
{
	FSharedConstNavQueryFilter NavigationFilter = UNavigationQueryFilter::GetQueryFilter(NavData, &Querier, TraceData.NavigationFilter);
	TArray<FNavigationProjectionWork> Workload;
	Workload.Reserve(PointsToProject.Num());

	if (TraceData.ProjectDown == TraceData.ProjectUp)
	{
		for (const auto& Point : PointsToProject)
		{
			Workload.Add(FNavigationProjectionWork(Point.Location));
		}
	}
	else
	{
		const FVector VerticalOffset = FVector(0, 0, (TraceData.ProjectUp - TraceData.ProjectDown) / 2);
		for (const auto& Point : PointsToProject)
		{
			Workload.Add(FNavigationProjectionWork(Point.Location + VerticalOffset));
		}
	}

	const FVector ProjectionExtent(TraceData.ExtentX, TraceData.ExtentX, (TraceData.ProjectDown + TraceData.ProjectUp) / 2);
	NavData.BatchProjectPoints(Workload, ProjectionExtent, NavigationFilter);

	for (int32 Idx = Workload.Num() - 1; Idx >= 0; Idx--)
	{
		if (Workload[Idx].bResult)
		{
			PointsToProject[Idx] = Workload[Idx].OutLocation;
			PointsToProject[Idx].Location.Z += TraceData.PostProjectionVerticalOffset;
		}
	}

	if (TraceMode == ERogueTraceMode::Discard)
	{
		const FNavLocation* PointsBegin = PointsToProject.GetData();
		int32 NewNum = Algo::StableRemoveIf(PointsToProject, [&Workload, PointsBegin](FNavLocation& Point)
		{
			return !Workload[IntCastChecked<int32>(&Point - PointsBegin)].bResult;
		});
		const bool bAllowShrinking = false;
		PointsToProject.SetNum(NewNum, bAllowShrinking);
	}
}

void FRogueQueryHelpers::RunPhysProjection(UWorld* World, const FRogueQueryTraceData& TraceData, TArray<FNavLocation>& PointsToProject,
	const ERogueTraceMode TraceMode)
{
	FVector TraceExtent(TraceData.ExtentX, TraceData.ExtentY, TraceData.ExtentZ);

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(EnvQueryTrace), TraceData.bTraceComplex);

	FRogueQueryBatchTrace BatchOb(World, TraceData, TraceParams, TraceExtent, TraceMode);

	switch (TraceData.TraceShape)
	{
		case ERogueQueryTraceShape::Line:
			BatchOb.DoProject<ERogueQueryTraceShape::Line>(PointsToProject, TraceData.ProjectUp, -TraceData.ProjectDown, TraceData.PostProjectionVerticalOffset);
			break;

		case ERogueQueryTraceShape::Sphere:
			BatchOb.DoProject<ERogueQueryTraceShape::Sphere>(PointsToProject, TraceData.ProjectUp, -TraceData.ProjectDown, TraceData.PostProjectionVerticalOffset);
			break;

		case ERogueQueryTraceShape::Capsule:
			BatchOb.DoProject<ERogueQueryTraceShape::Capsule>(PointsToProject, TraceData.ProjectUp, -TraceData.ProjectDown, TraceData.PostProjectionVerticalOffset);
			break;

		case ERogueQueryTraceShape::Box:
			BatchOb.DoProject<ERogueQueryTraceShape::Box>(PointsToProject, TraceData.ProjectUp, -TraceData.ProjectDown, TraceData.PostProjectionVerticalOffset);
			break;

		default:
			break;
	}
}

float FRogueQueryHelpers::GetPathDistance(UObject* QueryOwner, UWorld* World, const FVector& Start, const FVector& End, TSubclassOf<UNavigationQueryFilter> FilterClass)
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (NavSys == nullptr || QueryOwner == nullptr)
	{
		return MAX_FLT;
	}
	
	const ANavigationData* NavData = FindNavigationData(*NavSys, QueryOwner);
	if (NavData == nullptr)
	{
		return MAX_FLT;
	}
	
	const FSharedConstNavQueryFilter NavFilter = UNavigationQueryFilter::GetQueryFilter(*NavData, QueryOwner, FilterClass);
	FPathFindingQuery Query(QueryOwner, *NavData, Start, End, NavFilter);
	Query.SetAllowPartialPaths(false);

	const FPathFindingResult Result = NavSys->FindPathSync(Query, EPathFindingMode::Regular);
	if(Result.IsSuccessful())
	{
		return Result.Path->GetLength();
	}

	return MAX_FLT;
}

ANavigationData* FRogueQueryHelpers::FindNavigationData(UNavigationSystemV1& NavSys, UObject* Owner)
{
	if(const INavAgentInterface* NavAgent = Cast<INavAgentInterface>(Owner))
	{
		return NavSys.GetNavDataForProps(NavAgent->GetNavAgentPropertiesRef(), NavAgent->GetNavAgentLocation());
	}

	return NavSys.GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
}

void FRogueQueryHelpers::ProjectAndFilterNavPoints(TArray<FNavLocation>& Points, const FRogueQueryInstance& QueryInstance,
    const FRogueQueryTraceData& ProjectionData, const float RangeMultiplierValue)
{
	UObject* DataOwner = QueryInstance.Owner.Get();
	if(DataOwner == nullptr || QueryInstance.World == nullptr)
	{
		return;
	}

	const ANavigationData* QueryNavData = FindNavigationDataForQuery(DataOwner, QueryInstance.World);
	const ARecastNavMesh* NavMeshData = Cast<const ARecastNavMesh>(QueryNavData);
	if (NavMeshData == nullptr || DataOwner == nullptr)
	{
		return;
	}

	if (ProjectionData.TraceMode != ERogueQueryTrace::None)
	{
		if(QueryNavData != nullptr)
		{
			if (ProjectionData.TraceMode == ERogueQueryTrace::Navigation)
			{
				RunNavProjection(*QueryNavData, *DataOwner, ProjectionData, Points);
			}
			if (ProjectionData.TraceMode == ERogueQueryTrace::GeometryByProfile)
			{
				RunPhysProjection(QueryInstance.World, ProjectionData, Points);
			}
		}
	}

	ProjectToNavigation(QueryInstance, Points, ProjectionData, ERogueTraceMode::Keep);

	if(Points.Num() <= 1)
	{
		return;
	}

	#if WITH_RECAST
	const FSharedConstNavQueryFilter NavQueryFilter = ProjectionData.NavigationFilter ? UNavigationQueryFilter::GetQueryFilter(*NavMeshData, DataOwner, ProjectionData.NavigationFilter) : NavMeshData->GetDefaultQueryFilter();
	if (!NavQueryFilter.IsValid())
	{
		UE_LOG(LogRogueQuery, Error, TEXT("Query:%d can't obtain navigation filter! NavData:%s FilterClass:%s"),
			QueryInstance.QueryId, *GetNameSafe(NavMeshData), *GetNameSafe(ProjectionData.NavigationFilter.Get()));
		return;
	}

	const FSharedNavQueryFilter NavigationFilterCopy = NavQueryFilter->GetCopy();
	if (NavigationFilterCopy.IsValid())
	{
		TArray<NavNodeRef> Polys;
		TArray<FNavLocation> HitLocations;
		const FVector ProjectionExtent(ProjectionData.ExtentX, ProjectionData.ExtentX, (ProjectionData.ProjectDown + ProjectionData.ProjectUp) / 2);
		
		FVector::FReal CollectDistanceSq = 0.0f;
		for (int32 Idx = 0; Idx < Points.Num(); Idx++)
		{
			const FVector::FReal TestDistanceSq = FVector::DistSquared(Points[Idx].Location, QueryInstance.SourceLocation);
			CollectDistanceSq = FMath::Max(CollectDistanceSq, TestDistanceSq);
		}

		const FVector::FReal MaxPathDistance = FMath::Sqrt(CollectDistanceSq) * RangeMultiplierValue;

		Polys.Reset();

		FRecastDebugPathfindingData NodePoolData;
		NodePoolData.Flags = ERecastDebugPathfindingFlags::Basic;

		NavMeshData->GetPolysWithinPathingDistance(QueryInstance.SourceLocation, MaxPathDistance, Polys, NavigationFilterCopy, nullptr, &NodePoolData);

		for (int32 Idx = Points.Num() - 1; Idx >= 0; Idx--)
		{
			bool bHasPath = HasPath(NodePoolData, Points[Idx].NodeRef);
			if (!bHasPath && Points[Idx].NodeRef != INVALID_NAVNODEREF)
			{
				// try projecting it again, maybe it will match valid poly on different height
				HitLocations.Reset();
				FVector TestPt(Points[Idx].Location.X, Points[Idx].Location.Y, QueryInstance.SourceLocation.Z);

				NavMeshData->ProjectPointMulti(TestPt, HitLocations, ProjectionExtent, TestPt.Z - ProjectionData.ProjectDown, TestPt.Z + ProjectionData.ProjectUp, NavigationFilterCopy, nullptr);
				for (int32 HitIdx = 0; HitIdx < HitLocations.Num(); HitIdx++)
				{
					if (HasPath(NodePoolData, HitLocations[HitIdx].NodeRef))
					{
						Points[Idx] = HitLocations[HitIdx];
						Points[Idx].Location.Z += ProjectionData.PostProjectionVerticalOffset;
						bHasPath = true;
						break;
					}
				}
			}

			if (!bHasPath)
			{
				Points.RemoveAt(Idx);
			}
		}
		
	}
#endif
}

#if WITH_RECAST
bool FRogueQueryHelpers::HasPath(const FRecastDebugPathfindingData& NodePool, const NavNodeRef& NodeRef)
{
	FRecastDebugPathfindingNode SearchKey(NodeRef);
	const FRecastDebugPathfindingNode* MyNode = NodePool.Nodes.Find(SearchKey);
	return MyNode != nullptr;
}
#endif
