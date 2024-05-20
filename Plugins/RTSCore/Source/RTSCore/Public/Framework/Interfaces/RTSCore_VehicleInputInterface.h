// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RTSCore_VehicleInputInterface.generated.h"

struct FRTSCore_VehicleInput;

// This class does not need to be modified.
UINTERFACE()
class RTSCORE_API URTSCore_VehicleInputInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RTSCORE_API IRTSCore_VehicleInputInterface
{
	GENERATED_BODY()
	
public:
	virtual void ApplyVehicleInput(const FRTSCore_VehicleInput& VehicleInput) = 0;
	
	
	virtual void SetThrottle(const float ThrottleInput) = 0;
	virtual void SetSteering(const float SteeringInput) = 0;
	virtual void SetBrake(const float SteeringInput) = 0;
	virtual void SetHandbrake(const bool bHandbrake) = 0;	
	virtual void SetParked(const bool bIsParked) = 0;
	virtual float GetThrottle() const = 0;
	virtual float GetBrake() const = 0;
	virtual bool IsParked() const = 0;
	virtual FVector GetRVOAvoidanceVelocity() const = 0;
	
	virtual float GetMaxSpeed() const = 0;
	virtual float GetMaxTurningSpeed() const = 0;
	virtual float GetThrottleInput() const = 0;
	virtual bool GetHandBrakeInput() const = 0;
	virtual float GetBrakeInput() const = 0;
	virtual float GetMass() const = 0;
	virtual bool IsMaxAngularVelocity() const = 0;
	virtual TObjectPtr<UCurveFloat> GetBrakingCurve() const = 0;
	virtual TObjectPtr<UCurveFloat> GetCorneringCurve() const = 0;
	virtual TObjectPtr<UCurveFloat> GetThrottleCurve() const = 0;
	virtual TObjectPtr<UCurveFloat> GetSteeringCurve() const = 0;
	virtual TObjectPtr<UCurveFloat> GetSteeringDampeningCurve() const = 0;
	virtual TObjectPtr<UCurveFloat> GetCollisionSteeringCurve() const = 0;
	virtual TObjectPtr<UCurveFloat> GetCollisionBrakeCurve() const = 0;
};
