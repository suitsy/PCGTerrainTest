// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/RTSEquipment_ItemDataAsset.h"

bool URTSEquipment_ItemDataAsset::ShouldPerformCollisionCheck() const
{
	return bPerformCollisionChecks;
}
