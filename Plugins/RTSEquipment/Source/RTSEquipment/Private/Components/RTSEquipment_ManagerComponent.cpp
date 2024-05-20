// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/RTSEquipment_ManagerComponent.h"
#include "Components/RTSEquipment_InventoryComponent.h"


URTSEquipment_ManagerComponent::URTSEquipment_ManagerComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URTSEquipment_ManagerComponent::AddEquipmentComponent(APlayerController* NewOwningPlayer, AActor* Entity)
{
	if(URTSEquipment_InventoryComponent* EquipmentComponent = NewObject<URTSEquipment_InventoryComponent>(Entity, TEXT("EquipmentComponent")))
	{
		EquipmentComponent->RegisterComponent();
		Entity->AddInstanceComponent(EquipmentComponent);
	}
}
