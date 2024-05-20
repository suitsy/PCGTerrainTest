// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSEquipment_ItemDataAsset.h"
#include "RTSEquipment_ItemWeaponBaseDataAsset.generated.h"

class URTSEquipment_ItemAmmunitionDataAsset;
/**
 * 
 */
UCLASS()
class RTSEQUIPMENT_API URTSEquipment_ItemWeaponBaseDataAsset : public URTSEquipment_ItemDataAsset
{
	GENERATED_BODY()

public:
	URTSEquipment_ItemWeaponBaseDataAsset();
	virtual ERTSEquipment_WeaponSlot GetWeaponSlot();
	virtual FPrimaryAssetId GetAmmunitionType() const;
	virtual float GetRange() const;
	virtual float GetFireRate() const;
	virtual float GetReloadRate() const;
	virtual float GetAccuracy() const;
	virtual FName GetProjectileSocket() const;
	virtual float GetMinScatter() const;
	virtual float GetRotationSpeed() const;	
	virtual float GetPitchSpeed() const;
	virtual FVector2D GetHorizontalRange() const;
	virtual FVector2D GetVerticalRange() const;
	virtual float GetTraversalSpeed() const;
	virtual UAnimMontage* GetFireMontage() const;
	virtual UAnimMontage* GetReloadMontage() const;
	virtual USoundBase* GetFireSound() const;
	virtual USoundBase* GetReloadSound() const;
	virtual UParticleSystem* GetFireEffect() const;
	virtual UParticleSystem* GetFireEnvironmentEffect() const;	

protected:
	UPROPERTY(EditAnywhere, meta=(EditCondition = "Type == ERTSEquipment_Type::Weapon", EditConditionHides), Category = Weapon)
	ERTSEquipment_WeaponSlot WeaponSlot;

	UPROPERTY(EditAnywhere, meta=(EditCondition = "Type == ERTSEquipment_Type::Weapon", EditConditionHides, AllowedTypes = "AmmunitionData"), Category = Ammunition)
	FPrimaryAssetId AmmunitionType;

	UPROPERTY(EditAnywhere, meta=(EditCondition = "Type == ERTSEquipment_Type::Weapon", EditConditionHides), Category = "Weapon|Animations")
	UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, meta=(EditCondition = "Type == ERTSEquipment_Type::Weapon", EditConditionHides), Category = "Weapon|Animations")
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, meta=(EditCondition = "Type == ERTSEquipment_Type::Weapon", EditConditionHides), Category = "Weapon|Sounds")
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, meta=(EditCondition = "Type == ERTSEquipment_Type::Weapon", EditConditionHides), Category = "Weapon|Sounds")
	USoundBase* ReloadSound;

	UPROPERTY(EditAnywhere, meta=(EditCondition = "Type == ERTSEquipment_Type::Weapon", EditConditionHides), Category = "Weapon|Effects")
	UParticleSystem* FireEffect;

	UPROPERTY(EditAnywhere, meta=(EditCondition = "Type == ERTSEquipment_Type::Weapon", EditConditionHides), Category = "Weapon|Effects")
	UParticleSystem* FireEnvironmentEffect;
};