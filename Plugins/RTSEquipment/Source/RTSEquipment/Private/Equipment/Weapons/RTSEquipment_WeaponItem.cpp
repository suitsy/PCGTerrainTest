// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/Weapons/RTSEquipment_WeaponItem.h"
#include "Data/RTSEquipment_ItemDataAsset.h"
#include "Data/RTSEquipment_ItemWeaponBaseDataAsset.h"
#include "Framework/Interfaces/RTSCore_AiInterface.h"
#include "Kismet/GameplayStatics.h"

ARTSEquipment_WeaponItem::ARTSEquipment_WeaponItem(const FObjectInitializer& ObjectInitializer)
{
	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	LastFired = 0.f;
	TimeStartedReload = 0.f;
}

void ARTSEquipment_WeaponItem::OnEquipped(const FPrimaryAssetId& AssetId)
{
	Super::OnEquipped(AssetId);
}

void ARTSEquipment_WeaponItem::OnUnequipped()
{
	Super::OnUnequipped();	
}

void ARTSEquipment_WeaponItem::Fire()
{
	if(HasAuthority())	
	{
		NetMulticast_Fire();		
	}
}

void ARTSEquipment_WeaponItem::Reload()
{
	if(HasAuthority())	
	{
		NetMulticast_Reload();
	}
}

void ARTSEquipment_WeaponItem::NetMulticast_Fire_Implementation()
{
	if(GetOwner() && Muzzle && ItemDataAssetId.IsValid())	
	{
		if(const URTSEquipment_ItemWeaponBaseDataAsset* ItemData = GetData<URTSEquipment_ItemWeaponBaseDataAsset>(ItemDataAssetId))
		{
			if(IRTSCore_AiInterface* AiInterface = Cast<IRTSCore_AiInterface>(GetOwner()))
			{
				// Play fire montage
				AiInterface->PlayMontage(ItemData->GetFireMontage());

				// Play fire sound
				if(ItemData->GetFireSound() != nullptr)
				{
					UGameplayStatics::PlaySoundAtLocation(this, ItemData->GetFireSound(), Muzzle->GetComponentLocation());
				}

				// Play fire effect
				if(ItemData->GetFireEffect() != nullptr)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ItemData->GetFireEffect(), Muzzle->GetComponentLocation());
				}

				// Play environment effect
				if(ItemData->GetFireEnvironmentEffect() != nullptr)
				{
					//@TODO ray trace to nearby object to play effect of dust etc
				}
			}
		}
	}
}

void ARTSEquipment_WeaponItem::NetMulticast_Reload_Implementation()
{
	if(GetOwner() && ItemDataAssetId.IsValid())	
	{
		if(const URTSEquipment_ItemWeaponBaseDataAsset* ItemData = GetData<URTSEquipment_ItemWeaponBaseDataAsset>(ItemDataAssetId))
		{
			if(IRTSCore_AiInterface* AiInterface = Cast<IRTSCore_AiInterface>(GetOwner()))
			{
				// Play reload montage
				AiInterface->PlayMontage(ItemData->GetReloadMontage());

				// Play reload sound
				if(ItemData->GetReloadSound() != nullptr)
				{
					UGameplayStatics::PlaySoundAtLocation(this, ItemData->GetReloadSound(), GetActorLocation());
				}
			}
		}
	}
}
