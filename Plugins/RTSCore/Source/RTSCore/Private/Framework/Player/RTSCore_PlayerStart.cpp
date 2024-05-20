// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Player/RTSCore_PlayerStart.h"


ARTSCore_PlayerStart::ARTSCore_PlayerStart(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
}

void ARTSCore_PlayerStart::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ARTSCore_PlayerStart::TryClaim(AController* OccupyingController)
{
	if (OccupyingController != nullptr && !IsClaimed())
	{
		ClaimingController = OccupyingController;			
		return true;
	}	
	
	return false;
}

