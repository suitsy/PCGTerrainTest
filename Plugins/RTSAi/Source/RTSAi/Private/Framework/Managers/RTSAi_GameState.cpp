// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Managers/RTSAi_GameState.h"
#include "Framework/Components/RTSAi_CoverSystemComponent.h"

ARTSAi_GameState::ARTSAi_GameState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	
	CoverSystem = CreateDefaultSubobject<URTSAi_CoverSystemComponent>(TEXT("CoverSystemComponent"));
}

void ARTSAi_GameState::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		CallOrRegisterForEvent_OnGamePhasePreGameplay(FOnGamePhasePreGameplayDelegate::FDelegate::CreateUObject(this, &ThisClass::GenerateCover), GenerateCoverTaskId, FString("AiGameState - GenerateCover"));
	}
}

void ARTSAi_GameState::GenerateCover()
{
	if(CoverSystem != nullptr)
	{
		CoverSystem->OnCoverGenerated.AddLambda([this]()
		{
			// Register the load loadouts task complete
			RegistrationPhaseTaskComplete(GenerateCoverTaskId);
			if(CoverSystem != nullptr)
			{
				CoverSystem->OnCoverGenerated.RemoveAll(this);
			}
		});

		CoverSystem->GenerateCover();
	}
}
