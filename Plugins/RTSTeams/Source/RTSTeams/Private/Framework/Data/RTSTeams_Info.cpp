// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Data/RTSTeams_Info.h"
#include "Framework/Data/RTSTeams_DataAsset.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"


ARTSTeams_Info::ARTSTeams_Info(const FObjectInitializer& ObjectInitializer)  : Super(ObjectInitializer), TeamId(INDEX_NONE)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	NetPriority = 3.0f;
	SetReplicatingMovement(false);
}

void ARTSTeams_Info::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, TeamId, SharedParams);	
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, TeamDisplayAsset, SharedParams);	
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Members, SharedParams);	
}

bool ARTSTeams_Info::HasAvailableSlot() const
{
	return TeamDisplayAsset->Slots - Members.Num() > 0;
}

void ARTSTeams_Info::BeginPlay()
{
	Super::BeginPlay();
}

bool ARTSTeams_Info::AssignTeamMember(ARTSTeams_PlayerState* Member)
{
	if(HasAvailableSlot())
	{
		Members.Add(Member);
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Members, this);
		return true;
	}

	return false;
}

void ARTSTeams_Info::SetTeamId(const uint8 NewTeamId)
{
	if(!HasAuthority())
	{
		return;
	}

	TeamId = NewTeamId;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, TeamId, this);
}

void ARTSTeams_Info::SetTeamDisplayAsset(const TObjectPtr<URTSTeams_DataAsset> NewDisplayAsset)
{
	if(!HasAuthority() || NewDisplayAsset == nullptr)
	{
		return;
	}
	
	TeamDisplayAsset = NewDisplayAsset;	
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, TeamDisplayAsset, this);
}

