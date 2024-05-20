// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSEntities_AiControllerEquipment.h"
#include "RTSEntities_AiControllerVehicle.generated.h"

class USplineComponent;

UCLASS()
class RTSENTITIES_API ARTSEntities_AiControllerVehicle : public ARTSEntities_AiControllerEquipment
{
	GENERATED_BODY()

public:
	ARTSEntities_AiControllerVehicle(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	
	/** Navigation **/
public:
	virtual bool IsNavigating() const override;

	// Waypoints
	virtual void BeginNavigatingCurrentWaypoint() override;
	virtual void HandleWaypointNavigation() override;
	virtual void ManageFormationPositionVehicle();
	virtual bool HandleArrivalAtDestination() const override;
	virtual void HandleWaypointNavigationComplete() override;
	
	// Spline Navigation
	virtual void HandleVehicleNavigation();
	virtual FVector CalculateDesiredVelocity();
	virtual FVector GetAvoidanceVelocity();
	virtual FVector Truncate(const FVector& VectorToTruncate);
	void ApplyVehicleInput() const;
	
	FVector GetClosestPathLocation() const;
	FVector GetPathNavLocation() const;
	FRotator GetRotationToNavPoint() const;
	
	// Entity Interface
	virtual float GetMaxWaypointLength() override;
	virtual float GetMaxSpeed() const override;
	virtual float GetAcceptanceSpeed() const override;
	virtual float GetAcceptanceDistance() const override;	
	float GetArrivalDistance() const;		
	float GetMaxTurnRate() const;	
	float GetMaxDestinationBrake() const;

	UPROPERTY()
	float Mass;

	UPROPERTY()
	float MaxAcceleration;

	UPROPERTY()
	TObjectPtr<UCurveFloat> SteeringCurve;

	UPROPERTY()
	TObjectPtr<UCurveFloat> CollisionSteeringCurve;

	UPROPERTY()
	TObjectPtr<UCurveFloat> SteeringDampeningCurve;

	UPROPERTY()
	TObjectPtr<UCurveFloat> ThrottleCurve;

	UPROPERTY()
	TObjectPtr<UCurveFloat> BrakeCurve;

	UPROPERTY()
	TObjectPtr<UCurveFloat> CollisionBrakeCurve;

	UPROPERTY()
	float CurrentNavSplineDistance;

	UPROPERTY()
	FRTSCore_VehicleInput VehicleInput;

private:
#if WITH_EDITOR
	void Debug_Avoidance(const FVector& FinalVelocity) const;
#endif
	
};
