// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSEntities_DataTypes.h"
#include "Engine/DataAsset.h"
#include "RTSEntities_EntityDataAsset.generated.h"


class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FRTSEntities_Speeds
{
	GENERATED_BODY()

public:
	FRTSEntities_Speeds(): SpeedState(ERTSCore_SpeedState::NoSpeed), Speed(0.f) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERTSCore_SpeedState SpeedState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed;
};


UCLASS()
class RTSENTITIES_API URTSEntities_EntityDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection|Highlight")
	TSoftObjectPtr<UMaterialInstance> HighlightMaterial;

	/** Sets the selection marker type **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection|Marker")
	ERTSEntities_SelectionMarkerType SelectionMarkerType = ERTSEntities_SelectionMarkerType::Decal;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection|Marker")
	TSoftObjectPtr<UMaterialInstance> SelectedMarkerMaterial;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection|Marker")
	TSoftObjectPtr<UMaterialInstance> DestinationMarkerMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection|Marker", meta=(EditCondition = "SelectionMarkerType == ERTSEntities_SelectionMarkerType::NiagaraSystem", EditConditionHides))
	TSoftObjectPtr<UNiagaraSystem> NiagaraSystemSelection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection|Marker", meta=(EditCondition = "SelectionMarkerType == ERTSEntities_SelectionMarkerType::NiagaraSystem", EditConditionHides))
	TSoftObjectPtr<UNiagaraSystem> NiagaraSystemDestination;

	/** Decal size is determined by entity collision, use the modifier to adjust decal size, size is multiplied by this modifier **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	float SelectionMarkerRadius = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Command")
	TSoftObjectPtr<UMaterialInstance> CommandDestinationMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ai", meta=(AllowedTypes="AiData"))
	FPrimaryAssetId AiDataAssetId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation|Speed")
	float DefaultMaxSpeed;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation|Speed")
	TArray<FRTSEntities_Speeds> SpeedStates;		

	/** The spacing this entities needs to avoid collisions or overlap **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation|Spacing")
	float MinimumSpacing;	

	/** Distance at which the entity will trigger an arrived at destination **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation|Conditions")
	float AcceptanceRadius;

	/** Speed the entity must be under to trigger an arrived at a destination **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation|Conditions")
	float AcceptanceSpeed;

	/** Distance at which the entity will begin to reduce speed when arriving at a destination **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation|Conditions")
	float ArriveDistance;

	/** Rate the entity can turn when stationary **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation|Handling", meta=(EditCondition = "Type == ERTSCore_EntityType::Vehicle"))
	float MaxTurnRate;

	/** The max rate the entity will apply to brake when attempting to arrive at the destination **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation|Handling", meta=(EditCondition = "Type == ERTSCore_EntityType::Vehicle"))
	float MaxDestinationBrake;

	/** Equipment the entity will spawn with **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta=(AllowedTypes="EquipmentDataRifle"))
	TArray<FPrimaryAssetId> Equipment;

	float GetStateSpeed(const ERTSCore_SpeedState& SpeedState) const;
};
