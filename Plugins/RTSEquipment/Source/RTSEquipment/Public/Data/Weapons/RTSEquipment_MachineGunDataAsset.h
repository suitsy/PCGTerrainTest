// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/RTSEquipment_ItemWeaponBaseDataAsset.h"
#include "RTSEquipment_MachineGunDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class RTSEQUIPMENT_API URTSEquipment_MachineGunDataAsset : public URTSEquipment_ItemWeaponBaseDataAsset
{
	GENERATED_BODY()
	
public:
	virtual float GetTraversalSpeed() const override;
	
	UPROPERTY(EditAnywhere, Category = Statistics)
	FRTSEquipment_Statistics_MachineGun Statistics;
};
