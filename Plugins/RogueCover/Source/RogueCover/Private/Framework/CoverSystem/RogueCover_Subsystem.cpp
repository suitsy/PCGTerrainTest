// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CoverSystem/RogueCover_Subsystem.h"
#include "NavigationSystem.h"
#include "Framework/Settings/RogueCover_Settings.h"
#include "Framework/CoverSystem/RogueCover_Point.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NavMesh/RecastNavMesh.h"

URogueCover_Subsystem::URogueCover_Subsystem()
{
	// Octree
	CoverPointOctree = new FCoverPointOctree(FVector(0, 0, 0), 64000);

	bIsRefreshed = false;
	bIsRefreshing = false;
	bHasGeneratedCovers = false;
}

void URogueCover_Subsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	GenerateCoverPoints();

	// Bind to navigation generation in case we want to rebuild covers
	if (UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		NavSystem->OnNavigationGenerationFinishedDelegate.AddDynamic(this, &ThisClass::OnNavigationGenerationFinished);
	}
}

void URogueCover_Subsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(bHasGeneratedCovers)
	{
		DebugCoverPoints();
	}
}

TStatId URogueCover_Subsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URogueCover_Subsystem, STATGROUP_Tickables);
}

void URogueCover_Subsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	
}

void URogueCover_Subsystem::Deinitialize()
{
	// Unbind navigation delegate	
	if (UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		NavSystem->OnNavigationGenerationFinishedDelegate.RemoveDynamic(this, &ThisClass::OnNavigationGenerationFinished);
	}
	
	delete CoverPointOctree;
	
	Super::Deinitialize();

	
}

void URogueCover_Subsystem::GenerateCoverPoints()
{
	if (GetWorld())
	{
		if (!bIsRefreshing)
		{
			Generate(true, true);
		}
	}
	else if(AllCoverPoints.Num() > 0)
	{
		// Generate from saved data
		for (URogueCover_Point* CoverPoint: AllCoverPoints)
		{
			CoverPointOctree->AddElement(FCoverPointOctreeElement(*CoverPoint)); 
		}
	}
}

void URogueCover_Subsystem::Generate(const bool ForceRegeneration, const bool DoAsync)
{
	if (bHasGeneratedCovers && !ForceRegeneration)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// Reset visual debug information
	UKismetSystemLibrary::FlushPersistentDebugLines(World);

	// Reset Octree
	CoverPointOctree->Destroy();
	delete CoverPointOctree; 
	CoverPointOctree = new FCoverPointOctree(FVector(0, 0, 0), 64000);

	// reset
	bHasGeneratedCovers = false;

	// Get Navigation Mesh Data 
	const ARecastNavMesh* NavMeshData = GetNavMeshData(World);
	if (NavMeshData == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No navigation data available!"), *GetNameSafe(this));
		return;
	}

	FRecastDebugGeometry NavMeshGeometry;
	NavMeshGeometry.bGatherNavMeshEdges = -1;
	NavMeshData->BeginBatchQuery();
	NavMeshData->GetDebugGeometryForTile(NavMeshGeometry, INDEX_NONE);

	// Async processing
	/*if (DoAsync)
	{
		(new FAutoDeleteAsyncTask<FGenerateCoversAsyncTask>(this, NavMeshGeometry))->StartBackgroundTask();

		return;
	}
	else // use game thread
	{
		AnaliseMeshData(NavMeshGeometry);
	}*/

	if(const URogueCover_Settings* CoverSettings = GetDefault<URogueCover_Settings>())
	{
		if (CoverSettings->DebugDisplayLog)
		{
			UE_LOG(LogTemp, Log, TEXT("%s: Finished generating cover points."), *GetNameSafe(this));
			CoverPointOctree->DumpStats(); 
		}
	}

	OnCoverGenerationComplete.Broadcast();
}

ARecastNavMesh* URogueCover_Subsystem::GetNavMeshData(UWorld* World) const
{
	const UNavigationSystemV1* NavSys = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem());

	if (NavSys == nullptr)
	{
		return nullptr;
	}

	return Cast<ARecastNavMesh>(NavSys->GetDefaultNavDataInstance());
}

void URogueCover_Subsystem::AnaliseMeshData(const FRecastDebugGeometry& NavMeshGeometry)
{
	if(const URogueCover_Settings* CoverSettings = GetDefault<URogueCover_Settings>())
	{
		// Navigation mesh data analysis
		const TArray<FVector>& NavMeshEdgeVerts = NavMeshGeometry.NavMeshEdges;
		for (int32 Idx = 0; Idx < NavMeshEdgeVerts.Num(); Idx += 2)
		{
			const FNavLocation SegmentStart = FNavLocation(NavMeshEdgeVerts[Idx]);
			const FNavLocation SegmentEnd = FNavLocation(NavMeshEdgeVerts[Idx + 1]);

			const FVector Segment = SegmentEnd.Location - SegmentStart.Location;
			const float SegmentSize = Segment.Size();
			FVector Perp = Get2DPerpVector(Segment);
			FVector SegmentDirection = Segment;
			SegmentDirection.Normalize();

			// Check start and end position 			
			TestAndAddPoint(SegmentStart.Location, SegmentDirection, GetWorld(), Perp);
			TestAndAddPoint(SegmentEnd.Location, SegmentDirection, GetWorld(), Perp);

#if WITH_EDITOR
			bool bDebugDraw = false;
			if (CoverSettings->bDraw1AllSegmentPointsTested && (GetPlayerCameraLocation() - SegmentStart.Location).Length() < CoverSettings->DebugDistance)
			{
				bDebugDraw = true;
				DrawDebugSphere(GetWorld(), SegmentStart.Location, 20, 4, FColor::Green, true, 10.f);
				DrawDebugSphere(GetWorld(), SegmentEnd.Location, 20, 4, FColor::Red, true, 10.f);
			}
#endif

			// Check inner points if segment is long enough
			if (SegmentSize > CoverSettings->SegmentLength)
			{
				const int32 NumSegmentPieces = Segment.Size() / CoverSettings->SegmentLength;
				for (int32 idx = 1; idx < NumSegmentPieces; idx++)
				{
					const FVector SegmentPoint = SegmentStart.Location + (idx * CoverSettings->SegmentLength * SegmentDirection);
					TestAndAddPoint(SegmentPoint, SegmentDirection, GetWorld(), Perp);

#if WITH_EDITOR
					if (bDebugDraw)
					{
						DrawDebugSphere(GetWorld(), SegmentPoint, 20, 4, FColor::Blue, true, 10.f);
					}
#endif
				}
			}
		}
	}

	bHasGeneratedCovers = true;

	CoverPointOctree->ShrinkElements();
}

FVector URogueCover_Subsystem::Get2DPerpVector(const FVector& v1) const
{
	return FVector(v1.Y, -v1.X, 0);
}

void URogueCover_Subsystem::TestAndAddPoint(const FVector& SegmentPoint, const FVector& SegmentDirection, UWorld* World, const FVector& CoverDirection)
{
	if(const URogueCover_Settings* CoverSettings = GetDefault<URogueCover_Settings>())
	{
		const bool AlreadyCoverWithinBounds = CoverExistWithinBounds(FBoxCenterAndExtent(SegmentPoint, FVector(CoverSettings->MinSpaceBetweenValidPoints)));

#if WITH_EDITOR
	
		if (CoverSettings->bDraw2SegmentPointsWithinBounds && (GetPlayerCameraLocation() - SegmentPoint).Length() < CoverSettings->DebugDistance)
		{
			DrawDebugSphere(GetWorld(), SegmentPoint, 20, 2, AlreadyCoverWithinBounds ? FColor::Red : FColor::Green, true, 10.f);
		}
	
#endif

		// Check that there's no point at a distance of less than MinSpaceBetweenValidPoints already added
		if(AlreadyCoverWithinBounds)
			return;

		FVector TraceDirection = Get2DPerpVector(SegmentDirection);
		TraceDirection.Normalize();
		const FVector TraceVec = CoverSettings->TraceLength*TraceDirection;


		// Check if valid point in first direction 
		URogueCover_Point* Point = IsValidCoverPoint(World, SegmentPoint, -SegmentDirection, -TraceVec, CoverDirection);
		// If not then let's try the other side
		if (Point == nullptr)
		{
			Point = IsValidCoverPoint(World, SegmentPoint, SegmentDirection, TraceVec, CoverDirection);
		}
		
		// If found a valid point, add to storage
		if (Point != nullptr)
		{
			AllCoverPoints.Add(Point);
			const FCoverPointOctreeElement CoverPointOctreeElement(*Point); 
			CoverPointOctree->AddElement(CoverPointOctreeElement); 
		}
	}
}

bool URogueCover_Subsystem::CoverExistWithinBounds(const FBoxCenterAndExtent& BoundsIn) const
{
	bool Result = false;
	CoverPointOctree->FindFirstElementWithBoundsTest(BoundsIn, [&Result](const FCoverPointOctreeElement& CoverPoint) { Result = true; return true; });
	return Result;
}

FVector URogueCover_Subsystem::GetPlayerCameraLocation() const
{
	if(const APlayerController* Player = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if(const APlayerCameraManager* PlayerCameraManager = Player->PlayerCameraManager)
		{
			return PlayerCameraManager->GetCameraLocation();
		}
	}

	return FVector::ZeroVector;
}

URogueCover_Point* URogueCover_Subsystem::IsValidCoverPoint(UWorld* World, const FVector& SegmentPoint, const FVector& Segment, const FVector& TraceVec, const FVector& Perp) const
{
	FVector CoverDirection = -Perp;
	CoverDirection.Normalize();

	if(const URogueCover_Settings* CoverSettings = GetDefault<URogueCover_Settings>())
	{
		// Check if this position hides a crouched character when seen from the perpendicular direction 
		// That's the minimum requirement for being a cover 
		if (IsProvidingCover(World, SegmentPoint, Segment, TraceVec, 0, CoverSettings->HeightMaxCrouching))
		{
			const FVector LeaningRightVec = CoverSettings->OffsetWhenLeaningSides*(Segment); 
			const FVector LeaningLeftVec = -LeaningRightVec; 


			// Check if it is a right or left or front cover for crouched position 
			const float HeightCrouchingCheck = CoverSettings->HeightMaxCrouching / 2.f;
			const bool CanShootFromLeftCrouched = TwoPassesCheck(World, SegmentPoint, LeaningLeftVec, HeightCrouchingCheck, CoverDirection);
			const bool CanShootFromRightCrouched = TwoPassesCheck(World, SegmentPoint, LeaningRightVec, HeightCrouchingCheck, CoverDirection);
			const bool CanShootFromFrontCrouched = TwoPassesCheck(World, SegmentPoint, CoverSettings->HeightMaxStanding*FVector::UpVector - 30.f, HeightCrouchingCheck, CoverDirection); // -30.f for the offset between the point and the floor

			// Check for standing (No front in that case) 
			bool CanShootFromLeftStanding = false;
			bool CanShootFromRightStanding = false;
			if (IsProvidingCover(World, SegmentPoint, Segment, TraceVec, CoverSettings->HeightMaxCrouching + CoverSettings->StepDistTrace, CoverSettings->HeightMaxStanding))
			{
				const float HeightStandingCheck = CoverSettings->HeightMaxCrouching + ((CoverSettings->HeightMaxStanding - CoverSettings->HeightMaxCrouching) / 2.f);
				CanShootFromLeftStanding = CanShootFromLeftCrouched; // TwoPassesCheck(World, SegmentPoint, LeaningLeftVec, HeightStandingCheck, Perp);
				CanShootFromRightStanding = CanShootFromRightCrouched; //TwoPassesCheck(World, SegmentPoint, LeaningRightVec, HeightStandingCheck, Perp);
			}


			if ( CanShootFromLeftCrouched || CanShootFromRightCrouched || CanShootFromFrontCrouched || CanShootFromLeftStanding || CanShootFromRightStanding )
			{
				URogueCover_Point* NewPoint = NewObject<URogueCover_Point>();
				NewPoint->Location = SegmentPoint;
				NewPoint->SetDirectionToWall(CoverDirection);

				NewPoint->bLeftCoverCrouched = CanShootFromLeftCrouched;  
				NewPoint->bRightCoverCrouched = CanShootFromRightCrouched;
				NewPoint->bFrontCoverCrouched = CanShootFromFrontCrouched;

				NewPoint->bLeftCoverStanding = CanShootFromLeftStanding;
				NewPoint->bRightCoverStanding = CanShootFromRightStanding;

				return NewPoint;
			}
		}
	}
	
	return nullptr; 
}

bool URogueCover_Subsystem::IsProvidingCover(UWorld* World, const FVector& SegmentPoint, const FVector& SegmentDirection,	const FVector& TraceVec, const float StartHeight, const float EndHeight) const
{
	if(const URogueCover_Settings* CoverSettings = GetDefault<URogueCover_Settings>())
	{
		bool bHit = false;
		FCollisionQueryParams CollisionParams;
		const float HalfWidth = CoverSettings->WidthMaxAgent / 2.f;

#if WITH_EDITOR
		const bool bDebugLocal = CoverSettings->bDraw3SimpleCoverGeometryTest && (GetPlayerCameraLocation() - SegmentPoint).Length() < CoverSettings->DebugDistance;
		//if (bDebugLocal)
		//{
		//	const FName TraceTag("MyTraceTag");
		//	World->DebugDrawTraceTag = TraceTag;
		//	CollisionParams.TraceTag = TraceTag;
		//}
#endif

		// First trace to find normal to geometry
		FHitResult HitResult;
		bHit = World->LineTraceSingleByChannel(HitResult, SegmentPoint, SegmentPoint + TraceVec, ECC_WorldStatic, CollisionParams);
		if (!bHit)
		{
#if WITH_EDITOR
			if (bDebugLocal)
			{
				DrawDebugSphere(World, SegmentPoint + FVector(0.f, 0.f, StartHeight), 25, 4, FColor::Red, true, 10.f);
				DrawDebugDirectionalArrow(World, SegmentPoint, SegmentPoint + TraceVec, 10, FColor::Red, true, 10.f);
			}
#endif
			return false;
		}

		// Multi trace to ensure cover is safe using the normal to impact from previous trace as direction 
		FVector TraceDirection = TraceVec; // -HitResult.ImpactNormal;
		TraceDirection.Normalize();
		TraceDirection = CoverSettings->TraceLength * TraceDirection; 
		// First checking for crouch
		for (float xOffset = -HalfWidth; xOffset <= HalfWidth; xOffset += CoverSettings->StepDistTrace)
		{
			for (float zOffset = StartHeight; zOffset <= EndHeight; zOffset += CoverSettings->StepDistTrace)
			{
				const FVector StartPos = SegmentPoint + FVector(0, 0, zOffset) + xOffset*SegmentDirection;

				bHit = World->LineTraceTestByChannel(StartPos, StartPos + TraceDirection, ECC_WorldStatic, CollisionParams);
				if (!bHit)
				{
#if WITH_EDITOR
					if (bDebugLocal)
					{
						DrawDebugSphere(World, SegmentPoint + FVector(0.f, 0.f, StartHeight + 10.f), 25, 4, FColor::Orange, true, 10.f);
						DrawDebugDirectionalArrow(World, StartPos, StartPos + TraceDirection, 10, FColor::Orange, true, 10.f);
					}
#endif
					return false;
				}
			}
		}


#if WITH_EDITOR
		if (bDebugLocal)
		{
			DrawDebugSphere(World, SegmentPoint + FVector(0.f, 0.f, StartHeight + 20.f), 25, 4, FColor::Green, true, 10.f);
		}
#endif
	}
	
	return true;
}

bool URogueCover_Subsystem::TwoPassesCheck(UWorld* World, const FVector& SegmentPoint, const FVector& FirstTestDirection, const float& VerticalOffset, const FVector& CoverDirection) const
{
	if(const URogueCover_Settings* CoverSettings = GetDefault<URogueCover_Settings>())
	{
		const FVector PointOff = SegmentPoint + FVector(0.f, 0.f, VerticalOffset);
		const FVector FirstTestEndPosition = PointOff + FirstTestDirection;
		FQuat Quat = FVector::UpVector.ToOrientationRotator().Quaternion();
		Quat.Normalize();
		FHitResult HitResult;
	
		//Check if it is a left cover, a right cover, or a crouch cover
		FCollisionQueryParams CollisionParams;

#if WITH_EDITOR
		const bool bDebugLocal = (GetPlayerCameraLocation() - SegmentPoint).Length() < CoverSettings->DebugDistance;
		/*if (bDebugLocal && bDraw4SecondPassTracesSides)
		{
			const FName TraceTag("MyTraceTag");
			World->DebugDrawTraceTag = TraceTag;
			CollisionParams.TraceTag = TraceTag;
		}*/
#endif

		bool HitOnSide = World->SweepSingleByChannel(HitResult, PointOff, FirstTestEndPosition, Quat, ECC_WorldStatic, FCollisionShape::MakeSphere(CoverSettings->SphereSize), CollisionParams);

#if WITH_EDITOR
		if (bDebugLocal && CoverSettings->bDraw4SecondPassTracesSides)
		{
			DrawDebugSphere(World, SegmentPoint, 20, 2, HitOnSide ? FColor::Red : FColor::Green, true, 10.f);
			if (HitOnSide)
			{
				DrawDebugDirectionalArrow(World, PointOff, FirstTestEndPosition, 10, FColor::Blue, true, 10.f);
				DrawDebugDirectionalArrow(World, PointOff, HitResult.Location, 10, FColor::Red, true, 10.f);
			}
		}
#endif

		if (!HitOnSide)
		{
			FHitResult FrontResult, DownResult;

			// Side Front 
			const FVector SideFrontStart = FirstTestEndPosition;
			const FVector SideFrontEnd = SideFrontStart + CoverSettings->OffsetFrontAim * CoverDirection;
			bool HitSideFront = World->SweepSingleByChannel(FrontResult, SideFrontStart, SideFrontEnd, Quat, ECC_WorldStatic, FCollisionShape::MakeSphere(CoverSettings->SphereSize), CollisionParams);
		
			// Side Bottom = Floor check
			const FVector SideDownStart = PointOff + FVector(FirstTestDirection.X, FirstTestDirection.Y, 0.f);
			const FVector SideDownEnd = SideDownStart + FVector::UpVector*(-100);
			bool HitSideBottom = World->SweepSingleByChannel(DownResult, SideDownStart, SideDownEnd, Quat, ECC_WorldStatic, FCollisionShape::MakeSphere(CoverSettings->SphereSize), CollisionParams);

#if WITH_EDITOR
			if (bDebugLocal && CoverSettings->bDraw4SecondPassTracesSidesFrontAndBottom)
			{
				DrawDebugSphere(World, SegmentPoint, 20, 2, !HitSideFront && HitSideBottom ? FColor::Green : FColor::Red, true, 10.f);
			
				if (HitSideFront)
				{
					DrawDebugDirectionalArrow(World, SideFrontStart, SideFrontEnd, 10, FColor::Purple, true, 10.f);
					DrawDebugDirectionalArrow(World, SideFrontStart, FrontResult.Location, 10, FColor::Red, true, 10.f);
				}
				if (!HitSideBottom)
				{
					DrawDebugDirectionalArrow(World, SideDownStart, SideDownEnd, 10, FColor::Cyan, true, 10.f);
					DrawDebugDirectionalArrow(World, SideDownStart, SideDownEnd, 10, FColor::Red, true, 10.f);
				}
			}
#endif
			// Should not hit on side front to allow aiming, but should hit on side bottom to avoid going through the floor :P 
			if (!HitSideFront && HitSideBottom)
				return true;
		}
	}
	
	return false; 
}

void URogueCover_Subsystem::DebugCoverPoints()
{
	if(const URogueCover_Settings* CoverSettings = GetDefault<URogueCover_Settings>())
	{
		if(CoverSettings->bShowCoverDebug)
		{
			// debug info
			if (bIsRefreshed)
			{
				bIsRefreshed = false;

				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("ACoverGenerator : GenerateCovers() finished ASYNC")));

				if (CoverSettings->DebugDisplayLog)
				{
					UE_LOG(LogTemp, Log, TEXT("%s: Finished generating cover points."), *GetNameSafe(this));
					CoverPointOctree->DumpStats();
				}
			}

#if WITH_EDITOR
			// Display Debug information

			const FVector VerticalOffset = FVector(0, 0, 200); 
			if (CoverSettings->DebugDrawAllBusyPoints)
			{
				for (const URogueCover_Point* Cover : CoverPointsCurrentlyUsed)
				{
					if ((GetPlayerCameraLocation() - Cover->Location).Length() > CoverSettings->DebugDistance)
						continue;
	
					DrawDebugSphere(GetWorld(), Cover->Location + VerticalOffset, 50, 4, FColor::Red);
				}
			}

			if (CoverSettings->DebugDrawAllPoints)
			{
				const FVector CrouchHeight = FVector(0.f, 0.f, CoverSettings->HeightMaxCrouching);
				const FVector StandingHeight = FVector(0.f, 0.f, CoverSettings->HeightMaxStanding);

				TArray<URogueCover_Point*> PointsToDraw = GetCoverWithinBounds(FBoxCenterAndExtent(GetPlayerCameraLocation(), FVector(CoverSettings->DebugDistance)));

				for (const URogueCover_Point* Cover : PointsToDraw)
				{
					if ((GetPlayerCameraLocation() - Cover->Location).Length() > CoverSettings->DebugDistance)
						continue; 


					FVector Perp = Get2DPerpVector(Cover->GetDirectionToWall());
					Perp.Normalize();

					const FVector Left = Perp;
					const FVector Right = -Perp;
					const FVector Front = Cover->GetDirectionToWall();
					const FVector Top = FVector::UpVector; 

					if (Cover->bLeftCoverCrouched || Cover->bRightCoverCrouched || Cover->bFrontCoverCrouched)
					{
						const FVector CrouchLocation = Cover->Location + CrouchHeight;

						DrawDebugSphere(GetWorld(), CrouchLocation, 30, 4, FColor::Blue);

						if (Cover->bLeftCoverCrouched)
							DrawDebugCover(GetWorld(), CrouchLocation, Left, Front, 60.f);
						if (Cover->bRightCoverCrouched)
							DrawDebugCover(GetWorld(), CrouchLocation, Right, Front, 60.f);
						if (Cover->bFrontCoverCrouched)
							DrawDebugCover(GetWorld(), CrouchLocation, Top, Front, 60.f);

					}
					if (Cover->bLeftCoverStanding || Cover->bRightCoverStanding)
					{
						const FVector StandingLocation = Cover->Location + StandingHeight; 


						DrawDebugSphere(GetWorld(), StandingLocation, 30, 4, FColor::Blue);

						if (Cover->bLeftCoverStanding)
							DrawDebugCover(GetWorld(), StandingLocation, Left, Front, 60.f);
						if (Cover->bRightCoverStanding)
							DrawDebugCover(GetWorld(), StandingLocation, Right, Front, 60.f);
					}

				}

			}

			if (CoverSettings->bDrawOctreeBounds)
			{
				DrawOctreeBounds();
			}
		}
#endif
	}
}

TArray<URogueCover_Point*> URogueCover_Subsystem::GetCoverWithinBounds(const FBoxCenterAndExtent& BoundsIn) const
{
	// Iterating over a region in the octree and storing the elements
	TArray<URogueCover_Point*> Covers;
	CoverPointOctree->FindElementsWithBoundsTest(BoundsIn, [&Covers](const FCoverPointOctreeElement& CoverPoint) { Covers.Add(CoverPoint.GetOwner()); });

	// UE_LOG(LogTemp, Log, TEXT("Covers within bounds are(new) %d"), (Covers.Num()) );

	return Covers;
}

void URogueCover_Subsystem::DrawDebugCover(UWorld* World, const FVector& StartLocation, const FVector& Direction1, const FVector& Direction2, const float& DistanceToDraw) const
{
	const FVector End1 = StartLocation + DistanceToDraw * Direction1;

	DrawDebugDirectionalArrow(GetWorld(), StartLocation, End1, 15, FColor::Green);
	DrawDebugDirectionalArrow(GetWorld(), End1, End1 + 60.f * Direction2, 15, FColor::Green);
}

void URogueCover_Subsystem::DrawOctreeBounds() const
{
	if (!GetWorld())
		return;

	const FVector Extent = CoverPointOctree->GetRootBounds().Extent;

	const float Max = Extent.GetMax();
	const FVector MaxExtent = FVector(Max, Max, Max);
	const FVector Center = CoverPointOctree->GetRootBounds().Center;

	DrawDebugBox(GetWorld(), Center, MaxExtent, FColor().Blue, false, 0.0f);	
	DrawDebugSphere(GetWorld(), Center + MaxExtent, 4.0f, 12, FColor().White, false, 0.0f);
	DrawDebugSphere(GetWorld(), Center - MaxExtent, 4.0f, 12, FColor().White, false, 0.0f);
}

void URogueCover_Subsystem::OnNavigationGenerationFinished(ANavigationData* NavData)
{
	if(const URogueCover_Settings* CoverSettings = GetDefault<URogueCover_Settings>())
	{
		if (CoverSettings->bRegenerateAtNavigationRebuilt && !bIsRefreshing)
		{
			Generate(true, true);			
		}
	}
}
