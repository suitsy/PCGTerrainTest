// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RTSCore_EntityEquipmentInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class RTSCORE_API URTSCore_EntityEquipmentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RTSCORE_API IRTSCore_EntityEquipmentInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:	
	virtual void AssignEquipment(UActorComponent* NewEquipment) = 0;
	virtual UActorComponent* GetEquipment() const = 0;
	virtual void OnEquipmentCreated() = 0;	
	virtual TArray<FRotator> GetTurretRotations() const = 0;
	virtual TArray<FRotator> GetWeaponRotations() const = 0;
	virtual void RequestReload() = 0;	
	virtual void FireWeapon() = 0;
	virtual void ReloadWeapon() = 0;
	virtual int32 GetCurrentWeaponAmmo() const = 0;
	virtual bool HasInventoryAmmoForCurrentWeapon() const = 0;
	virtual float GetCurrentWeaponFireRate() const = 0;
	virtual float GetCurrentWeaponReloadRate() const = 0;
	virtual float GetCurrentWeaponAccuracy() const = 0;
	virtual bool CanEngageTarget() = 0;
	virtual bool CanUpdateTarget() = 0;
	virtual FVector GetTargetLastKnownLocation() const = 0;
};
