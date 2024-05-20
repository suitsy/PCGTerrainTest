// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/Weapons/RTSEquipment_WeaponTurretDataAsset.h"

float URTSEquipment_WeaponTurretDataAsset::GetMinScatter() const
{
	return Statistics.MinScatter;
}

float URTSEquipment_WeaponTurretDataAsset::GetRotationSpeed() const
{
	return Statistics.RotationSpeed;
}

float URTSEquipment_WeaponTurretDataAsset::GetPitchSpeed() const
{
	return Statistics.PitchSpeed;
}

FVector2D URTSEquipment_WeaponTurretDataAsset::GetHorizontalRange() const
{
	return Statistics.HorizontalRange;
}

FVector2D URTSEquipment_WeaponTurretDataAsset::GetVerticalRange() const
{
	return Statistics.VerticalRange;
}

FName URTSEquipment_WeaponTurretDataAsset::GetTurretBoneName() const
{
	return Statistics.TurretBone;
}

FName URTSEquipment_WeaponTurretDataAsset::GetTurretWeaponBoneName() const
{
	return Statistics.WeaponBone;
}
