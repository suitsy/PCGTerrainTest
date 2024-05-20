// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RTSVehicles_AnimInstance.generated.h"

class UChaosWheeledVehicleMovementComponent;
class ARTSVehicles_Base;
class AAIController;


UCLASS()
class RTSVEHICLES_API URTSVehicles_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	explicit URTSVehicles_AnimInstance(const FObjectInitializer& ObjectInitializer);
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly)
	ARTSVehicles_Base* VehiclePawn;
	
	UPROPERTY(BlueprintReadOnly)
	AAIController* OwningController;	
	
	UPROPERTY(BlueprintReadOnly)
	UChaosWheeledVehicleMovementComponent* VehicleMovementComponent;	

	UPROPERTY(BlueprintReadOnly)
	int32 WheelsAmount;

	UPROPERTY(BlueprintReadOnly)
	TArray<float> WheelZOffsets;

	

	UPROPERTY(BlueprintReadOnly)
	float WheelRotMiddleR;

	UPROPERTY(BlueprintReadOnly)
	float WheelRotMiddleL;

	UPROPERTY(BlueprintReadOnly)
	float WheelRotFrontR;

	UPROPERTY(BlueprintReadOnly)
	float WheelRotFrontL;

	UPROPERTY(BlueprintReadOnly)
	float WheelRotRearR;

	UPROPERTY(BlueprintReadOnly)
	float WheelRotRearL;

	UPROPERTY(BlueprintReadOnly)
	float WheelRotAccessoryR;

	UPROPERTY(BlueprintReadOnly)
	float WheelRotAccessoryL;

	UPROPERTY(BlueprintReadOnly)
	float SaggingDegreeR;

	UPROPERTY(BlueprintReadOnly)
	float SaggingDegreeL;

	UPROPERTY(BlueprintReadOnly)
	TArray<double> VibrationOffsetR;

	UPROPERTY(BlueprintReadOnly)
	TArray<double> VibrationOffsetL;

	UPROPERTY(BlueprintReadOnly)
	bool bChassisLockedR;

	UPROPERTY(BlueprintReadOnly)
	bool bChassisLockedL;

	UPROPERTY(BlueprintReadOnly)
	TArray<FRotator> AntennaRotation;

	UPROPERTY(BlueprintReadOnly)
	TArray<FRotator> TurretRotations;

	UPROPERTY(BlueprintReadOnly)
	TArray<FRotator> WeaponRotations;
};
