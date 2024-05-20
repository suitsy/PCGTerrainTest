// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Interfaces/RTSCore_TeamInterface.h"
#include "GameFramework/PlayerState.h"
#include "RTSTeams_PlayerState.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class RTSTEAMS_API ARTSTeams_PlayerState : public APlayerState, public IRTSCore_TeamInterface
{
	GENERATED_BODY()

public:
	ARTSTeams_PlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** IRTSCore_Team Implementation **/
	virtual bool IsInitialised() const override { return TeamInitiated; }
	virtual void SetInitialised() override;
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override;
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
	virtual FOnTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override { return &OnTeamChanged; }
	virtual uint8 GetTeamId() const { return TeamId.GetId(); }	
	
protected:	
	UFUNCTION()
	void OnRep_TeamId(FGenericTeamId OldTeamId);	

	UFUNCTION(Server, Reliable)
	void Server_TeamInitiated();

	UPROPERTY(ReplicatedUsing=OnRep_TeamId)
	FGenericTeamId TeamId;

	UPROPERTY()
	FOnTeamIndexChangedDelegate OnTeamChanged;

	UPROPERTY(Replicated)
	uint8 TeamInitiated:1;
	/** End IRTSCore_Team Implementation **/	
};
