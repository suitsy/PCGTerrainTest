// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Components/RTSAi_CoverSystemComponent.h"
#include "Framework/Settings/RTSAi_DeveloperSettings.h"


URTSAi_CoverSystemComponent::URTSAi_CoverSystemComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	//CoverGenerator = nullptr;
}

void URTSAi_CoverSystemComponent::GenerateCover() const
{
	
	//CoverGenerator->Generate();
}

void URTSAi_CoverSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	InitialiseCoverSystem();
}

void URTSAi_CoverSystemComponent::InitialiseCoverSystem()
{
	if(const URTSAi_DeveloperSettings* DeveloperSettings = GetDefault<URTSAi_DeveloperSettings>())
	{
		/*if(GetWorld() && DeveloperSettings->bUseCoverSystem && DeveloperSettings->CoverSystemClass.LoadSynchronous())
		{
			CoverGenerator = GetWorld()->SpawnActor<ACoverGenerator>(DeveloperSettings->CoverSystemClass.LoadSynchronous());
			if(CoverGenerator != nullptr)
			{
				CoverGenerator->OnCoverGenerationComplete.AddLambda([this]()
				{
					OnCoverGenerated.Broadcast();
				});
			}
		}*/
	}
}

