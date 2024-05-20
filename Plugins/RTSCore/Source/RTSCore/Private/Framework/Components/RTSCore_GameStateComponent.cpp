// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Components/RTSCore_GameStateComponent.h"


URTSCore_GameStateComponent::URTSCore_GameStateComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

bool URTSCore_GameStateComponent::HasAuthority() const
{
	if(const AActor* Owner = GetOwner())
	{
		return Owner->HasAuthority();
	}
	
	return false;
}

