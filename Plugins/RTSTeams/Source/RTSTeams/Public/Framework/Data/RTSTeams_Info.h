// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "RTSTeams_Info.generated.h"

class ARTSTeams_PlayerState;
class URTSTeams_ManagerComponent;
class URTSTeams_DataAsset;

UCLASS()
class RTSTEAMS_API ARTSTeams_Info : public AInfo
{
	GENERATED_BODY()

public:
	explicit ARTSTeams_Info(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	friend URTSTeams_ManagerComponent;
	
	uint8 GetTeamId() const { return TeamId; }
	bool HasAvailableSlot() const;
	
	
protected:
	virtual void BeginPlay() override;
	bool AssignTeamMember(ARTSTeams_PlayerState* Member);
	void RemoveTeamMembers(ARTSTeams_PlayerState* Member) { Members.Remove(Member); }
	URTSTeams_DataAsset* GetTeamDisplayAsset() const { return TeamDisplayAsset; }
	TArray<ARTSTeams_PlayerState*> GetTeamMembers() const { return Members; }
	int32 GetTeamMemberCount() const { return Members.Num(); }

private:
	void SetTeamId(uint8 NewTeamId);
	void SetTeamDisplayAsset(const TObjectPtr<URTSTeams_DataAsset> NewDisplayAsset);
	
	UPROPERTY(Replicated)
	uint8 TeamId;
	
	UPROPERTY(Replicated)
	TObjectPtr<URTSTeams_DataAsset> TeamDisplayAsset;
	
	UPROPERTY(Replicated)
	TArray<ARTSTeams_PlayerState*> Members;
};
