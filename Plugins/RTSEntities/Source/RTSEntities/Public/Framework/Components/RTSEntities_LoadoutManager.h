// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Components/RTSCore_GameStateComponent.h"
#include "Framework/Data/RTSCore_DataTypes.h"
#include "RTSEntities_LoadoutManager.generated.h"

class ARTSEntities_PlayerController;
class URTSEntities_Controller;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RTSENTITIES_API URTSEntities_LoadoutManager : public URTSCore_GameStateComponent
{
	GENERATED_BODY()

public:
	explicit URTSEntities_LoadoutManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());	
	
protected:
	virtual void BeginPlay() override;
	void OnPlayerInitialized(AController* Controller);
	void RegisterPlayersToLoad();
	void PreloadEntityData();
	void InitialisePlayerLoadoutData();
	FPrimaryAssetId GetPlayerLoadout(const ARTSEntities_PlayerController* PlayerController);
	void LoadPlayerLoadout(ARTSEntities_PlayerController* PlayerController, const FPrimaryAssetId& PlayerLoadoutAsset);
	bool AreAllPlayerLoadoutsCreated() const;
	
	UFUNCTION()
	void OnEntityDataLoaded();
	
	UFUNCTION()
	void OnPlayerLoadoutDataLoaded(ARTSEntities_PlayerController* PlayerController, const FPrimaryAssetId PlayerLoadoutAsset);

	UFUNCTION()
	void OnPlayerGroupDataLoaded(ARTSEntities_PlayerController* PlayerController, const TArray<FPrimaryAssetId> PlayerGroupAssets);

	UFUNCTION()
	void OnPlayerEntityDataLoaded(ARTSEntities_PlayerController* PlayerController, const TArray<FPrimaryAssetId> PlayerGroupAssets, const TArray<FPrimaryAssetId> PlayerEntityAssets);

	UFUNCTION()
	void OnAllPlayerEntityDataLoaded(ARTSEntities_PlayerController* PlayerController, const TArray<FPrimaryAssetId> PlayerGroupAssets, int32 AttemptsLeft);

	UFUNCTION()
	void OnPlayerLoadoutLoaded(const ARTSEntities_PlayerController* PlayerController);

private:	
	/** All loadout assets being used in this session - Server Only **/
	UPROPERTY()
	TArray<FPrimaryAssetId> AllLoadoutDataAssets;

	UPROPERTY()
	TArray<FPrimaryAssetId> AllFormationDataAssets;

	/** Registered players map, [Player, IsLoaded] **/
	UPROPERTY()
	TMap<ARTSEntities_PlayerController*, bool> RegisteredPlayers;

	UPROPERTY()
	FGuid PreloadTaskId;

	UPROPERTY()
	int32 PlayerDataLoadAttempts;
};
