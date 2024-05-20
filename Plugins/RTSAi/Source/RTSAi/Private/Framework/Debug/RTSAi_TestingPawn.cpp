// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Debug/RTSAi_TestingPawn.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"

ARTSAi_TestingPawn::ARTSAi_TestingPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourceComponent"));	
}

void ARTSAi_TestingPawn::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamId = NewTeamID;
}

void ARTSAi_TestingPawn::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority() && StimuliSourceComponent && bDetectInGame)
	{
		StimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
		StimuliSourceComponent->RegisterWithPerceptionSystem();
	}
}
