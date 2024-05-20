// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Managers/RTSEntities_GameState.h"
#include "Framework/Components/RTSEntities_LoadoutManager.h"

ARTSEntities_GameState::ARTSEntities_GameState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	
	LoadoutManager = CreateDefaultSubobject<URTSEntities_LoadoutManager>(TEXT("LoadoutManagerComponent"));
}

void ARTSEntities_GameState::BeginPlay()
{
	Super::BeginPlay();
}
