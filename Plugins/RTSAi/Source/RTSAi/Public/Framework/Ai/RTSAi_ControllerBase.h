// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RTSAi_ControllerBase.generated.h"



//DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnEntityCompletedCommandDelegate, FGuid, AActor*, bool);
//DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnEntityAbortedCommandDelegate, FGuid, AActor*, bool);

UCLASS()
class RTSAI_API ARTSAi_ControllerBase : public AAIController
{
	GENERATED_BODY()

public:
	ARTSAi_ControllerBase(const FObjectInitializer& ObjectInitializer);
	/*virtual void Tick(float DeltaTime) override;
	virtual void SetAiData(const FPrimaryAssetId& BehaviourAssetId);	
	virtual URTSAi_DataAsset* GetAiData();
	virtual void SetGroup(AActor* AiGroup) { Group = AiGroup; }

	// State 
	int32 GetState(const ERTSAi_StateCategory Category);
	virtual void SetState(const ERTSAi_StateCategory Category, const int32 NewState);

	// IRTSCore_AiControllerInterface
	virtual int32 GetBehaviourState() override;
	virtual int32 GetSpeedState() override;
	virtual int32 GetConditionState() override;
	virtual int32 GetPostureState() override;
	virtual int32 GetNavigationState() override;
	virtual void RequestReload() override;
	virtual bool CanUpdateTarget() override;
	virtual void AssignEquipment(UActorComponent* NewEquipment) override { Equipment = NewEquipment; }
	virtual UActorComponent* GetEquipment() const override { return Equipment; }
	virtual void OnEquipmentCreated() override;	
	virtual TArray<FRotator> GetTurretRotations() const override;
	virtual TArray<FRotator> GetWeaponRotations() const override;

	virtual void ExecuteNavigationCommand(const FRTSAi_MemberNavigationData& EntityNavigationData);*/
	// End IRTSCore_AiControllerInterface
	
protected:
	/*virtual void BeginPlay() override;	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void LoadAiData();
	virtual void OnAiDataLoaded();
	virtual void InitAiForGameplay();
	
	UPROPERTY()
	AActor* Group;

	/** State *#1#
	virtual void InitialiseAiBehaviour(const URTSAi_DataAsset* BehaviourData);	
	
	UPROPERTY()
	FPrimaryAssetId AiDataAsset;

	UPROPERTY()
	UBehaviorTree* BehaviourTreeAsset;	

	UPROPERTY(Replicated)
	FRTSAi_State State;*/

private:	
	/*UPROPERTY()
	bool bAiInitialised = false;

	/** Navigation *#1#
public:
	virtual void AbortNavigation(const FGuid CommandId);
	virtual bool IsNavigating() const;
	virtual void UpdateNavigation(const TArray<FRTSAi_MemberNavigationData>& InGroupNavData, const int32 InNavIndex);

	// Delegates
	FOnEntityCompletedCommandDelegate OnEntityCompletedCommand;
	FOnEntityAbortedCommandDelegate OnEntityAbortedCommand;
	
protected:
	virtual bool HasValidNavigation();

	// Waypoint Creation
	virtual void GenerateNavigationPath();	
	virtual void CalculateOffsetLocation(FVector& ReferencePoint, FVector& OffsetLocation);
	virtual TArray<FRTSAi_NavigationWaypoint> GenerateWaypoints(const TArray<FVector>& EntityPathPoints);
	virtual void ValidateLocation(FVector& OffsetLocation);
	virtual void GenerateCompletePath(const TArray<FVector>& EntityPathPoints, TArray<FVector>& CompletePathPoints, int32& FormUpIndex);
	virtual void GenerateStandOffLocation(FVector& StandoffLocation, const FVector& FirstNavPoint);
	virtual void CreateSplinePath();	
	virtual void AssignNavigationCornerSharpness();
	virtual void ResetWaypoints();

	// Waypoint Navigation
	virtual void SetCurrentWaypoint();
	virtual void BeginNavigatingCurrentWaypoint();
	virtual void HandleWaypointNavigation();
	virtual bool HandleArrivalAtDestination();
	virtual void HandleWaypointNavigationComplete();
	virtual void ManageFormationPosition();
	virtual void HandleWaypointApproach();
	virtual void SetCurrentWaypointComplete();
	virtual bool GetMovementDirection() const;		
	virtual bool IsApproachingCurrentWaypoint() const;
	virtual bool HasReachedDestination() const;	
	
	// Entity Interface
	virtual FVector GetAgentExtent() const;
	virtual float GetMaxWaypointLength();
	virtual FVector GetFormationOffset() const;
	virtual float GetStandOffRadius() const;
	virtual bool ShouldUseFormation() const;
	virtual float GetSpeed() const;
	virtual float GetMaxSpeed() const;
	virtual float GetAcceptanceSpeed() const;		
	virtual float GetAcceptanceDistance() const;

	// Behaviour
	void BT_UpdateCurrentWaypoint();
	void BT_UpdateDestination();
	void BT_SetDestination(const FVector& NewDestination);
	FVector BT_GetDestination() const;
	void BT_UpdateNavigationState();
	void BT_SetNavigationStateAvoidance();
	void BT_SetNavigationStateReNav();
	
	UPROPERTY(ReplicatedUsing = OnRep_NavData)
	TArray<FRTSAi_MemberNavigationData> NavData;
	
	UPROPERTY(ReplicatedUsing = OnRep_NavData)
	FRTSAi_MemberNavigationData NavigationData;

	UPROPERTY(Replicated)
	int32 NavIndex;

	UPROPERTY(ReplicatedUsing = OnRep_Waypoints)
	TArray<FRTSAi_NavigationWaypoint> Waypoints;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWaypointIndex)
	int32 CurrentWaypointIndex;

	UPROPERTY()
	TObjectPtr<USplineComponent> NavigationSpline;

	UPROPERTY()
	float CurrentNavSplineDistance;
	
	UPROPERTY()
	UNavigationSystemV1* NavSystem;
	
	UPROPERTY()
	bool bReverse;*/

/*#if WITH_EDITOR	
	void Debug_MemberNav() const;
	void Debug_RawPathPoints(const TArray<FVector>& PathPoints) const;
	void Debug_GuidePath() const;
	void Debug_EntityPath(const TArray<FVector>& EntityPoints) const;
	void Debug_Waypoints();
	void Debug_Formation(FVector EntityLocation, FVector SubordinateLocation, const bool bSubordinateInPosition) const;
	void Debug_EntityGroupIndex() const;
#endif*/

private:	
	/*UFUNCTION()
	void OnRep_NavData();
	
	UFUNCTION()
	void OnRep_Waypoints();
	
	UFUNCTION()
	void OnRep_CurrentWaypointIndex();*/

	
	/** Perception **/
public:	
	/*float GetLastSeenTargetTime() const { return LastSeenTargetTime; }
	
protected:
	void SetupPerceptionSystem();
	void InitialisePerceptionSight(const URTSAi_DataAsset* BehaviourData);
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	void StartUpdatingPerception();
	void StopUpdatingPerception();

	UFUNCTION()
	void UpdatePerception();	

	UFUNCTION()
	void UpdatePerceivedActors(const TArray<AActor*>& UpdatedActors);
	
	void ReportHostiles(const TArray<AActor*>& Hostiles);
		
	UPROPERTY()
	UAISenseConfig_Sight* SightConfig;
		
	UPROPERTY()
	TMap<AActor*, float> PerceivedEntities;

	UPROPERTY(Replicated)
	float LastSeenTargetTime;

protected:	
	UPROPERTY()
	FTimerHandle Handle_UpdatingPerception;

	/** Collision Handling *#1#	
protected:	
	virtual bool SweepForCollision(float& HitRange);
	virtual FCollisionShape GetCollisionShape() const override;
	virtual void SetCollisionType(ERTSAi_CollisionType& CollisionType, const float DotProduct);
	virtual int32 GetClosestWaypointIndex(const FVector& Location, const int32 StartIndex);

	UPROPERTY()
	ERTSAi_CollisionMode CollisionMode;	

	/** Combat *#1#
public:
	void AssignTargetData(const FRTSAi_TargetData& TargetData);	
	FVector GetTargetLastKnownLocation() const;
	bool HasTarget();
	
	/** Check if we have a LOS and Range to current target, if not fall back to last know location *#1#
	bool CanEngageTarget();
	bool ValidateCover();
	void FireOnTarget();
	void ReloadWeapon();	
	void SetAmmunitionState(const ERTSAi_AmmoState AmmoState);
	bool HasLineOfSight(const FVector& Location) const;
	
protected:
	void UpdateTargetData();
	void UpdateTargetLastKnownLocation();	
	bool TargetInRange(const FVector& TargetLocation) const;

	UFUNCTION()
	void FireWeaponTimer();

	UFUNCTION()
	void ReloadTimer();
	
	UPROPERTY()
	float LastAssignedTargetTime;
	
	UPROPERTY()
	FRTSAi_TargetData CurrentTarget;

	UPROPERTY()
	FTimerHandle TimerHandle_FireWeapon;

	UPROPERTY()
	FTimerHandle TimerHandle_Reload;

	/** Equipment *#1#
public:
	int32 GetCurrentWeaponAmmo() const;
	bool HasInventoryAmmoForCurrentWeapon() const;
	float GetCurrentWeaponFireRate() const;
	float GetCurrentWeaponReloadRate() const;
	float GetCurrentWeaponAccuracy() const;	

protected:	
	UPROPERTY()
	UActorComponent* Equipment;*/

	

/*#if WITH_EDITOR
	/** Debug *#1#	
private:
	void DebugTargets(const AActor* Other) const;
	void DebugTarget();
#endif*/
	
};
