// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSEntities_DataTypes.h"
#include "Engine/DataAsset.h"
#include "RTSEntities_EntityDataAsset.generated.h"


UCLASS()
class RTSENTITIES_API URTSEntities_EntityDataAsset : public UPrimaryDataAsset
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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	ERTSCore_EntityClass EntityClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	ERTSCore_EntityType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	TSoftObjectPtr<UMaterialInstance> HighlightMaterial;

	/** Enabling use of selection decal add the selection decal component to entity **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	bool bUseDecal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	TSoftObjectPtr<UMaterialInstance> SelectedMaterial;

	/** Decal size is determined by entity collision, use the modifier to adjust decal size, size is multiplied by this modifier **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	float DecalSizeModifier = 1.5f;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Command")
	TSoftObjectPtr<UMaterialInstance> CommandDestinationMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ai", meta=(AllowedTypes="AiData"))
	FPrimaryAssetId AiDataAssetId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration|Navigation")
	float DefaultMaxSpeed;		

	/** The spacing this entities needs to avoid collisions or overlap **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration|Navigation")
	float Spacing;	

	/** Distance at which the entity will trigger an arrived at destination **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration|Navigation")
	float AcceptanceRadius;

	/** Speed the entity must be under to trigger an arrived at a destination **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration|Navigation")
	float AcceptanceSpeed;

	/** Distance at which the entity will begin to reduce speed when arriving at a destination **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration|Navigation")
	float ArriveDistance;

	/** Rate the entity can turn when stationary **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration|Navigation", meta=(EditCondition = "Type == ERTSCore_EntityType::Vehicle"))
	float MaxTurnRate;

	/** The max rate the entity will apply to brake when attempting to arrive at the destination **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration|Navigation", meta=(EditCondition = "Type == ERTSCore_EntityType::Vehicle"))
	float MaxDestinationBrake;

	/** Equipment the entity will spawn with **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration|Inventory", meta=(AllowedTypes="EquipmentDataRifle"))
	TArray<FPrimaryAssetId> Equipment;
};
