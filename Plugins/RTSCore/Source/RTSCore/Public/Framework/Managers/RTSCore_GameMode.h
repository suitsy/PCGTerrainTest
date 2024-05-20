// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/RTSCore_DataTypes.h"
#include "GameFramework/GameModeBase.h"
#include "RTSCore_GameMode.generated.h"


class URTSCore_GameDataAsset;
class ARTSCore_PlayerController;

/**
 * Post login event, triggered when a player or bot joins the game as well as after seamless and non seamless travel
 * This is called after the player has finished initialization
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameModePlayerInitialized, AController* /*NewPlayer*/);


UCLASS(Abstract)
class RTSCORE_API ARTSCore_GameMode : public AGameModeBase
{
	GENERATED_BODY()

	/** Game Mode Defaults and Overrides **/
public:
	ARTSCore_GameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	virtual void GenericPlayerInitialization(AController* NewPlayer) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	virtual void FailedToRestartPlayer(AController* NewPlayer) override;
	virtual void RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset = false);
	virtual AActor* ChoosePlayerStart_Implementation(AController* NewPlayer) override;
	virtual void RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot) override;

	UFUNCTION()	
	virtual void InitGameData();

	// Delegate called on player initialization, this is called after the player has finished initialization
	FOnGameModePlayerInitialized OnGameModePlayerInitialized;

protected:	
	/** Game Phase Management **/
public:
	virtual ERTSCore_GamePhaseState GetGamePhaseState() const;
	virtual void OnPreGameplay();	

	/** Game Data Management **/
protected:
	virtual void HandleGameDataAssignment();
	virtual void OnGameDataAssignment();
	virtual void OnGameDataLoaded();
	virtual URTSCore_GameDataAsset* GetGameData() const;
	virtual void InitialisePlayers();

	UPROPERTY()
	TArray<ARTSCore_PlayerController*> SessionPlayers;

	UPROPERTY()
	FPrimaryAssetId CurrentGameDataAssetId;
	
private:
	UPROPERTY()
	FGuid GameDataTaskLoadedId;
	
	UPROPERTY()
	FGuid PreGameplayTaskId;
};
	
