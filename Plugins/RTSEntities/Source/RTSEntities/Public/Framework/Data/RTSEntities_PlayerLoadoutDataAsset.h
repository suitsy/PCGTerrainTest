// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RTSEntities_PlayerLoadoutDataAsset.generated.h"


UCLASS()
class RTSENTITIES_API URTSEntities_PlayerLoadoutDataAsset : public UPrimaryDataAsset
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FText Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	TSoftObjectPtr<UTexture2D> Icon;	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta=(AllowedTypes="GroupData"))
	TArray<FPrimaryAssetId> Groups;
};
