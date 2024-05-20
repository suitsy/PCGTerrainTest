// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSEquipment_DataTypes.h"
#include "Engine/DataAsset.h"
#include "RTSEquipment_ItemDataAsset.generated.h"

class URTSEquipment_Instance;
/**
 * 
 */
UCLASS()
class RTSEQUIPMENT_API URTSEquipment_ItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Type of this item, set in native parent class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Asset Settings")
	FPrimaryAssetType DataType;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(DataType, GetFName());
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Display)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Display)
	FText DisplayImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Display)
	ERTSEquipment_Type Type = ERTSEquipment_Type::Equipment;

	// Actors to spawn on the pawn when this is equipped
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<FRTSEquipment_ItemData> ActorsToSpawn;	

	virtual bool ShouldPerformCollisionCheck() const;

protected:
	UPROPERTY(EditAnywhere, Category = Physics)
	bool bPerformCollisionChecks;
};
