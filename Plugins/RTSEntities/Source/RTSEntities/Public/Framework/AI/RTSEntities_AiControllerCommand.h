// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSEntities_AiControllerBase.h"
#include "RTSEntities_AiControllerCommand.generated.h"

class ARTSEntities_MarkerActor;
class URTSEntities_NavigateTo;
class USplineComponent;
class UNavigationSystemV1;


UCLASS()
class RTSENTITIES_API ARTSEntities_AiControllerCommand : public ARTSEntities_AiControllerBase
{
	GENERATED_BODY()

public:
	ARTSEntities_AiControllerCommand(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void InitAiForGameplay() override;
	
	/** Ai Behaviour **/
protected:
	void BehaviourUpdate_NavigationState();
	void BehaviourUpdate_BehaviourState(const ERTSCore_BehaviourState& StateRequest);
	
	//** End Ai Behaviour **/
	

public:
	/** Command System **/
	virtual bool HasActiveCommand() const { return ActiveCommand != nullptr; }
	virtual bool IsActiveCommand(const FGuid Id) const;
	virtual void ExecuteCommand(URTSEntities_Command* Command);
	virtual void ChangeFormation(const FPrimaryAssetId NewFormation);
	virtual void AbortNavigation();
	
protected:	
	virtual void CompleteCurrentCommand(const ERTSEntities_CommandStatus Status);
	virtual void ExecuteNavigation();
		
	UPROPERTY()
	URTSEntities_Command* ActiveCommand;
	
	UPROPERTY()
	FRTSEntities_Navigation NavigationData;

	FDelegateHandle CompleteCallbackHandle;	
	/** End Command System **/
	

	/** Navigation **/
public:
	virtual bool IsNavigating() const;

protected:
	const UNavigationSystemV1* GetNavSystemChecked() const;
	virtual bool NavigationIsValid();
	virtual void GenerateEntityFormationPath(FRTSEntities_FormationPosition& FormationPosition, TArray<FVector>& EntityPathPoints);
	virtual void GenerateFormationReferencePath(FRTSEntities_FormationPosition& FormationPosition);
	virtual void CalculateOffsetLocation(FVector& ReferencePoint, const FRotator& Rotation, FVector& OffsetLocation, const FVector& Offset);
	virtual int32 GetCurrentDestinationIndex();
	virtual FVector GetCurrentDestination();
	virtual void SetDestination(); 

	// Waypoints
	virtual void GenerateWaypoints(TArray<FVector>& EntityPathPoints);
	virtual int32 GetNextValidWaypointIndex();
	virtual void CreateSplinePath();
	virtual void UpdateSplinePath(const int32 UpdateFromIndex);
	virtual void AssignWaypointCornerSharpness(TArray<FRTSEntities_NavigationWaypoint>& NewWaypoints);
	virtual void SetCurrentWaypoint();
	virtual void BeginNavigatingCurrentWaypoint();
	virtual void HandleWaypointNavigation();
	virtual void ManageFormationPosition();
	virtual void HandleWaypointApproach();
	virtual bool IsApproachingCurrentWaypoint() const;
	virtual void SetCurrentWaypointComplete();
	virtual bool HasReachedDestination() const;
	virtual bool HandleArrivalAtDestination() const { return true; }
	virtual void HandleWaypointNavigationComplete();
	virtual void ResetWaypoints();
	virtual void HandleWaypointsUpdated();
	virtual void UpdateCurrentWaypointIndex(const int32 NewWaypointIndex);
	virtual void CreateTransitionWaypoint(const ERTSEntities_WaypointType& Type);
	
	// Entity Interface
	virtual FVector GetAgentExtent() const;
	virtual float GetMaxWaypointLength();
	virtual float GetSpeed() const;
	virtual float GetMaxSpeed() const;
	virtual float GetAcceptanceSpeed() const;		
	virtual float GetAcceptanceDistance() const;

	UPROPERTY()
	FRTSEntities_EntityPosition EntityPosition;

	UPROPERTY()
	TArray<FRTSEntities_NavigationWaypoint> Waypoints;

	UPROPERTY()
	int32 CurrentWaypointIndex;

	UPROPERTY()
	TObjectPtr<USplineComponent> NavigationSpline;

#if WITH_EDITOR	
	void Debug_Waypoints();
	void Debug_Formation(FVector EntityLocation, FVector SubordinateLocation, const bool bSubordinateInPosition) const;
#endif
	/** End Navigation **/	
};
