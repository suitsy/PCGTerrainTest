// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Components/RTSCore_GameStateComponent.h"
#include "Framework/Interfaces/RTSCore_TeamManager.h"
#include "RTSTeams_ManagerComponent.generated.h"


class UNiagaraComponent;
class ARTSTeams_PlayerState;
class ARTSTeams_Info;
class URTSTeams_DataAsset;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RTSTEAMS_API URTSTeams_ManagerComponent : public URTSCore_GameStateComponent, public IRTSCore_TeamManagerInterface
{
	GENERATED_BODY()

public:
	explicit URTSTeams_ManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());	
	uint8 GetPlayerTeam(const ARTSTeams_PlayerState* PlayerState) const;
	ARTSTeams_Info* GetTeamInfo(const uint8 TeamId) const;
	URTSTeams_DataAsset* GetTeamDisplayAsset(const uint8 TeamId) const;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnRegister() override;
	void OnGamePhaseInitiate();
	void OnTeamsDataLoaded();
	void CreateTeams();
	void CreateTeam(const uint8 TeamId, const FPrimaryAssetId& TeamDataAssetId);
	bool HasAvailableSlot(const uint8 TeamId) const;
	void AssignPlayersToTeam();
	void AssignConnectedPlayerTeam(ARTSTeams_PlayerState* Teams_PlayerState, TArray<uint8> TeamsIndexArray, int32 AttemptsLeft) const;
	void AssignPlayerToTeam(ARTSTeams_PlayerState* Teams_PlayerState, const uint8 TeamId) const;
	void AssignConnectingPlayerTeam(ARTSTeams_PlayerState* PlayerState) const;
	void AssignPlayerRandomTeam(ARTSTeams_PlayerState* PlayerState, TArray<uint8>& TeamsIndexArray) const;
	void OnPlayerInitialized(AController* NewPlayer);
	
	UPROPERTY()
	TMap<uint8, FPrimaryAssetId> TeamsData;

	UPROPERTY(Replicated)
	TArray<ARTSTeams_Info*> Teams;

	UPROPERTY(Replicated)
	ARTSTeams_Info* DefaultTeam;

private:
	UPROPERTY()
	FGuid LoadTeamsTaskId;

	


	/** IRTSCore_TeamManagerInterface **/
public:
	virtual void ApplyTeamDataToMaterial(const uint8 TeamId, UMaterialInstanceDynamic* MaterialInstance) override;
	virtual void ApplyTeamDataToDecalComponent(const uint8 TeamId, UDecalComponent* DecalComponent) override;
	virtual void ApplyTeamDataToMeshComponent(const uint8 TeamId, UMeshComponent* MeshComponent) override;
	virtual void ApplyTeamDataToNiagaraComponent(const uint8 TeamId, UNiagaraComponent* NiagaraComponent) override;
	virtual void ApplyTeamDataToActor(const uint8 TeamId, AActor* TargetActor, bool bIncludeChildActors) override;

	/** End IRTSCore_TeamManagerInterface **/
};
