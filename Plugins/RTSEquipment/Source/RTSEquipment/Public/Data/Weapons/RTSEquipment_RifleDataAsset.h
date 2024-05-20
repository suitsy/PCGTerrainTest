// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/RTSEquipment_ItemWeaponBaseDataAsset.h"
#include "RTSEquipment_RifleDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class RTSEQUIPMENT_API URTSEquipment_RifleDataAsset : public URTSEquipment_ItemWeaponBaseDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = Statistics)
	FRTSEquipment_Statistics_Rifle Statistics;
};
