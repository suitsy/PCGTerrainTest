// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/RogueRVO_Subsystem.h"
#include "AI/RVOAvoidanceInterface.h"
#include "AI/Navigation/NavEdgeProviderInterface.h"
#include "Framework/Components/RogueRVO_Component.h"
#include "Framework/Settings/RogueRVO_DeveloperSettings.h"

URogueRVO_Subsystem::URogueRVO_Subsystem()
{
	DefaultTimeToLive = 1.5f;
	LockTimeAfterAvoid = 0.2f;
	LockTimeAfterClean = 0.001f;
	DeltaTimeToPredict = 0.5f;
	ArtificialRadiusExpansion = 1.5f;
	bRequestedUpdateTimer = false;
	bAutoPurgeOutdatedObjects = true;
	HeightCheckMargin = 10.0f;
	EdgeProviderInterface = nullptr;
}

void URogueRVO_Subsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void URogueRVO_Subsystem::Deinitialize()
{
	Super::Deinitialize();
}

void URogueRVO_Subsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

TStatId URogueRVO_Subsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URogueRVO_Subsystem, STATGROUP_Tickables);
}

void URogueRVO_Subsystem::RegisterRVOComponent(URogueRVO_Component* RVOComponent, const float AvoidanceWeight)
{
	if (IRVOAvoidanceInterface* AvoidingComp = Cast<IRVOAvoidanceInterface>(RVOComponent))
	{
		const int32 NewAvoidanceUID = GetNewAvoidanceUID();
		AvoidingComp->SetRVOAvoidanceUID(NewAvoidanceUID);
		AvoidingComp->SetRVOAvoidanceWeight(AvoidanceWeight);

		RequestUpdateTimer();

		FRogueRVO_NavAvoidanceData AvoidanceData(this, AvoidingComp);
		UpdateRVO_Internal(NewAvoidanceUID, AvoidanceData);
	}
}

void URogueRVO_Subsystem::UpdateRVOComponent(URogueRVO_Component* RVOComponent)
{
	check(RVOComponent);
	const FRogueRVO_NavAvoidanceData NewAvoidanceData(this, RVOComponent);
	UpdateRVO_Internal(RVOComponent->GetRVOAvoidanceUID(), NewAvoidanceData);
}

FVector URogueRVO_Subsystem::GetAvoidanceVelocityForComponent(URogueRVO_Component* RVOComponent)
{
	if (IRVOAvoidanceInterface* AvoidingComp = Cast<IRVOAvoidanceInterface>(RVOComponent))
	{
		const FRogueRVO_NavAvoidanceData AvoidanceData(this, AvoidingComp);
		return GetAvoidanceVelocityIgnoringUID(AvoidanceData, DeltaTimeToPredict, AvoidingComp->GetRVOAvoidanceUID());
	}

	return FVector::ZeroVector;
}

FVector URogueRVO_Subsystem::GetAvoidanceVelocityIgnoringUID(const FRogueRVO_NavAvoidanceData& AvoidanceData, float DeltaTime, int32 IgnoreThisUID)
{
	return GetAvoidanceVelocity_Internal(AvoidanceData, DeltaTime, &IgnoreThisUID);
}

void URogueRVO_Subsystem::OverrideToMaxWeight(const int32 AvoidanceUID, const float Duration)
{
	if (FRogueRVO_NavAvoidanceData *AvoidObj = AvoidanceObjects.Find(AvoidanceUID))
	{
		if(const UWorld* WorldContext = GetTickableGameObjectWorld())
		{
			AvoidObj->OverrideWeightTime = WorldContext->TimeSeconds + Duration;
		}
	}
}

void URogueRVO_Subsystem::RemoveAvoidanceObject(const int32 AvoidanceUID)
{
	if (AvoidanceUID < 0)
	{
		return;
	}

	if (FRogueRVO_NavAvoidanceData* ExistingDataPtr = AvoidanceObjects.Find(AvoidanceUID))
	{
		ExistingDataPtr->RemainingTimeToLive = 0.0f;
	}

	//Expired, not in pool yet, assign to pool
	NewKeyPool.AddUnique(AvoidanceUID);
}

int32 URogueRVO_Subsystem::GetNewAvoidanceUID()
{
	int32 NewUID = AvoidanceObjects.Num();
	if (NewKeyPool.Num())
	{
		NewUID = NewKeyPool[NewKeyPool.Num() - 1];
		NewKeyPool.RemoveAt(NewKeyPool.Num() - 1, 1, false);
	}
	return NewUID;
}

void URogueRVO_Subsystem::RequestUpdateTimer()
{
	if(const UWorld* WorldContext = GetTickableGameObjectWorld())
	{
		if (!bRequestedUpdateTimer)
		{
			bRequestedUpdateTimer = true;
			WorldContext->GetTimerManager().SetTimer(TimerHandle_RemoveOutdatedObjects, this, &ThisClass::RemoveOutdatedObjects, DefaultTimeToLive * 0.5f, false);
		}
	}	
}

void URogueRVO_Subsystem::RemoveOutdatedObjects()
{
	if (bAutoPurgeOutdatedObjects == false)
	{
		// will be handled manually
		return;
	}

	bRequestedUpdateTimer = false;

	//Update RVO stuff. Mainly, this involves removing outdated RVO entries.
	bool bHasActiveObstacles = false;
	for (auto& AvoidanceObj : AvoidanceObjects)
	{
		FRogueRVO_NavAvoidanceData& AvoidanceData = AvoidanceObj.Value;

		if (AvoidanceData.RemainingTimeToLive > (DefaultTimeToLive * 0.5f))
		{
			//Update record with reduced TTL
			AvoidanceData.RemainingTimeToLive -= (DefaultTimeToLive * 0.5f);
			bHasActiveObstacles = true;
		}
		else if (!AvoidanceData.ShouldBeIgnored())
		{
			const int32 ObjectId = AvoidanceObj.Key;
			AvoidanceData.RemainingTimeToLive = 0.0f;
			NewKeyPool.AddUnique(ObjectId);
		}
	}

	if (bHasActiveObstacles)
	{
		RequestUpdateTimer();
	}
}

void URogueRVO_Subsystem::UpdateRVO_Internal(const int32 AvoidanceUID, const FRogueRVO_NavAvoidanceData& AvoidanceData)
{
	if (FRogueRVO_NavAvoidanceData* ExistingData = AvoidanceObjects.Find(AvoidanceUID))
	{
		// Maintain override weight time for existing data
		const double OverrideWeightTime = ExistingData->OverrideWeightTime;
		*ExistingData = AvoidanceData;
		ExistingData->OverrideWeightTime = OverrideWeightTime;
	}
	else
	{
		AvoidanceObjects.Add(AvoidanceUID, AvoidanceData);
	}
}

FVector URogueRVO_Subsystem::GetAvoidanceVelocity_Internal(const FRogueRVO_NavAvoidanceData& AvoidanceData, float DeltaTime, int32* IgnoreThisUID)
{
	if (DeltaTime <= 0.0f)
	{
		return AvoidanceData.Velocity;
	}

	FVector ReturnVelocity = AvoidanceData.Velocity * DeltaTime;
	FVector::FReal MaxSpeed = ReturnVelocity.Size2D();
	double CurrentTime;

	
	if (const UWorld* WorldContext = GetTickableGameObjectWorld())
	{
		CurrentTime = WorldContext->TimeSeconds;
	}
	else
	{
		return AvoidanceData.Velocity;
	}

	bool Unobstructed = true;
	
	//If we're moving very slowly, just push forward. Not sure it's worth avoiding at this speed, though I could be wrong.
	if (MaxSpeed < 0.01f)
	{
		return AvoidanceData.Velocity;
	}
	
	AllCones.Empty(AllCones.Max());
	for (auto& AvoidanceObj : AvoidanceObjects)
	{
		if ((IgnoreThisUID) && (*IgnoreThisUID == AvoidanceObj.Key))
		{
			continue;
		}
		FRogueRVO_NavAvoidanceData& OtherObject = AvoidanceObj.Value;

		//
		//Start with a few fast-rejects
		//

		//If the object has expired, ignore it
		if (OtherObject.ShouldBeIgnored())
		{
			continue;
		}

		//If other object is not in avoided group, ignore it
		if (AvoidanceData.ShouldIgnoreGroup(OtherObject.GroupMask))
		{
			continue;
		}

		// Check other object is in avoidance test radius range
		if (FVector2D(OtherObject.Center - AvoidanceData.Center).SizeSquared() > FMath::Square(AvoidanceData.TestRadius2D))
		{
			continue;
		}

		if (FMath::Abs(OtherObject.Center.Z - AvoidanceData.Center.Z) > OtherObject.HalfHeight + AvoidanceData.HalfHeight + HeightCheckMargin)
		{
			continue;
		}

		// Check if we are moving away from the obstacle, ignore it. Even if we're the slower one, let the other obstacle path around us.
		if ((ReturnVelocity | (OtherObject.Center - AvoidanceData.Center)) <= 0.0f)
		{
			continue;
		}

		// Create data for the avoidance routine		
		FVector PointBRelative = OtherObject.Center - AvoidanceData.Center;
		FVector EffectiveVelocityB, PointPlane[2], TowardB, SidewaysFromB;
		float RadiusB = (OtherObject.Radius * 10.f) + AvoidanceData.Radius;

		// Remove height for these checks, height checking will come later
		PointBRelative.Z = 0.0f;
		
		TowardB = PointBRelative.GetSafeNormal2D();
		if (TowardB.IsZero())
		{
			//Already intersecting, or aligned vertically, scrap this whole object.
			continue;
		}
		SidewaysFromB.Set(-TowardB.Y, TowardB.X, 0.0f);		
		
		// Use RVO (as opposed to VO) only for objects that are not overridden to max weight AND that are currently moving toward us.
		if ((OtherObject.OverrideWeightTime <= CurrentTime) && ((OtherObject.Velocity|PointBRelative) < 0.0f))
		{
			// Calculate the effective velocity
			float OtherWeight = (OtherObject.Weight + (1.0f - AvoidanceData.Weight)) * 0.5f;			//Use the average of what the other wants to be and what we want it to be.
			EffectiveVelocityB = ((AvoidanceData.Velocity * (1.0f - OtherWeight)) + (OtherObject.Velocity * OtherWeight)) * DeltaTime;
		}
		else
		{
			EffectiveVelocityB = OtherObject.Velocity * DeltaTime;		//This is equivalent to VO (not RVO) because the other object is not going to reciprocate our avoidance.
		}				
		checkSlow(EffectiveVelocityB.Z == 0.0f);

		FRogueRVO_VelocityAvoidanceCone NewCone;

		//Make the left plane
		PointPlane[0] = EffectiveVelocityB + (PointBRelative + (SidewaysFromB * RadiusB));
		PointPlane[1].Set(PointPlane[0].X, PointPlane[0].Y, PointPlane[0].Z + 100.0f);
		NewCone.ConePlane[0] = FPlane(EffectiveVelocityB, PointPlane[0], PointPlane[1]);		//First point is relative to A, which is ZeroVector in this implementation
		checkSlow((((PointBRelative+EffectiveVelocityB)|NewCone.ConePlane[0]) - NewCone.ConePlane[0].W) > 0.0f);		

		//Make the right plane
		PointPlane[0] = EffectiveVelocityB + (PointBRelative - (SidewaysFromB * RadiusB));
		PointPlane[1].Set(PointPlane[0].X, PointPlane[0].Y, PointPlane[0].Z - 100.0f);
		NewCone.ConePlane[1] = FPlane(EffectiveVelocityB, PointPlane[0], PointPlane[1]);		//First point is relative to A, which is ZeroVector in this implementation
		checkSlow((((PointBRelative+EffectiveVelocityB)|NewCone.ConePlane[1]) - NewCone.ConePlane[1].W) > 0.0f);
		
		if ((((ReturnVelocity|NewCone.ConePlane[0]) - NewCone.ConePlane[0].W) > 0.0f)
			&& (((ReturnVelocity|NewCone.ConePlane[1]) - NewCone.ConePlane[1].W) > 0.0f))
		{
			Unobstructed = false;
		}
		
		AllCones.Add(NewCone);		
	}
	
	if (Unobstructed)
	{
		//Trivial case, our ideal velocity is available.
		return AvoidanceData.Velocity;
	}

	TArray<FNavEdgeSegment> NavEdges;
	if (EdgeProviderOb.IsValid())
	{
		DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Avoidance: collect nav edges"), STAT_AIAvoidanceEdgeCollect, STATGROUP_AI);
		EdgeProviderInterface->GetEdges(AvoidanceData.Center, AvoidanceData.TestRadius2D, NavEdges);
	}

	//Find a good velocity that isn't inside a cone.
	if (AllCones.Num())
	{
		FVector::FReal AngleCurrent;
		FVector::FReal AngleF = ReturnVelocity.HeadingAngle();
		FVector::FReal BestScore = 0.0f;
		FVector::FReal BestScorePotential;
		FVector BestVelocity = FVector::ZeroVector;		//Worst case is we just stand completely still. Should we also allow backing up? Should we test standing still?
		const int AngleCount = 4;		//Every angle will be tested both right and left.
		FVector::FReal AngleOffset[AngleCount] = {FMath::DegreesToRadians<float>(23.0f), FMath::DegreesToRadians<float>(40.0f), FMath::DegreesToRadians<float>(55.0f), FMath::DegreesToRadians<float>(85.0f)};
		FVector AngleVector[AngleCount<<1];

		//Determine check angles
		for (int i = 0; i < AngleCount; ++i)
		{
			AngleCurrent = AngleF - AngleOffset[i];
			AngleVector[(i<<1)].Set(FMath::Cos(AngleCurrent), FMath::Sin(AngleCurrent), 0.0f);
			AngleCurrent = AngleF + AngleOffset[i];
			AngleVector[(i<<1) + 1].Set(FMath::Cos(AngleCurrent), FMath::Sin(AngleCurrent), 0.0f);	
		}

		//Sample velocity-space destination points and drag them back to form lines
		for (int AngleToCheck = 0; AngleToCheck < (AngleCount<<1); ++AngleToCheck)
		{
			FVector VelSpacePoint = AngleVector[AngleToCheck] * MaxSpeed;

			//Skip testing if we know we can't possibly get a better score than what we have already.
			//Note: This assumes the furthest point is the highest-scoring value (i.e. VelSpacePoint is not moving backward relative to ReturnVelocity)
			BestScorePotential = (VelSpacePoint|ReturnVelocity) * (VelSpacePoint|VelSpacePoint);
			if (BestScorePotential > BestScore)
			{
				const bool bAvoidsNavEdges = NavEdges.Num() > 0 ? AvoidsNavEdges(AvoidanceData.Center, VelSpacePoint, NavEdges, AvoidanceData.HalfHeight) : true;
				if (bAvoidsNavEdges)
				{
					FVector CandidateVelocity = AvoidCones(AllCones, FVector::ZeroVector, VelSpacePoint, AllCones.Num());
					FVector::FReal CandidateScore = (CandidateVelocity|ReturnVelocity) * (CandidateVelocity|CandidateVelocity);

					//Vectors are rated by their length and their overall forward movement.
					if (CandidateScore > BestScore)
					{
						BestScore = CandidateScore;
						BestVelocity = CandidateVelocity;
					}
				}
			}
		}
		ReturnVelocity = BestVelocity;
#if WITH_EDITOR
		Debug_RVO(AvoidanceData, ReturnVelocity, DeltaTime);
#endif
	}

	return ReturnVelocity / DeltaTime;		//Remove prediction-time scaling
}

void URogueRVO_Subsystem::SetNavEdgeProvider(INavEdgeProviderInterface* InEdgeProvider)
{
	EdgeProviderInterface = InEdgeProvider;
	EdgeProviderOb = Cast<UObject>(InEdgeProvider);
}

bool URogueRVO_Subsystem::AvoidsNavEdges(const FVector& OrgLocation, const FVector& TestVelocity, const TArray<FNavEdgeSegment>& NavEdges, float MaxZDiff)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Avoidance: avoid nav edges"), STAT_AIAvoidanceEdgeAvoid, STATGROUP_AI);

	for (int32 Idx = 0; Idx < NavEdges.Num(); Idx++)
	{
		const FVector2D Seg0ToSeg1(NavEdges[Idx].P1 - NavEdges[Idx].P0);
		const FVector2D OrgToNewPos(TestVelocity);
		const FVector2D OrgToSeg0(NavEdges[Idx].P0 - OrgLocation);
		const FVector2D::FReal CrossD = FVector2D::CrossProduct(Seg0ToSeg1, OrgToNewPos);
		if (FMath::Abs(CrossD) < UE_KINDA_SMALL_NUMBER)
		{
			continue;
		}

		const FVector2D::FReal CrossS = FVector2D::CrossProduct(OrgToNewPos, OrgToSeg0) / CrossD;
		const FVector2D::FReal CrossT = FVector2D::CrossProduct(Seg0ToSeg1, OrgToSeg0) / CrossD;
		if (CrossS < 0.0f || CrossS > 1.0f || CrossT < 0.0f || CrossT > 1.0f)
		{
			continue;
		}

		const FVector CrossPt = FMath::Lerp(NavEdges[Idx].P0, NavEdges[Idx].P1, CrossS);
		const FVector2D::FReal ZDiff = FMath::Abs(OrgLocation.Z - CrossPt.Z);
		if (ZDiff > MaxZDiff)
		{
			continue;
		}

		return false;
	}
	
	return true;
}

FVector URogueRVO_Subsystem::AvoidCones(TArray<FRogueRVO_VelocityAvoidanceCone>& AllCones, const FVector& BasePosition,
	const FVector& DesiredPosition, const int NumConesToTest)
{
	FVector CurrentPosition = DesiredPosition;
	FVector::FReal DistanceInsidePlane_Current[2];
	FVector::FReal DistanceInsidePlane_Base[2];
	FVector::FReal Weighting[2];
	int ConePlaneIndex;

	//AllCones is non-const so that it can be reordered, but nothing should be added or removed from it.
	checkSlow(NumConesToTest <= AllCones.Num());
	TArray<FRogueRVO_VelocityAvoidanceCone>::TIterator It = AllCones.CreateIterator();
	for (int i = 0; i < NumConesToTest; ++i, ++It)
	{
		const FRogueRVO_VelocityAvoidanceCone& CurrentCone = *It;

		//See if CurrentPosition is outside this cone. If it is, then this cone doesn't obstruct us.
		DistanceInsidePlane_Current[0] = (CurrentPosition|CurrentCone.ConePlane[0]) - CurrentCone.ConePlane[0].W;
		DistanceInsidePlane_Current[1] = (CurrentPosition|CurrentCone.ConePlane[1]) - CurrentCone.ConePlane[1].W;
		if ((DistanceInsidePlane_Current[0] <= 0.0f) || (DistanceInsidePlane_Current[1] <= 0.0f))
		{
			//We're not inside this cone, continue past it.
			//If we wanted to, we could check if CurrentPosition and BasePosition are on the same side of the cone, in which case the cone can be removed from this segment test entirely.
			continue;
		}

		//If we've gotten here, CurrentPosition is inside the cone. If BasePosition is also inside the cone, this entire segment is blocked.
		DistanceInsidePlane_Base[0] = (BasePosition|CurrentCone.ConePlane[0]) - CurrentCone.ConePlane[0].W;
		DistanceInsidePlane_Base[1] = (BasePosition|CurrentCone.ConePlane[1]) - CurrentCone.ConePlane[1].W;

		//We know that the BasePosition isn't in the cone, but CurrentPosition is. We should find the point where the line segment between CurrentPosition and BasePosition exits the cone.
#define CALCULATE_WEIGHTING(index) Weighting[index] = -DistanceInsidePlane_Base[index] / (DistanceInsidePlane_Current[index] - DistanceInsidePlane_Base[index]);
		if (DistanceInsidePlane_Base[0] <= 0.0f)
		{
			CALCULATE_WEIGHTING(0);
			if (DistanceInsidePlane_Base[1] <= 0.0f)
			{
				CALCULATE_WEIGHTING(1);
				ConePlaneIndex = (Weighting[1] > Weighting[0]) ? 1 : 0;
			}
			else
			{
				ConePlaneIndex = 0;
			}
		}
		else if (DistanceInsidePlane_Base[1] <= 0.0f)
		{
			CALCULATE_WEIGHTING(1);
			ConePlaneIndex = 1;
		}
		else
		{
			//BasePosition is also in the cone. This entire line segment of movement is invalidated. I'm considering a way to return false/NULL here.
			return BasePosition;
		}
		//Weighted average of points based on planar distance gives us the answer we want without needing to make a direction vector.
		CurrentPosition = (CurrentPosition * Weighting[ConePlaneIndex]) + (BasePosition * (1.0f - Weighting[ConePlaneIndex]));
#undef CALCULATE_WEIGHTING

		//This cone doesn't need to be used again, so drop it from the list (by shuffling it to the end and decrementing the cone count).
		//This probably ruins our iterator, but it doesn't matter because we're done.
		AllCones.Swap(i, NumConesToTest - 1);		//Don't care if this is swapping an element with itself; the swap function checks this already.
		return AvoidCones(AllCones, BasePosition, CurrentPosition, NumConesToTest - 1);
	}

	return CurrentPosition;
}

#if WITH_EDITOR
bool URogueRVO_Subsystem::IsDebugOnForUID(const int32 AvoidanceUID)
{
	return AvoidanceUID != INDEX_NONE;
}

void URogueRVO_Subsystem::Debug_RVO(const FRogueRVO_NavAvoidanceData& AvoidanceData, const FVector& Velocity, const float DeltaTime) const
{
	if(const URogueRVO_DeveloperSettings* DeveloperSettings = GetDefault<URogueRVO_DeveloperSettings>())
	{
		if(DeveloperSettings->DebugEnabled)
		{
			DrawDebugDirectionalArrow(GetWorld(), AvoidanceData.Center + AvoidanceData.Velocity, AvoidanceData.Center + (Velocity / DeltaTime), 75.0f, FColor(64,255,64), false, 2.0f, SDPG_MAX);
			DrawDebugDirectionalArrow(GetWorld(), AvoidanceData.Center, AvoidanceData.Center + AvoidanceData.Velocity, 2.5f, FColor(0,255,255), false, 0.05f, SDPG_MAX);
		}
	}
}
#endif
