// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/RTSEquipment_Item.h"

#include "Data/RTSEquipment_ItemDataAsset.h"
#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"


ARTSEquipment_Item::ARTSEquipment_Item()
{
	PrimaryActorTick.bCanEverTick = false;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMeshComponent->SetSimulatePhysics(false);
	bReplicates = true;
}

void ARTSEquipment_Item::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemDataAssetId);
}

void ARTSEquipment_Item::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARTSEquipment_Item::OnEquipped(const FPrimaryAssetId& AssetId)
{
	ItemDataAssetId = AssetId;
}

void ARTSEquipment_Item::OnUnequipped()
{
}
