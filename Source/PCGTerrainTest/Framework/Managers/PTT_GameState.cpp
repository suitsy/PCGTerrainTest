// Fill out your copyright notice in the Description page of Project Settings.


#include "PTT_GameState.h"

#include "Components/RTSEquipment_ManagerComponent.h"

APTT_GameState::APTT_GameState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	EquipmentManagerComponent = CreateDefaultSubobject<URTSEquipment_ManagerComponent>(TEXT("EquipmentManagerComponent"));
}

void APTT_GameState::BeginPlay()
{
	Super::BeginPlay();
}
