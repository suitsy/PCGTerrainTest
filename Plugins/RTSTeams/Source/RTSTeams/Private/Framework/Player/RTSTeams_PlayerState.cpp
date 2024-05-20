// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Player/RTSTeams_PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

ARTSTeams_PlayerState::ARTSTeams_PlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer),
	TeamInitiated(0)
{
	TeamId = FGenericTeamId::NoTeam;
}

void ARTSTeams_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, TeamId, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, TeamInitiated, SharedParams);
}

void ARTSTeams_PlayerState::SetInitialised()
{
	Server_TeamInitiated();
}

void ARTSTeams_PlayerState::SetGenericTeamId(const FGenericTeamId& NewTeamId)
{
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot set team for %s on non-authority"), *GetPathName(this));
		return;
	}

	const FGenericTeamId OldTeamId = TeamId;
	TeamId = NewTeamId;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, TeamId, this);
	
	ConditionalBroadcastTeamChanged(this, OldTeamId, NewTeamId);
}

void ARTSTeams_PlayerState::OnRep_TeamId(FGenericTeamId OldTeamId)
{
	ConditionalBroadcastTeamChanged(this, OldTeamId, TeamId);
}

void ARTSTeams_PlayerState::Server_TeamInitiated_Implementation()
{
	TeamInitiated = 1;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, TeamInitiated, this);
}
