// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RTSTeams_DataAsset.generated.h"


class ARTSTeams_PlayerState;
class UNiagaraComponent;

UCLASS(BlueprintType)
class RTSTEAMS_API URTSTeams_DataAsset : public UPrimaryDataAsset
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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText TeamName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, FLinearColor> ColorParameters;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, float> ScalarParameters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, TObjectPtr<UTexture>> TextureParameters;	
	
	/** Can players join the same team */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Slots = 1;
	
	UFUNCTION(BlueprintCallable, Category=Teams)
	void ApplyToMaterial(UMaterialInstanceDynamic* Material);

	UFUNCTION(BlueprintCallable, Category=Teams)
	void ApplyToDecalComponent(UDecalComponent* DecalComponent);

	UFUNCTION(BlueprintCallable, Category=Teams)
	void ApplyToMeshComponent(UMeshComponent* MeshComponent);

	UFUNCTION(BlueprintCallable, Category=Teams)
	void ApplyToNiagaraComponent(UNiagaraComponent* NiagaraComponent);

	UFUNCTION(BlueprintCallable, Category=Teams, meta=(DefaultToSelf="TargetActor"))
	void ApplyToActor(AActor* TargetActor, bool bIncludeChildActors = true);
};
