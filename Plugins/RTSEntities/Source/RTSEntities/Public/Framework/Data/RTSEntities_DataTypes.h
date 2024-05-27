#pragma once

#include "Framework/Data/RTSCore_DataTypes.h"
#include "RTSEntities_DataTypes.generated.h"

class AAIController;
class URTSEntities_State;
class URTSEntities_Command;
class ARTSEntities_Group;

UENUM(BlueprintType)
enum class ESubGroupId
{
	GroupA,
	GroupB,
	GroupAll,
	GroupSingle,
	GroupNull
};

UENUM(BlueprintType)
enum class EFormationType : uint8
{
	Wedge,
	Line,
	Column,
	EchelonLeft,
	EchelonRight
};

UENUM()
enum class ERTSEntities_CommandStatus
{
	NoStatus,
	Queued,
	Active,
	Aborted,
	Updating,
	Completed,
	Preview
};

UENUM(BlueprintType)
enum class ERTSEntities_CommandType
{
	Idle,
	NavigateTo,
	NavigateToSlow,
	NavigateToFast,
	Cover
};

UENUM(BlueprintType)
enum class ERTSEntities_CommandCategory
{
	NoCommand,
	Navigation,
	Targeting
};

UENUM(BlueprintType)
enum ERTSEntities_CommandTypeOld
{
	/** 0 **/ Idle,
	/** 1 **/ NavigateTo,
	/** 2 **/ NavigateToSlow,
	/** 3 **/ NavigateToFast,
	/** 4 **/ Defend,
	/** 5 **/ Fallback,
	/** 6 **/ Attack,
	/** 7 **/ AttackIndirect,
	/** 8 **/ Suppress,
	/** 9 **/ Flank,
	/** 10 **/ Orientate,
	/** 11 **/ Focus,
	/** 12 **/ Regroup
};

UENUM(BlueprintType)
enum class ERTSEntities_NavigationType
{
	NoFormation,
	Formation	
};

UENUM(BlueprintType)
enum class ERTSEntities_WaypointType
{
	NoWaypoint,
	Nav,
	Start,
	FormUp,
	Destination
};

UENUM(BlueprintType)
enum class ERTSEntities_SelectionSide
{
	NoSide,
	LeftSide,
	RightSide,
	CenterSide
};

UENUM(BlueprintType)
enum class ERTSEntities_SelectionMarkerType
{
	NoSelectionMarker,
	Decal,
	NiagaraSystem
};

UENUM()
enum class ERTSEntities_SpacingType
{
	SpacingChangeNone,
	EntityChange,
	GroupChange,
	SpacingChangeBoth
};


USTRUCT(BlueprintType)
struct FRTSEntities_PlayerSelection
{
	GENERATED_BODY()

public:
	FRTSEntities_PlayerSelection(): Lead(nullptr), Group(nullptr) {}
	FRTSEntities_PlayerSelection(
		AActor* InLead,
		ARTSEntities_Group* InGroup,
		const TArray<AActor*>& InEntities
	):
		Lead(InLead),
		Group(InGroup),
		Entities(InEntities)
	{}

	UPROPERTY()
	AActor* Lead;
	
	UPROPERTY()
	ARTSEntities_Group* Group;

	UPROPERTY()
	TArray<AActor*> Entities;
	
	bool IsValid() const { return Group != nullptr; }
	FVector GetSelectionCenterPosition() const;
	static AActor* GetClosestMemberToPosition(const TArray<AActor*>& GroupMembers, const FVector& Destination);
	float GetMaxFormUpDistance() const;
	void UpdateGroupLead();
	void SetSelectionFormation(const FPrimaryAssetId& Formation);	
	
	bool operator==(const FRTSEntities_PlayerSelection& Other) const
	{
		return Group == Other.Group;
	}
	
	bool operator==(const ARTSEntities_Group* OtherGroup) const
	{
		return Group == OtherGroup;
	}
};

USTRUCT()
struct FRTSEntities_EntityPosition
{
	GENERATED_BODY()

public:
	FRTSEntities_EntityPosition():
		Owner(nullptr),
		Destination(FVector::ZeroVector),
		FormationDestination(FVector::ZeroVector),
		Rotation(FRotator::ZeroRotator),
		Side(ERTSEntities_SelectionSide::NoSide),
		Spacing(0),
		FormUpIndex(0)
	{}

	FRTSEntities_EntityPosition(
		const FVector& InLocation,
		const FVector& InFormationLocation,
		const FRotator& InRotation,
		const ERTSEntities_SelectionSide InSide
	):
		Owner(nullptr),
		Destination(InLocation),
		FormationDestination(InFormationLocation),
		Rotation(InRotation),
		Side(InSide),
		Spacing(0.f),
		FormUpIndex(0)
	{}

	UPROPERTY()
	AActor* Owner;	

	UPROPERTY()
	FVector Destination;

	UPROPERTY()
	FVector FormationDestination;

	UPROPERTY()
	FRotator Rotation;
	
	UPROPERTY()
	ERTSEntities_SelectionSide Side;
	
	UPROPERTY()
	float Spacing;

	UPROPERTY()
	int32 FormUpIndex;

	bool IsValid() const { return Owner != nullptr; }
};

USTRUCT()
struct FRTSEntities_FormationPosition
{
	GENERATED_BODY()

public:
	FRTSEntities_FormationPosition():
		Destination(FVector::ZeroVector),
		Rotation(FRotator::ZeroRotator),
		SourceLocation(FVector::ZeroVector),
		Offset(FVector::ZeroVector),
		Side(ERTSEntities_SelectionSide::NoSide),
		Owner(nullptr),
		SelectionSpacing(0.f),
		UseFormation(0)
	{}

	FRTSEntities_FormationPosition(
		const FVector& InLocation,
		const FRotator& InRotation,
		const FVector& InOffset,
		const ERTSEntities_SelectionSide InSide
	):
		Destination(InLocation),
		Rotation(InRotation),
		SourceLocation(FVector::ZeroVector),
		Offset(InOffset), 
		Side(InSide),
		Owner(nullptr),
		SelectionSpacing(0.f),
		UseFormation(0)
	{}
	
	UPROPERTY()
	FVector Destination;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector SourceLocation;

	UPROPERTY()
	FVector Offset;
	
	UPROPERTY()
	ERTSEntities_SelectionSide Side;

	UPROPERTY()
	AActor* Owner;

	UPROPERTY()
	FRTSEntities_PlayerSelection Selection;

	UPROPERTY()
	float SelectionSpacing;

	UPROPERTY()
	TArray<FRTSEntities_EntityPosition> EntityPositions;

	UPROPERTY()
	TArray<FVector> ReferencePathPoints;

	UPROPERTY()
	uint8 UseFormation:1;

	bool IsValid() const
	{
		return !Destination.Equals(FVector::ZeroVector, 0.0001f) && Side != ERTSEntities_SelectionSide::NoSide;
	}
};

USTRUCT()
struct FRTSEntities_Navigation
{
	GENERATED_BODY()

public:
	FRTSEntities_Navigation(): 	
		FormationId(FPrimaryAssetId()),
		Location(FVector::ZeroVector),
		Rotation(FRotator::ZeroRotator),
		GroupSpacing(0.f),
		EntitySpacing(0.f),
		FormationThreshold(1000.f),
		Positions(TArray<FRTSEntities_FormationPosition>{}),
		Offset(FVector::ZeroVector)
	{}

	explicit FRTSEntities_Navigation(
		const FPrimaryAssetId& InFormationId,
		const float InGroupSpacing,
		const float InEntitySpacing,
		const float InFormationThreshold,
		const FVector& InLocation,
		const FRotator& InRotation
	): 	
        FormationId(InFormationId),
		Location(InLocation),
		Rotation(InRotation),
        GroupSpacing(InGroupSpacing),
		EntitySpacing(InEntitySpacing),
		FormationThreshold(InFormationThreshold),
		Positions(TArray<FRTSEntities_FormationPosition>{}),
        Offset(FVector::ZeroVector)
	{}

	UPROPERTY()
	FPrimaryAssetId FormationId;

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	float GroupSpacing;

	UPROPERTY()
	float EntitySpacing;

	UPROPERTY()
	float FormationThreshold;
	
	UPROPERTY()
	TArray<FRTSEntities_FormationPosition> Positions;

	UPROPERTY()
	FVector Offset;

	bool IsValid() const { return FormationId.IsValid(); }
};

USTRUCT()
struct FRTSEntities_PlayerSelections
{
	GENERATED_BODY()

public:
	FRTSEntities_PlayerSelections():
		Selections(TArray<FRTSEntities_PlayerSelection>()),
		LeadSelectionIndex(-1),
		GroupFormationPositions(TArray<FRTSEntities_FormationPosition>())
	{}

	UPROPERTY()
	TArray<FRTSEntities_PlayerSelection> Selections;

	UPROPERTY()
	int32 LeadSelectionIndex;
	
	UPROPERTY()
	TArray<FRTSEntities_FormationPosition> GroupFormationPositions;

	bool IsValid() const;	
	void Clear() { Selections.Empty(); LeadSelectionIndex = -1; }
	TArray<FPrimaryAssetId> GetFormationData() const;
	FPrimaryAssetId GetFormation() const;
	FPrimaryAssetId GetNextFormation(const float Input) const;
	ARTSEntities_Group* GetLeadGroup() const;
};

USTRUCT(BlueprintType)
struct FRTSEntities_ClientCommandData
{
	GENERATED_BODY()
	
public:
	FRTSEntities_ClientCommandData():
		CommandType(ERTSEntities_CommandType::Idle),
		HasNavigation(0),
		TargetTransform(FTransform::Identity),
		SourceTransform(FTransform::Identity),
		BehaviourState(ERTSCore_BehaviourState::Safe)

	{}
	
	FRTSEntities_ClientCommandData(
		const ERTSEntities_CommandType& Type,
		const uint8 HasNavigation
	):
		CommandType(Type),
		HasNavigation(HasNavigation),
		TargetTransform(FTransform::Identity),
		SourceTransform(FTransform::Identity),
		BehaviourState(ERTSCore_BehaviourState::Safe)
	{}

	UPROPERTY()
	ERTSEntities_CommandType CommandType;

	UPROPERTY()
	uint8 HasNavigation:1;

	UPROPERTY()
	FTransform TargetTransform;

	UPROPERTY()
	FTransform SourceTransform;

	UPROPERTY()
	ERTSCore_BehaviourState BehaviourState;

	bool IsValid() const { return CommandType != ERTSEntities_CommandType::Idle; }
	void SetTargetLocation(const FVector& Location) { TargetTransform.SetLocation(Location); }
	void SetTargetRotation(const FRotator& Rotation) { TargetTransform.SetRotation(Rotation.Quaternion()); }
	void SetSourceLocation(const FVector& Location) { SourceTransform.SetLocation(Location); }
	void SetSourceRotation(const FRotator& Rotation) { SourceTransform.SetRotation(Rotation.Quaternion()); }
};

USTRUCT(BlueprintType)
struct FRTSEntities_CommandData
{
	GENERATED_BODY()
	
public:
	FRTSEntities_CommandData():
		Id(FGuid()),
		CommandType(ERTSEntities_CommandType::Idle),
		Queue(0),
		Status(ERTSEntities_CommandStatus::NoStatus),
		TargetTransform(FTransform::Identity),
		SourceTransform(FTransform::Identity),
		LeadSelectionIndex(-1),
		Selected(FRTSEntities_PlayerSelections()),
		TargetActor(nullptr),
		HasNavigation(false),
		BehaviourState(ERTSCore_BehaviourState::Safe)
	{}

	UPROPERTY()
	FGuid Id;

	UPROPERTY()
	ERTSEntities_CommandType CommandType;

	UPROPERTY()
	uint8 Queue:1;
	
	UPROPERTY()
	ERTSEntities_CommandStatus Status;	

	UPROPERTY()
	FTransform TargetTransform;

	UPROPERTY()
	FTransform SourceTransform;

	UPROPERTY()
	int32 LeadSelectionIndex;	

	UPROPERTY()
	FRTSEntities_PlayerSelections Selected;

	UPROPERTY()
	AActor* TargetActor;

	UPROPERTY()
	uint8 HasNavigation:1;

	UPROPERTY()
	FRTSEntities_Navigation Navigation;

	UPROPERTY()
	ERTSCore_BehaviourState BehaviourState;

	bool IsValid() const { return CommandType != ERTSEntities_CommandType::Idle; }
	void ApplyClientData(const FRTSEntities_ClientCommandData& ClientData);
	void SetSelected(const FRTSEntities_PlayerSelections& InSelected);
	void AssignSelectionData();
	void GetCommandCenterLocation(FVector& CenterLocation) const;
	FVector GetLocation() const { return TargetTransform.GetLocation(); }
	FRotator GetRotation() const { return TargetTransform.GetRotation().Rotator(); }
};


USTRUCT(BlueprintType)
struct FRTSEntities_EntityConfigData
{
	GENERATED_BODY()

public:
	FRTSEntities_EntityConfigData(): EntityData(FPrimaryAssetId()), Index(-1), SubGroupId(0){}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowedTypes="EntityData"))
	FPrimaryAssetId EntityData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Index;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SubGroupId;
};

USTRUCT()
struct FRTSEntities_InitialisationData
{
	GENERATED_BODY()

public:
	FRTSEntities_InitialisationData(): Entity(nullptr), EntityConfigData(FRTSEntities_EntityConfigData()), SelectionMarkerType(ERTSEntities_SelectionMarkerType::Decal), SelectionMarkerRadius(0.f), Group(nullptr), DefaultFormation(FPrimaryAssetId()) {}
	FRTSEntities_InitialisationData(AActor* InEntity, const FRTSEntities_EntityConfigData& InData, const ERTSEntities_SelectionMarkerType SelectionMarkerType, const float InSelectionMarkerRadius, ARTSEntities_Group* InGroup, const FPrimaryAssetId& InFormation)
	: Entity(InEntity), EntityConfigData(InData), SelectionMarkerType(SelectionMarkerType), SelectionMarkerRadius(InSelectionMarkerRadius), Group(InGroup), DefaultFormation(FPrimaryAssetId(InFormation)) {}

	UPROPERTY()
	AActor* Entity;

	UPROPERTY()
	FRTSEntities_EntityConfigData EntityConfigData;
	
	UPROPERTY()
	ERTSEntities_SelectionMarkerType SelectionMarkerType;
	
	UPROPERTY()
	float SelectionMarkerRadius;
	
	UPROPERTY()
	ARTSEntities_Group* Group;
	
	UPROPERTY()
	FPrimaryAssetId DefaultFormation;

	bool IsValid() const { return Entity != nullptr && Group != nullptr; }
};

USTRUCT(BlueprintType)
struct FRTSEntities_NavigationWaypoint
{
	GENERATED_BODY()

public:
	FRTSEntities_NavigationWaypoint():
		WaypointType(ERTSEntities_WaypointType::NoWaypoint),
		Location(FVector::ZeroVector),
		Rotation(FRotator::ZeroRotator),
		bIsCompleted(false),
		DistanceToCompletion(0.f),
		CornerSharpness(0.f),
		MoveType(ERTSEntities_NavigationType::NoFormation)
	{}
	FRTSEntities_NavigationWaypoint(const ERTSEntities_WaypointType InWaypointType, const FVector& InWaypointLocation, const FRotator& InWaypointRotation,
		const ERTSEntities_NavigationType InMoveType) :
		WaypointType(InWaypointType),
		Location(InWaypointLocation),
		Rotation(InWaypointRotation),
		bIsCompleted(false),
		DistanceToCompletion(0.f),
		CornerSharpness(0.f),
		MoveType(InMoveType)
	{}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERTSEntities_WaypointType WaypointType;
	
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
	ERTSEntities_NavigationType MoveType;
	
	bool IsCompleted() const { return bIsCompleted; }
	bool IsValid() const { return WaypointType != ERTSEntities_WaypointType::NoWaypoint; }
	bool UseFormation() const { return MoveType == ERTSEntities_NavigationType::Formation; }
	void Completed() { bIsCompleted = true; }
	void UpdateDistanceToCompletion(const FVector& CurrentLocation) { DistanceToCompletion = (CurrentLocation - Location).Length(); }
};

USTRUCT()
struct FRTSEntities_GroupState
{
	GENERATED_BODY()

public:
	FRTSEntities_GroupState():
		Navigation(ERTSCore_NavigationState::Idle),
		Behaviour(ERTSCore_BehaviourState::NoBehaviour),
		Speed(ERTSCore_SpeedState::NoSpeed),
		Condition(ERTSCore_ConditionState::NoCondition),
		Posture(ERTSCore_PostureState::NoPosture)
	{}	

	UPROPERTY()
	ERTSCore_NavigationState Navigation;
	
	UPROPERTY()
	ERTSCore_BehaviourState Behaviour;

	UPROPERTY()
	ERTSCore_SpeedState Speed;

	UPROPERTY()
	ERTSCore_ConditionState Condition;

	UPROPERTY()
	ERTSCore_PostureState Posture;
	
	void SetState(const ERTSCore_StateCategory Category, const int32 State);
	int32 GetState(const ERTSCore_StateCategory Category);
};

USTRUCT()
struct FRTSEntities_TargetData
{
	GENERATED_BODY()
	
public:
	FRTSEntities_TargetData():
		TargetActor(nullptr),
		Source(nullptr),
		LastSeenEnemyTime(0),
		LastSeenEnemyLocation(FVector::ZeroVector),
		ThreatLevel(ERTSCore_ThreatLevel::NoThreat)
	{}
	FRTSEntities_TargetData(
		AActor* NewTarget,
		AAIController* Reporter,
		const float Time,
		const FVector& Location
	):
		TargetActor(NewTarget),
		Source(Reporter),
		LastSeenEnemyTime(Time),
		LastSeenEnemyLocation(Location),
		ThreatLevel(ERTSCore_ThreatLevel::NoThreat)
	{}

	UPROPERTY()
	AActor* TargetActor;
	
	UPROPERTY()
	AAIController* Source;
	
	UPROPERTY()
	float LastSeenEnemyTime;

	UPROPERTY()
	FVector LastSeenEnemyLocation;

	UPROPERTY()
	ERTSCore_ThreatLevel ThreatLevel;

	bool operator==(const FRTSEntities_TargetData& Other) const
	{
		return TargetActor == Other.TargetActor;
	}

	bool IsValid() const { return TargetActor != nullptr; }
};
