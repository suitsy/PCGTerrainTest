// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSEntities_AiControllerEquipment.h"
#include "RTSEntities_AIControllerCharacter.generated.h"


UCLASS()
class RTSENTITIES_API ARTSEntities_AIControllerCharacter : public ARTSEntities_AiControllerEquipment
{
	GENERATED_BODY()

public:
	ARTSEntities_AIControllerCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaTime) override;

	/** Navigation **/
	virtual bool IsNavigating() const override;

protected:
	virtual void BeginPlay() override;

	// Waypoints
	virtual void BeginNavigatingCurrentWaypoint() override;
	virtual void HandleWaypointNavigation() override;
	virtual void ManageFormationPosition() override;
	virtual void HandleWaypointApproach() override;
	virtual bool HandleArrivalAtDestination() const override;
	virtual void HandleWaypointNavigationComplete() override;
	
	// Entity Interface
	virtual float GetMaxWaypointLength() override;
	virtual bool HasReachedDestination() const override;
};
