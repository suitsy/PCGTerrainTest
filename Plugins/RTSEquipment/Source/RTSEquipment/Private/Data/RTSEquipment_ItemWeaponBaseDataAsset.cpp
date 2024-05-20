// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/RTSEquipment_ItemWeaponBaseDataAsset.h"

URTSEquipment_ItemWeaponBaseDataAsset::URTSEquipment_ItemWeaponBaseDataAsset():
	WeaponSlot(ERTSEquipment_WeaponSlot::Primary),
	FireMontage(nullptr),
	ReloadMontage(nullptr),
	FireSound(nullptr),
	ReloadSound(nullptr),
	FireEffect(nullptr),
	FireEnvironmentEffect(nullptr)
{
	Type = ERTSEquipment_Type::Weapon;
}

ERTSEquipment_WeaponSlot URTSEquipment_ItemWeaponBaseDataAsset::GetWeaponSlot()
{
	return WeaponSlot;
}

FPrimaryAssetId URTSEquipment_ItemWeaponBaseDataAsset::GetAmmunitionType() const
{
	return AmmunitionType.IsValid() ? AmmunitionType : FPrimaryAssetId();
}

float URTSEquipment_ItemWeaponBaseDataAsset::GetRange() const
{
	return 8000.f;
}

float URTSEquipment_ItemWeaponBaseDataAsset::GetFireRate() const
{
	return 0.5f;
}

float URTSEquipment_ItemWeaponBaseDataAsset::GetReloadRate() const
{
	return 5.f;
}

float URTSEquipment_ItemWeaponBaseDataAsset::GetAccuracy() const
{
	return 30.f;
}

FName URTSEquipment_ItemWeaponBaseDataAsset::GetProjectileSocket() const
{
	return FName(TEXT("muzzle"));
}

float URTSEquipment_ItemWeaponBaseDataAsset::GetMinScatter() const
{
	return 0.f;
}

float URTSEquipment_ItemWeaponBaseDataAsset::GetRotationSpeed() const
{
	return 0.f;
}

float URTSEquipment_ItemWeaponBaseDataAsset::GetPitchSpeed() const
{
	return 0.f;
}

FVector2D URTSEquipment_ItemWeaponBaseDataAsset::GetHorizontalRange() const
{
	return FVector2D::ZeroVector;
}

FVector2D URTSEquipment_ItemWeaponBaseDataAsset::GetVerticalRange() const
{
	return FVector2D::ZeroVector;
}

float URTSEquipment_ItemWeaponBaseDataAsset::GetTraversalSpeed() const
{
	return 0.f;
}

UAnimMontage* URTSEquipment_ItemWeaponBaseDataAsset::GetFireMontage() const
{
	return FireMontage;
}

UAnimMontage* URTSEquipment_ItemWeaponBaseDataAsset::GetReloadMontage() const
{
	return ReloadMontage;
}

USoundBase* URTSEquipment_ItemWeaponBaseDataAsset::GetFireSound() const
{
	return FireSound;
}

USoundBase* URTSEquipment_ItemWeaponBaseDataAsset::GetReloadSound() const
{
	return ReloadSound;
}

UParticleSystem* URTSEquipment_ItemWeaponBaseDataAsset::GetFireEffect() const
{
	return FireEffect;
}

UParticleSystem* URTSEquipment_ItemWeaponBaseDataAsset::GetFireEnvironmentEffect() const
{
	return FireEnvironmentEffect;
}
