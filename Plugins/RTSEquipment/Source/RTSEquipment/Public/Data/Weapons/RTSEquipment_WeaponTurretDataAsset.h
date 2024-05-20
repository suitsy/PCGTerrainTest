// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/RTSEquipment_ItemWeaponBaseDataAsset.h"
#include "RTSEquipment_WeaponTurretDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class RTSEQUIPMENT_API URTSEquipment_WeaponTurretDataAsset : public URTSEquipment_ItemWeaponBaseDataAsset
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(EditAnywhere, Category = Turret)
	int32 AnimationIndex;
	
	UPROPERTY(EditAnywhere, Category = Turret)
	USoundBase* RotationSound;
	
	// Stats	
	virtual float GetMinScatter() const override;
	virtual float GetRotationSpeed() const override;
	virtual float GetPitchSpeed() const override;
	virtual FVector2D GetHorizontalRange() const override;
	virtual FVector2D GetVerticalRange() const override;
	
	virtual FName GetTurretBoneName() const;	
	virtual FName GetTurretWeaponBoneName() const;	
	
	UPROPERTY(EditAnywhere, Category = Statistics)
	FRTSEquipment_Statistics_Turret Statistics;
};
