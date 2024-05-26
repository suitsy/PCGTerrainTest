// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSEntities_DataTypes.h"
#include "Engine/DataAsset.h"
#include "RTSEntities_FormationDataAsset.generated.h"


class UNavigationPath;

UCLASS()
class RTSENTITIES_API URTSEntities_FormationDataAsset : public UPrimaryDataAsset
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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EFormationType Type;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FText DisplayName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FText Description;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	TSoftObjectPtr<UTexture2D> Icon;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FVector Offset;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool Alt;

	/** The angle the subordinate formation member needs to be to be able to maintain the formation **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta=(ClampMin=-1.f, ClampMax=1.f))
	FVector2D SubordinateRearAngle = FVector2D(-0.5f, -1.f);

	void CreateFormation(FRTSEntities_Navigation& Navigation, const TArray<FRTSEntities_PlayerSelection>& Selections) const;

protected:
	static void GetSelectedAverageSpacing(const FRTSEntities_Navigation& Navigation, const TArray<FRTSEntities_PlayerSelection>& Selections, float& EntitiesSpacing); 
	static float GetSelectionSpacing(const FRTSEntities_PlayerSelection& Selection, const float EntitySpacing);
	static float GetEntitiesAverageSpacing(const TArray<AActor*>& Entities, const float Spacing);
	static float GetEntityReqSpacing(const AActor* Entity);
	void CreateFormationPositions(FRTSEntities_Navigation& Navigation, const TArray<FRTSEntities_PlayerSelection>& Selections, const float EntitiesSpacing) const;
	static bool NextPositionPermutation(int* Array, int Size);
	static void AssignSelectionPositions(FRTSEntities_Navigation& Navigation, const TArray<FRTSEntities_PlayerSelection>& Selections);
	void UpdateSelectionAssignedPositions(FRTSEntities_Navigation& Navigation) const;
	void CreateEntityPositions(FRTSEntities_Navigation& Navigation, const int32 Index) const; 
	static void AssignEntityPositions(FRTSEntities_FormationPosition& Position);
	void UpdateEntityPositions(FRTSEntities_Navigation& Navigation, const int32 Index) const;
	virtual void GenerateFormationReferencePaths(FRTSEntities_Navigation& Navigation) const;
	
};
