// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSEquipment_ItemDataAsset.h"
#include "RTSEquipment_ItemAmmunitionDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class RTSEQUIPMENT_API URTSEquipment_ItemAmmunitionDataAsset : public URTSEquipment_ItemDataAsset
{
	GENERATED_BODY()

public:
	URTSEquipment_ItemAmmunitionDataAsset();
	
	UPROPERTY(EditAnywhere, meta=(EditCondition = "Type == ERTSEquipment_Type::Ammunition", EditConditionHides), Category = Ammunition)
	int32 MagazineCapacity;

	UPROPERTY(EditAnywhere, meta=(EditCondition = "Type == ERTSEquipment_Type::Ammunition", EditConditionHides), Category = Ammunition)
	int32 MagazineCount;
};
