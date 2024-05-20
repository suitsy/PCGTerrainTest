// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/Vehicle/RTSVehicles_AnimInstance.h"

#include "AIController.h"
#include "Entities/Vehicle/RTSVehicles_Base.h"
#include "ChaosWheeledVehicleMovementComponent.h"

URTSVehicles_AnimInstance::URTSVehicles_AnimInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	VehiclePawn = nullptr;
	OwningController = nullptr;
	VehicleMovementComponent = nullptr;
	WheelsAmount = 0;
	WheelRotFrontL = 0;
	WheelRotFrontR = 0;
	WheelRotMiddleL = 0;
	WheelRotMiddleR = 0;
	WheelRotRearL = 0;
	WheelRotRearR = 0;
	WheelRotAccessoryL = 0;
	WheelRotAccessoryR = 0;
	SaggingDegreeL = 0;
	SaggingDegreeR = 0;
	bChassisLockedR = false;
	bChassisLockedL = false;
}

void URTSVehicles_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	VehiclePawn = Cast<ARTSVehicles_Base>(TryGetPawnOwner());
	if(VehiclePawn)
	{
		VehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(VehiclePawn->GetVehicleMovement());		
	}
}

void URTSVehicles_AnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	
	if(!VehiclePawn || !VehicleMovementComponent)
	{
		return;
	}

	WheelsAmount = VehicleMovementComponent->GetNumWheels();
	WheelZOffsets = VehiclePawn->GetWheelZOffsets();
	WheelRotFrontL = VehiclePawn->GetWheelRotFrontL();
	WheelRotFrontR = VehiclePawn->GetWheelRotFrontR(); 	
	WheelRotMiddleL = VehiclePawn->GetWheelRotMiddleL();
	WheelRotMiddleR = VehiclePawn->GetWheelRotMiddleR();
	WheelRotRearL = VehiclePawn->GetWheelRotRearL();
	WheelRotRearR = VehiclePawn->GetWheelRotRearR();
	WheelRotAccessoryL = VehiclePawn->GetWheelRotAccessoryL();
	WheelRotAccessoryR = VehiclePawn->GetWheelRotAccessoryR();
	SaggingDegreeL = VehiclePawn->GetSaggingDegreeL();
	SaggingDegreeR = VehiclePawn->GetSaggingDegreeR();
	VibrationOffsetL = VehiclePawn->GetVibrationOffsetL();
	VibrationOffsetR = VehiclePawn->GetVibrationOffsetR();
	bChassisLockedR = VehiclePawn->GetChassisLockedR();
	bChassisLockedL = VehiclePawn->GetChassisLockedL();
	AntennaRotation = VehiclePawn->GetAntennaRotation();
	TurretRotations = VehiclePawn->GetTurretRotations();
	WeaponRotations = VehiclePawn->GetWeaponRotations();
}
