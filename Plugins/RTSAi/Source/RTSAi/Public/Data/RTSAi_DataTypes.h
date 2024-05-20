#pragma once

#include "AIController.h"
#include "RTSAi_DataTypes.generated.h"


UENUM(BlueprintType)
enum class ERTSAi_StateCategory : uint8
{
	Behaviour,
	Speed,
	Condition,
	Posture,
	Navigation
};

UENUM(BlueprintType)
enum class ERTSAi_BehaviourState : uint8
{
	NoBehaviour,
	Stealth,
	Safe,
	Cautious,
	Combat
};

UENUM(BlueprintType)
enum class ERTSAi_SpeedState : uint8
{
	NoSpeed,
	Prone,
	Crouch,
	Walk,
	Run,
	Sprint
};

UENUM(BlueprintType)
enum class ERTSAi_ConditionState : uint8
{
	NoCondition,
	Normal,
	Injured,
	Restricted,
	Incapacitated,
	Dead
};

UENUM(BlueprintType)
enum class ERTSAi_StanceState : uint8
{
	Standing,
	Crouch,
	Prone
};

UENUM(BlueprintType)
enum class ERTSAi_PostureState : uint8
{
	NoPosture,
	Withdrawing,
	Defensive,
	Aggressive,
	Offensive
};

UENUM(BlueprintType)
enum class ERTSAi_NavigationState : uint8
{
	NavOff,
	Idle,
	Navigating,
	Avoiding,
	ReNavigating
};

UENUM(BlueprintType)
enum class ERTSAi_AmmoState : uint8
{
	NoAmmo,
	HasAmmo,
	Reloading
};

UENUM(BlueprintType)
enum class ERTSAi_CoverState : uint8
{
	NoCover,
	Partial,
	Full
};

UENUM(BlueprintType)
enum class ERTSAi_CollisionType : uint8
{
	CollisionTypeNone,
	SameDirection,
	Perpendicular,
	Incoming
};

UENUM(BlueprintType)
enum class ERTSAi_CollisionMode
{
	NoCollision,
	Braking,
	Avoiding,
	EmergencyStop,
	CollisionBackOff
};

UENUM(BlueprintType)
enum ERTSAi_SelectionSide
{
	NoSide,
	LeftSide,
	RightSide,
	CenterSide
};

UENUM(BlueprintType)
enum ERTSAi_WaypointType
{
	NoWaypoint,
	Nav,
	Start,
	FormUp,
	Destination
};

UENUM(BlueprintType)
enum ERTSAi_MoveType
{
	NoFormation,
	Formation	
};

USTRUCT()
struct FRTSAi_EnemyPerception
{
	GENERATED_BODY()

public:
	FRTSAi_EnemyPerception(): LastSeenTime(0), LastSeenLocation(FVector::ZeroVector), KnownEnemies(TArray<AActor*>()) {}

	UPROPERTY()
	float LastSeenTime;

	UPROPERTY()
	FVector LastSeenLocation;

	UPROPERTY()
	TArray<AActor*> KnownEnemies;
};

USTRUCT()
struct FRTSAi_GroupPosition
{
	GENERATED_BODY()

	FRTSAi_GroupPosition(): Location(FVector::ZeroVector), Index(0), Side(ERTSAi_SelectionSide::NoSide), Spacing(0)	{}
	FRTSAi_GroupPosition(const FVector InLocation, const int32 InIndex): Location(InLocation), Index(InIndex), Side(), Spacing(0) {}
	FRTSAi_GroupPosition(const FVector InLocation, const int32 InIndex, const TEnumAsByte<ERTSAi_SelectionSide> InSide ):
	Location(InLocation), Index(InIndex), Side(InSide), Spacing(0) {}

	UPROPERTY()
	FVector Location;
	
	UPROPERTY()
	int32 Index;
	
	UPROPERTY()
	TEnumAsByte<ERTSAi_SelectionSide> Side;

	UPROPERTY()
	float Spacing;
};

USTRUCT(BlueprintType)
struct FRTSAi_MemberNavigationData
{
	GENERATED_BODY()

public:
	FRTSAi_MemberNavigationData():
		Entity(nullptr),
		CommandId(FGuid()),
		SourceLocation(FVector::ZeroVector),
		SourceRotation(FRotator::ZeroRotator),
		DestinationLocation(FVector::ZeroVector),
		DestinationRotation(FRotator::ZeroRotator),
		GuidePathPoints(TArray<FVector>())
	{}
	FRTSAi_MemberNavigationData(
		AActor* InEntity,
		const FGuid InCommandId,
		const FVector& InSourceLocation,
		const FRotator& InSourceRotation,
		const FVector& InDestinationLocation,
		const FRotator& InDestinationRotation
	):
		Entity(InEntity),
		CommandId(InCommandId),
		SourceLocation(InSourceLocation),
		SourceRotation(InSourceRotation),
		DestinationLocation(InDestinationLocation),
		DestinationRotation(InDestinationRotation),
		GuidePathPoints(TArray<FVector>())
	{}

	UPROPERTY()
	AActor* Entity;

	UPROPERTY()
	FGuid CommandId;

	UPROPERTY()
	FVector SourceLocation;

	UPROPERTY()
	FRotator SourceRotation;
	
	UPROPERTY()
	FVector DestinationLocation;

	UPROPERTY()
	FRotator DestinationRotation;
	
	UPROPERTY()
	FRTSAi_GroupPosition FormationPosition;

	UPROPERTY()
	TArray<FVector> GuidePathPoints;

	bool IsValid() const { return GuidePathPoints.Num() > 1; }
};

USTRUCT(BlueprintType)
struct FRTSAi_NavigationWaypoint
{
	GENERATED_BODY()

public:
	FRTSAi_NavigationWaypoint():
		WaypointType(ERTSAi_WaypointType::NoWaypoint),
		Location(FVector::ZeroVector),
		Rotation(FRotator::ZeroRotator),
		bIsCompleted(false),
		DistanceToCompletion(0.f),
		CornerSharpness(0.f),
		MoveType(ERTSAi_MoveType::NoFormation)
	{}
	FRTSAi_NavigationWaypoint(const TEnumAsByte<ERTSAi_WaypointType> InWaypointType, const FVector& InWaypointLocation, const FRotator& InWaypointRotation,
		const TEnumAsByte<ERTSAi_MoveType> InMoveType = ERTSAi_MoveType::Formation) :
		WaypointType(InWaypointType),
		Location(InWaypointLocation),
		Rotation(InWaypointRotation),
		bIsCompleted(false),
		DistanceToCompletion(0.f),
		CornerSharpness(0.f),
		MoveType(InMoveType)
	{}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ERTSAi_WaypointType> WaypointType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceToCompletion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CornerSharpness;

	/** Formation Data **/	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ERTSAi_MoveType> MoveType;
	
	bool IsCompleted() const { return bIsCompleted; }
	bool IsValid() const { return WaypointType != ERTSAi_WaypointType::NoWaypoint; }
	bool UseFormation() const { return MoveType == ERTSAi_MoveType::Formation; }
	void Completed() { bIsCompleted = true; }
	void UpdateDistanceToCompletion(const FVector& CurrentLocation) { DistanceToCompletion = (CurrentLocation - Location).Length(); }
};

USTRUCT()
struct FRTSAi_CollisionMemory
{
	GENERATED_BODY()

public:
	FRTSAi_CollisionMemory(): Entity(nullptr), DirectAvoidance(false) {}
	FRTSAi_CollisionMemory(AActor* InEntity): Entity(InEntity), DirectAvoidance(true) {}

	UPROPERTY()
	AActor* Entity;
	
	UPROPERTY()
	bool DirectAvoidance;
	
	/*FRTSAi_CollisionMemory(): Entity(nullptr), LastSeen(0), Direction(FVector::ZeroVector), Angle(0.f), Side(0.f), ActiveAvoidance(false) {}
	FRTSAi_CollisionMemory(AActor* InEntity, const float SeenTime, const FVector& InDirection, const float InAngle, const float InSide, const bool InActive):
	Entity(InEntity), LastSeen(SeenTime), Direction(InDirection), Angle(InAngle), Side(InSide), ActiveAvoidance(InActive) {}

	UPROPERTY()
	AActor* Entity;

	UPROPERTY()
	float LastSeen;

	UPROPERTY()
	FVector Direction;

	UPROPERTY()
	float Angle;

	UPROPERTY()
	float Side;

	UPROPERTY()
	bool ActiveAvoidance;*/
};

USTRUCT()
struct FRTSAi_VehicleInput
{
	GENERATED_BODY()

public:
	FRTSAi_VehicleInput(): Steering(0.f), Throttle(0.f), Braking(0.f) {}
	FRTSAi_VehicleInput(const float InSteering, const float InThrottle, const float InBraking):
	Steering(InSteering), Throttle(InThrottle), Braking(InBraking) {}

	UPROPERTY()
	float Steering;

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float Braking;	
};
