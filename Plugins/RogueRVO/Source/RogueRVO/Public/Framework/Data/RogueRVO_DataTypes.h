#pragma once

#include "RogueRVO_DataTypes.generated.h"

class IRVOAvoidanceInterface;
class URogueRVO_Subsystem;

struct FRogueRVO_VelocityAvoidanceCone
{
	FPlane ConePlane[2];			//Left and right cone planes - these should point in toward each other. Technically, this is a convex hull, it's just unbounded.
};

USTRUCT()
struct FRogueRVO_NavAvoidanceData
{
	GENERATED_USTRUCT_BODY()
	
	FRogueRVO_NavAvoidanceData(): RemainingTimeToLive(0), Radius(0), HalfHeight(0), Weight(0), OverrideWeightTime(0),
	GroupMask(0), GroupsToAvoid(0), GroupsToIgnore(0), TestRadius2D(0) {}

	FRogueRVO_NavAvoidanceData(const URogueRVO_Subsystem* Manager, IRVOAvoidanceInterface* AvoidanceComp);	
	void Init(const URogueRVO_Subsystem* RVO_Subsystem, IRVOAvoidanceInterface* AvoidanceComp);
	
	/** Current location */
	FVector Center;

	/** Current velocity */
	FVector Velocity;

	/** RVO data is automatically cleared if it's not overwritten first. This makes it easier to use safely. */
	float RemainingTimeToLive;

	/** Radius (object is treated as a cylinder) */
	float Radius;

	/** Height (object is treated as a cylinder) */
	float HalfHeight;

	/** Weight for RVO (set by user) */
	float Weight;

	/** Weight is treated as a hard 1.0 while this is active. This is set by code. */
	double OverrideWeightTime;

	/** Group data */
	int32 GroupMask;

	/** Avoid agents is they belong to one of specified groups */
	int32 GroupsToAvoid;

	/** Do NOT avoid agents is they belong to one of specified groups, takes priority over GroupsToAvoid */
	int32 GroupsToIgnore;

	/** Radius of the area to consider for avoidance */
	float TestRadius2D;	

	FORCEINLINE bool ShouldBeIgnored() const
	{
		return (RemainingTimeToLive <= 0.0f);
	}

	FORCEINLINE bool ShouldIgnoreGroup(int32 OtherGroupMask) const
	{
		return ((GroupsToAvoid& OtherGroupMask) == 0) || ((GroupsToIgnore & OtherGroupMask) != 0);
	}
};

USTRUCT()
struct FRogueRVO_VehicleInput
{
	GENERATED_BODY()

public:
	FRogueRVO_VehicleInput(): Steering(0.f), Throttle(0.f), Braking(0.f) {}
	FRogueRVO_VehicleInput(const float InSteering, const float InThrottle, const float InBraking):
	Steering(InSteering), Throttle(InThrottle), Braking(InBraking) {}

	UPROPERTY()
	float Steering;

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float Braking;	
};

USTRUCT()
struct FRogueRVO_NavData
{
	GENERATED_BODY()

public:
	FRogueRVO_NavData():
		bApproachingDestination(false),
		bSlowForCorner(false),
		bOnPath(false),
		NavTargetLocation(FVector()),
		TurnRatio(0.f),
		DistanceToDestination(0.f)
	{}
	
	FRogueRVO_NavData(
		const bool bIsApproachDest,
		const bool bShouldSlowForCorner,
		const bool bIsOnPath,
		const FVector& TargetLocation,
		const float InTurnRatio,
		const float DistToDest,
		const float InMaxDestinationBrake)
	:		
		bApproachingDestination(bShouldSlowForCorner),
		bSlowForCorner(bIsApproachDest),
		bOnPath(bIsOnPath),
		NavTargetLocation(TargetLocation),
		TurnRatio(InTurnRatio),
		DistanceToDestination(DistToDest)
	{}
	
	UPROPERTY()
	uint8 bApproachingDestination:1;
	
	UPROPERTY()
	uint8 bSlowForCorner:1;
	
	UPROPERTY()
	uint8 bOnPath:1;
	
	UPROPERTY()
	FVector NavTargetLocation;
	
	UPROPERTY()
	float TurnRatio;

	UPROPERTY()
	float DistanceToDestination;
};
