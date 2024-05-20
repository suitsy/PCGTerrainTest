// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/RTSEquipment_Item.h"
#include "RTSEquipment_WeaponItem.generated.h"

/**
 * 
 */
UCLASS()
class RTSEQUIPMENT_API ARTSEquipment_WeaponItem : public ARTSEquipment_Item
{
	GENERATED_BODY()

public:
	explicit ARTSEquipment_WeaponItem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnEquipped(const FPrimaryAssetId& AssetId) override;
	virtual void OnUnequipped() override;	
	virtual void Fire();
	virtual void Reload();

protected:	
	UFUNCTION(NetMulticast, Unreliable)
	virtual void NetMulticast_Fire();

	UFUNCTION(NetMulticast, Unreliable)
	virtual void NetMulticast_Reload();

	UPROPERTY()
	float LastFired;

	UPROPERTY()
	float TimeStartedReload;

private:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (AllowPrivateAccess = true))
	USceneComponent* Muzzle;
};
