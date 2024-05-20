// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RTSCore_InventoryInterface.generated.h"

struct FRTSEquipment_Slot;
// This class does not need to be modified.
UINTERFACE()
class RTSCORE_API URTSCore_InventoryInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RTSCORE_API IRTSCore_InventoryInterface
{
	GENERATED_BODY()

public:
	virtual void SetEquipmentData(APlayerController* NewOwningPlayer, const TArray<FPrimaryAssetId>& EquipmentData) = 0;
	virtual bool HasWeaponEquipped() const = 0;
	virtual void FireWeapon() = 0;
	virtual void ReloadWeapon() = 0;
	virtual float GetCurrentWeaponRange() = 0;
	virtual int32 GetCurrentWeaponMagazineAmmoCount() = 0;
	virtual float GetCurrentWeaponFireRate() = 0;
	virtual bool HasInventoryAmmoForCurrentWeapon() = 0;
	virtual float GetCurrentWeaponAccuracy() = 0;
	virtual float GetCurrentWeaponReloadRate() = 0;	
	virtual TArray<FRotator> GetTurretRotations() const = 0;
	virtual TArray<FRotator> GetWeaponRotations() const = 0;
};
