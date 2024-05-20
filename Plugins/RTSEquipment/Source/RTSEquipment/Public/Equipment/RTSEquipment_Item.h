// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RTSEquipment_Item.generated.h"

UCLASS()
class RTSEQUIPMENT_API ARTSEquipment_Item : public AActor
{
	GENERATED_BODY()

public:
	ARTSEquipment_Item();
	virtual void OnEquipped(const FPrimaryAssetId& AssetId);
	virtual void OnUnequipped();
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(Replicated)
	FPrimaryAssetId ItemDataAssetId;
};
