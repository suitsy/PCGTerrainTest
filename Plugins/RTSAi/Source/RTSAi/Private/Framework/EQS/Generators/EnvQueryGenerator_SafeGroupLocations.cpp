// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/EQS/Generators/EnvQueryGenerator_SafeGroupLocations.h"
#include "NavigationSystem.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"

UEnvQueryGenerator_SafeGroupLocations::UEnvQueryGenerator_SafeGroupLocations(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ItemType = UEnvQueryItemType_Point::StaticClass();
	Context = UEnvQueryContext_Querier::StaticClass();
	OffsetX.DefaultValue = 0.f;
	OffsetY.DefaultValue = 1.f;
	PositionsAmount.DefaultValue = 4.f;

	Direction.DirMode = EEnvDirection::Rotation;
	Direction.Rotation = UEnvQueryContext_Querier::StaticClass();
	TraceData.SetGeometryOnly();
}

void UEnvQueryGenerator_SafeGroupLocations::GenerateItems(FEnvQueryInstance& QueryInstance) const
{	
	if(const UObject* QueryOwner = QueryInstance.Owner.Get())
	{
		OffsetX.BindData(QueryOwner, QueryInstance.QueryID);
		const float X = OffsetX.GetValue();

		OffsetY.BindData(QueryOwner, QueryInstance.QueryID);
		const float Y = OffsetY.GetValue();
	
		PositionsAmount.BindData(QueryOwner, QueryInstance.QueryID);		
		const int32 Amount = PositionsAmount.GetValue();
		
		SpaceBetween.BindData(QueryOwner, QueryInstance.QueryID);
		const float PointSpacing = SpaceBetween.GetValue();

		TArray<FNavLocation> ItemPoints;
		TArray<FVector> ContextLocations;
		QueryInstance.PrepareContext(Context, ContextLocations);
		TArray<FRotator> Rot;
		QueryInstance.PrepareContext(Direction.Rotation, Rot);

		for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
		{
			if(ContextLocations.IsValidIndex(ContextIndex) && Rot.IsValidIndex(ContextIndex))
			{			
				FVector2D Spacings = FVector2D(0.f, 0.f);
				TArray<FVector> PrevPoint;
				PrevPoint.Init(ContextLocations[ContextIndex], 2);
				const bool bIsOddCount = Amount % 2 != 0;
				const int32 PosAmount = bIsOddCount ? Amount + 1 : Amount;
			
				TArray<FNavLocation> GridPoints;
				const int32 GridItemCount = FPlatformMath::TruncToInt(((PointSpacing * 2.f) * (PosAmount * 2.f) / PointSpacing) + 1);
				GenerateGridPoints(GridPoints, QueryInstance, PointSpacing, GridItemCount);			
			
				for (int PosIndex = 0; PosIndex < PosAmount; ++PosIndex)
				{
					if(PrevPoint.IsValidIndex(0) && PrevPoint.IsValidIndex(1))
					{
						// Calculate formation position source point and convert to grid point (nav mesh point)
						FNavLocation Point;
						GenerateSourcePoint(Point, ContextLocations[ContextIndex], Rot[ContextIndex], PosIndex, FVector(X, Y, 0.f), Spacings, PointSpacing, bIsOddCount);
						GetNearestGridPoint(Point, GridPoints);				
				
						// Check obstacle to point 
						if(TraceToPosition(QueryInstance, ContextLocations[ContextIndex], Point))
						{
							// LOS blocked, trace path to point at position distance
							GetPathPointAtDistance(Point, QueryInstance, PosIndex % 2 != 0 ? PrevPoint[0] : PrevPoint[1], PosIndex, PointSpacing * 1.5f);
							GetNearestGridPoint(Point, GridPoints);
						}
					
						// Assign previous position				
						PosIndex % 2 != 0 ? PrevPoint[0] = Point : PrevPoint[1] = Point;
				
						// Add final point
						ItemPoints.Add(Point);
					}
				}
			}

		
			/*if(ContextLocations.IsValidIndex(0) && Rot.IsValidIndex(0))
			{
				TArray<FNavLocation> GridPoints;
				GenerateSourcePoints(SourcePoints, QueryInstance, PointSpacing, ContextLocations[0], Rot[0], TotalSpacing);
				GenerateGridPoints(GridPoints, QueryInstance, PointSpacing, TotalSpacing);			
				ValidateSourcePoints(SourcePoints, GridPoints, QueryInstance, ContextLocations[0]);
			}*/
		
			ProjectAndFilterNavPoints(ItemPoints, QueryInstance);
			StoreNavPoints(ItemPoints, QueryInstance);
		}
	}
}

void UEnvQueryGenerator_SafeGroupLocations::GenerateGridPoints(TArray<FNavLocation>& GridPoints, FEnvQueryInstance& QueryInstance,
	const float Spacing, const int32 ItemCount) const
{	
	const int32 ItemCountHalf = ItemCount / 2;
	const float GridSpacing = Spacing * 0.5f;

	TArray<FVector> ContextLocations;
	QueryInstance.PrepareContext(Context, ContextLocations);	
	
	GridPoints.Reserve(ItemCount * ItemCount * ContextLocations.Num());

	for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
	{
		for (int32 IndexX = 0; IndexX < ItemCount; ++IndexX)
		{
			for (int32 IndexY = 0; IndexY < ItemCount; ++IndexY)
			{
				const FNavLocation TestPoint = FNavLocation(ContextLocations[ContextIndex] - FVector(GridSpacing * (IndexX - ItemCountHalf), GridSpacing * (IndexY - ItemCountHalf), 0));
				GridPoints.Add(TestPoint);
			}
		}
	}	

	ProjectAndFilterNavPoints(GridPoints, QueryInstance);

	/*for (int i = 0; i < GridPoints.Num(); ++i)
	{
		DrawDebugSphere(QueryInstance.World, GridPoints[i].Location, 10.f, 8, FColor::Black, false, 7.f, 0, 1.f);
	}*/
}

void UEnvQueryGenerator_SafeGroupLocations::GetNearestGridPoint(FNavLocation& Point, TArray<FNavLocation>& GridPoints) const
{
	float LowestDist = MAX_FLT;
	FNavLocation NearPoint;

	for (const FNavLocation& GridPoint : GridPoints)
	{
		// Calculate the cost of the path to the current closest point
		const float Distance = (GridPoint.Location - Point.Location).Length();

		// Check if the current point has a lower cost than the lowest found so far
		if (Distance < LowestDist)
		{
			LowestDist = Distance;
			NearPoint = GridPoint;
		}
	}

	Point = FNavLocation(NearPoint);
}

void UEnvQueryGenerator_SafeGroupLocations::GenerateSourcePoint(FNavLocation& Point, const FVector& ContextPosition, const FRotator& Rotation, const int PosIndex, FVector LocalOffset, FVector2D& Spacings, const float PointSpacing, const bool bIsOddCount) const
{
	if(PosIndex % 2 != 0)
	{
		if(Spacings.X == 0.f && !bIsOddCount)
		{
			Spacings.X += PointSpacing;
		}				
	
		LocalOffset *= Spacings.X;
		LocalOffset.Y = LocalOffset.Y * -1;
		Spacings.X += PointSpacing;
	}
	else
	{
		if(Spacings.Y == 0.f && !bIsOddCount)
		{
			Spacings.Y += PointSpacing;
		}				
	
		LocalOffset *= Spacings.Y;
		Spacings.Y += PointSpacing;
	}	

	LocalOffset = Rotation.RotateVector(LocalOffset);
	Point = FNavLocation(ContextPosition + LocalOffset);	
}

bool UEnvQueryGenerator_SafeGroupLocations::TraceToPosition(FEnvQueryInstance& QueryInstance, const FVector& ContextPosition, const FVector& TargetPosition) const
{
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(EnvQueryTrace), TraceData.bTraceComplex);

	AActor* IgnoredActors = nullptr;
	/*if (QueryInstance.PrepareContext(Context, IgnoredActors))
	{
		TraceParams.AddIgnoredActors(IgnoredActors);
	}*/

	const ECollisionChannel TraceCollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceData.TraceChannel);
	const FVector TraceExtent(TraceData.ExtentX, TraceData.ExtentY, TraceData.ExtentZ);
	FRunTraceSignature TraceFunc;
	switch (TraceData.TraceShape)
	{
		case EEnvTraceShape::Line:
			TraceFunc.BindUObject(const_cast<UEnvQueryGenerator_SafeGroupLocations*>(this), &UEnvQueryGenerator_SafeGroupLocations::RunLineTraceTo);
			break;

		case EEnvTraceShape::Box:
			TraceFunc.BindUObject(const_cast<UEnvQueryGenerator_SafeGroupLocations*>(this), &UEnvQueryGenerator_SafeGroupLocations::RunBoxTraceTo);
			break;

		case EEnvTraceShape::Sphere:
			TraceFunc.BindUObject(const_cast<UEnvQueryGenerator_SafeGroupLocations*>(this), &UEnvQueryGenerator_SafeGroupLocations::RunSphereTraceTo);
			break;

		case EEnvTraceShape::Capsule:
			TraceFunc.BindUObject(const_cast<UEnvQueryGenerator_SafeGroupLocations*>(this), &UEnvQueryGenerator_SafeGroupLocations::RunCapsuleTraceTo);
			break;

		default:
			return false;
	}

	return TraceFunc.Execute(TargetPosition, ContextPosition, IgnoredActors, QueryInstance.World, TraceCollisionChannel, TraceParams, TraceExtent);
}

void UEnvQueryGenerator_SafeGroupLocations::GetPathPointAtDistance(FNavLocation& Point, FEnvQueryInstance& QueryInstance, const FVector& ContextPosition, const int PosIndex, const float PathDistance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(QueryInstance.World);
	if (NavSys == nullptr || QueryOwner == nullptr)
	{
		return;
	}

	const ANavigationData* NavData = FindNavigationData(*NavSys, QueryOwner);
	if (NavData == nullptr)
	{
		return;
	}

	const FSharedConstNavQueryFilter NavFilter = UNavigationQueryFilter::GetQueryFilter(*NavData, QueryOwner, FilterClass);
	FPathFindingQuery Query(QueryOwner, *NavData, ContextPosition, Point.Location, NavFilter);
	Query.SetAllowPartialPaths(false);

	const FPathFindingResult Result = NavSys->FindPathSync(Query, EPathFindingMode::Regular);
	const TArray<FNavPathPoint>& PathPoints = Result.Path->GetPathPoints();
	//const float MaxDistance = PosIndex % 2 == 0 ? PathDistance.X : PathDistance.Y;
	float DistanceTraveled = 0.0f;

	// Iterate through path points until reaching the desired distance
	for (int32 i = 0; i < PathPoints.Num() - 1; ++i)
	{
		// Calculate the distance between current point and the next point in the path
		const float SegmentDistance = FVector::Distance(PathPoints[i], PathPoints[i + 1]);

		// Check if traveling to the next point exceeds the target distance
		if (DistanceTraveled + SegmentDistance > PathDistance)
		{
			// Calculate the interpolation factor based on remaining distance
			const float RemainingDistance = PathDistance - DistanceTraveled;
			const float InterpolationFactor = RemainingDistance / SegmentDistance;

			// Interpolate between current point and next point
			const FVector NewPosition = FMath::Lerp<FVector>(PathPoints[i], PathPoints[i + 1], InterpolationFactor);
			//DrawDebugSphere(QueryInstance.World, NewPosition, 50.f, 8, FColor::Red, false, 7.f, 0, 1.f);
		
			Point = FNavLocation(NewPosition);
			return;
		}

		// If not reached the desired distance yet, accumulate distance traveled
		DistanceTraveled += SegmentDistance;
	}
}

ANavigationData* UEnvQueryGenerator_SafeGroupLocations::FindNavigationData(UNavigationSystemV1& NavSys, UObject* Owner) const
{
	INavAgentInterface* NavAgent = Cast<INavAgentInterface>(Owner);
	if (NavAgent)
	{
		return NavSys.GetNavDataForProps(NavAgent->GetNavAgentPropertiesRef(), NavAgent->GetNavAgentLocation());
	}

	return NavSys.GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
}

bool UEnvQueryGenerator_SafeGroupLocations::RunLineTraceTo(const FVector& ItemPos, const FVector& ContextPos, AActor* ItemActor, UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params, const FVector& Extent)
{
	FCollisionQueryParams TraceParams(Params);
	TraceParams.AddIgnoredActor(ItemActor);

	const bool bHit = World->LineTraceTestByChannel(ContextPos, ItemPos, Channel, TraceParams);
	return bHit;
}

bool UEnvQueryGenerator_SafeGroupLocations::RunBoxTraceTo(const FVector& ItemPos, const FVector& ContextPos, AActor* ItemActor, UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params, const FVector& Extent)
{
	FCollisionQueryParams TraceParams(Params);
	TraceParams.AddIgnoredActor(ItemActor);

	const bool bHit = World->SweepTestByChannel(ContextPos, ItemPos, FQuat((ItemPos - ContextPos).Rotation()), Channel, FCollisionShape::MakeBox(Extent), TraceParams);
	return bHit;
}

bool UEnvQueryGenerator_SafeGroupLocations::RunSphereTraceTo(const FVector& ItemPos, const FVector& ContextPos, AActor* ItemActor, UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params, const FVector& Extent)
{
	FCollisionQueryParams TraceParams(Params);
	TraceParams.AddIgnoredActor(ItemActor);

	const bool bHit = World->SweepTestByChannel(ContextPos, ItemPos, FQuat::Identity, Channel, FCollisionShape::MakeSphere(FloatCastChecked<float>(Extent.X, UE::LWC::DefaultFloatPrecision)), TraceParams);
	return bHit;
}

bool UEnvQueryGenerator_SafeGroupLocations::RunCapsuleTraceTo(const FVector& ItemPos, const FVector& ContextPos, AActor* ItemActor, UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params, const FVector& Extent)
{
	FCollisionQueryParams TraceParams(Params);
	TraceParams.AddIgnoredActor(ItemActor);

	const bool bHit = World->SweepTestByChannel(ContextPos, ItemPos, FQuat::Identity, Channel, FCollisionShape::MakeCapsule(FloatCastChecked<float>(Extent.X, UE::LWC::DefaultFloatPrecision), FloatCastChecked<float>(Extent.Z, UE::LWC::DefaultFloatPrecision)), TraceParams);
	return bHit;
}

FSharedConstNavQueryFilter UEnvQueryGenerator_SafeGroupLocations::GetQueryFilter(const ANavigationData& NavData,
	TSubclassOf<UNavigationQueryFilter> FilterClass)
{
	if (FilterClass)
	{
		const UNavigationQueryFilter* DefFilterOb = FilterClass.GetDefaultObject();
		// no way we have not default object here
		check(DefFilterOb);
		return DefFilterOb->GetQueryFilter(NavData, nullptr);
	}

	return nullptr;
}

FSharedConstNavQueryFilter UEnvQueryGenerator_SafeGroupLocations::GetQueryFilter(const ANavigationData& NavData,
	const UObject* Querier, TSubclassOf<UNavigationQueryFilter> FilterClass)
{
	if (FilterClass)
	{
		const UNavigationQueryFilter* DefFilterOb = FilterClass.GetDefaultObject();
		// no way we have not default object here
		check(DefFilterOb);
		return DefFilterOb->GetQueryFilter(NavData, Querier);
	}

	return nullptr;
}

/*
void UEnvQueryGenerator_SafeGroupLocations::ValidateSourcePoints(TArray<FNavLocation>& SourcePoints, TArray<FNavLocation>& GridPoints,
                                                                 FEnvQueryInstance& QueryInstance, const FVector& SourcePosition) const
{
	TArray<FNavLocation> ValidatedPoints;
	for (FNavLocation& Point : SourcePoints)
	{
		TArray<FNavLocation> ClosestPoints;
		TArray<float> ClosestDistances;
	
		const int32 MaxClosestPoints = 25;
		ClosestDistances.Init(MAX_FLT, MaxClosestPoints);
		ClosestPoints.Init(FNavLocation(), MaxClosestPoints);
		
		for (int i = 0; i < GridPoints.Num(); ++i)
		{				
			const float Dist = (Point.Location - GridPoints[i].Location).Length();

			// Check if the current distance is closer than any of the existing closest distances        
			for (int32 j = 0; j < MaxClosestPoints; ++j)
			{
				if (Dist < ClosestDistances[j])
				{
					// Shift all closer distances down by one index
					for (int32 k = MaxClosestPoints - 1; k > j; --k)
					{
						ClosestDistances[k] = ClosestDistances[k - 1];
						ClosestPoints[k] = ClosestPoints[k - 1];
					}

					// Insert the new closest distance and point
					ClosestDistances[j] = Dist;
					ClosestPoints[j] = GridPoints[i];
					break;
				}
			}
		}
		
		ValidatedPoints.Append(ClosestPoints);
		
		if(UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(QueryInstance.World))
		{
			if(UObject* QueryOwner = QueryInstance.Owner.Get())
			{
				const ANavigationData* NavData = FindNavigationData(*NavSys, QueryOwner);
				const FSharedConstNavQueryFilter NavFilter = UNavigationQueryFilter::GetQueryFilter(*NavData, QueryOwner, FilterClass);

				float LowestCost = MAX_FLT;
				FNavLocation BestPoint;
			
				for (FNavLocation& ClosestPoint : ClosestPoints)
				{
					// Calculate the cost of the path to the current closest point
					const float PathLength = FindPathLengthTo(ClosestPoint.Location, SourcePosition, EPathFindingMode::Regular, *NavData, *NavSys, NavFilter, QueryOwner);

					// Check if the current point has a lower cost than the lowest found so far
					if (PathLength < LowestCost)
					{
						LowestCost = PathLength;
						BestPoint = ClosestPoint;
					}
				}
		
				ValidatedPoints.Add(BestPoint);
				//DrawDebugSphere(QueryInstance.World, BestPoint.Location, 70.f, 8, FColor::Red, false, 5.f, 0, 1.f);
			}
		}
	}
	
	SourcePoints = ValidatedPoints;		
}

void UEnvQueryGenerator_SafeGroupLocations::GenerateSourcePoints(TArray<FNavLocation>& SourcePoints,
	FEnvQueryInstance& QueryInstance, const float Spacing, const FVector& SourcePosition, const FRotator& Rotation, float& TotalSpacing) const
{
	OffsetX.BindData(QueryInstance.Owner.Get(), QueryInstance.QueryID);
	const float X = OffsetX.GetValue();

	OffsetY.BindData(QueryInstance.Owner.Get(), QueryInstance.QueryID);
	const float Y = OffsetY.GetValue();
	
	PositionsAmount.BindData(QueryInstance.Owner.Get(), QueryInstance.QueryID);		
	const int32 Amount = PositionsAmount.GetValue();

	float SpacingLeft = 0.f, SpacingRight = 0.f;
	const bool bIsOddCount = Amount % 2 != 0;		
	
	for (int i = 0; i < Amount; ++i)
	{
		FVector LocalOffset = FVector(X, Y, 0.f);

		if(i % 2 != 0)
		{
			if(SpacingLeft == 0.f && !bIsOddCount)
			{
				SpacingLeft += Spacing * 0.5f;
			}				
	
			LocalOffset *= SpacingLeft;
			LocalOffset.Y = LocalOffset.Y * -1;
			SpacingLeft += Spacing;
		}
		else
		{
			if(SpacingRight == 0.f && !bIsOddCount)
			{
				SpacingRight += Spacing * 0.5f;
			}				
	
			LocalOffset *= SpacingRight;
			SpacingRight += Spacing;
		}				
		
		LocalOffset = Rotation.RotateVector(LocalOffset);
		FVector NewPos = SourcePosition + LocalOffset;
		SourcePoints.Add(FNavLocation(NewPos));

		DrawDebugDirectionalArrow(QueryInstance.World, NewPos, NewPos + FVector(0.f,0.f,400.f), 300.f, FColor::Red, false, 5.f, 0, 5.f);
	}

	TotalSpacing = SpacingLeft + SpacingRight;
}

/*
for (int j = 0; j < 3; ++j)
		{
 FVector SourceOffset = FVector::ZeroVector;
			if (j == 1)
			{
				SourceOffset.X = Spacing * 0.5f;
				
			}
			else if (j == 2)
			{
				SourceOffset.X = -Spacing * 0.5f;
			}
			SourceOffset = Rotation.RotateVector(SourceOffset);#1#

/*FCollisionQueryParams TraceParams(TEXT("EnvQueryTrace"), TraceData.bTraceComplex);
TraceParams.bTraceComplex = true;
TraceParams.bReturnPhysicalMaterial = false;

TArray<AActor*> IgnoredActors;
if (QueryInstance.PrepareContext(Context, IgnoredActors))
{
	TraceParams.AddIgnoredActors(IgnoredActors);
}
}

const ECollisionChannel TraceCollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceData.TraceChannel);

// Calculate end trace distance 
FVector TraceDirection = SourcePosition + LocalOffset - SourcePosition;
const float TraceDistance = TraceDirection.Length();
TraceDirection.Normalize();

const FVector EndTrace = SourcePosition + TraceDistance * TraceDirection;

FHitResult HitResult;
QueryInstance.World->LineTraceSingleByChannel(HitResult, SourcePosition, EndTrace, TraceCollisionChannel, TraceParams);

if(HitResult.bBlockingHit)
{
	PositionsArray.Add(FNavLocation(HitResult.Location));
}
else
{
	PositionsArray.Add(FNavLocation(SourcePosition + LocalOffset));
}#1#

bool UEnvQueryGenerator_SafeGroupLocations::RunLineTraceTo(const FVector& ItemPos, const FVector& ContextPos,
	AActor* ItemActor, UWorld* World, ECollisionChannel Channel, const FCollisionQueryParams& Params) const
{
	FCollisionQueryParams TraceParams(Params);
	TraceParams.AddIgnoredActor(ItemActor);

	const bool bHit = World->LineTraceTestByChannel(ContextPos, ItemPos, Channel, TraceParams);
	return bHit;
}

float UEnvQueryGenerator_SafeGroupLocations::FindPathLengthTo(const FVector& ItemPos, const FVector& ContextPos,
	EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystemV1& NavSys,
	FSharedConstNavQueryFilter NavFilter, const UObject* PathOwner) const
{	
	FPathFindingQuery Query(PathOwner, NavData, ContextPos, ItemPos, NavFilter);
	Query.SetAllowPartialPaths(false);

	FPathFindingResult Result = NavSys.FindPathSync(Query, Mode);
	// Static cast this to a float, for EQS scoring purposes float precision is OK.
	return (Result.IsSuccessful()) ? static_cast<float>(Result.Path->GetLength()) : BIG_NUMBER;
}

FVector UEnvQueryGenerator_SafeGroupLocations::FindPositionAtPathLength(const FVector& ItemPos,
	const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystemV1& NavSys,
	FSharedConstNavQueryFilter NavFilter, const UObject* PathOwner) const
{
	FPathFindingQuery Query(PathOwner, NavData, ContextPos, ItemPos, NavFilter);
	Query.SetAllowPartialPaths(false);

	FPathFindingResult Result = NavSys.FindPathSync(Query, Mode);
	const TArray<FVector>& PathPoints = Result.Path->GetPathPoints();
	float DistanceTraveled = 0.0f;
	float TargetDistance = X; // X is the distance you want to travel

	// Iterate through path points until reaching the desired distance
	for (int32 i = 0; i < PathPoints.Num() - 1; ++i)
	{
		// Calculate the distance between current point and the next point in the path
		float SegmentDistance = FVector::Distance(PathPoints[i], PathPoints[i + 1]);

		// Check if traveling to the next point exceeds the target distance
		if (DistanceTraveled + SegmentDistance > TargetDistance)
		{
			// Calculate the interpolation factor based on remaining distance
			float RemainingDistance = TargetDistance - DistanceTraveled;
			float InterpolationFactor = RemainingDistance / SegmentDistance;

			// Interpolate between current point and next point
			FVector Position = FMath::Lerp(PathPoints[i], PathPoints[i + 1], InterpolationFactor);
        
			// Position now holds the position X distance along the path
			break; // No need to iterate further
		}

		// If not reached the desired distance yet, accumulate distance traveled
		DistanceTraveled += SegmentDistance;
	}
}*/
