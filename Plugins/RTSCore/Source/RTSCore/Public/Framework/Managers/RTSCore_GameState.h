// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/RTSCore_DataTypes.h"
#include "GameFramework/GameStateBase.h"
#include "RTSCore_GameState.generated.h"


class UCommonActivatableWidget;
class URTSCore_GameDataAsset;


DECLARE_MULTICAST_DELEGATE(FOnGamePhaseLoadGameDataDelegate);
DECLARE_MULTICAST_DELEGATE(FOnGamePhaseInitiateDelegate);
DECLARE_MULTICAST_DELEGATE(FOnGamePhaseSetupDelegate);
DECLARE_MULTICAST_DELEGATE(FOnGamePhasePreloadDelegate);
DECLARE_MULTICAST_DELEGATE(FOnGamePhasePreGameplayDelegate);
DECLARE_MULTICAST_DELEGATE(FOnGamePhaseGameplayDelegate);
DECLARE_MULTICAST_DELEGATE(FOnGamePhasePostGameplayDelegate);
DECLARE_MULTICAST_DELEGATE(FOnGamePhaseAbortDelegate);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChangedDelegate, const ERTSCore_GamePhaseState);


UCLASS(Abstract)
class RTSCORE_API ARTSCore_GameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	explicit ARTSCore_GameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetGameData(URTSCore_GameDataAsset* NewGameData);
	virtual URTSCore_GameDataAsset* GetGameData() const { return GameData; }

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(Replicated)
	URTSCore_GameDataAsset* GameData;

private:
	UPROPERTY()
	FGuid WaitGameDataTaskId;
	
	UPROPERTY()
	FGuid GameplayTaskId;
	
	UPROPERTY()
	FGuid LoadMainMenuTaskId;
	
	UPROPERTY()
	FGuid UnLoadGameMenuTaskId;
	
	UPROPERTY()
	FGuid LoadTeamManagerTaskId;

	/** Game Phase Management **/
public:
	virtual void ResetGamePhaseTaskRegistry();
	virtual ERTSCore_GamePhaseState GetGamePhaseState() const;
	virtual void RegistrationPhaseTaskComplete(const FGuid RegistrationId);
	virtual void RegistrationPhaseTaskFailed(const FGuid RegistrationId);
	
	/** Call or Register delegates ensure the delegate is called once a game state has been set
	 *  notifying subscribers of the change, these delegates are available server only **/
	
	// Initial phase of loaded game, load menu, preload general required data
	void CallOrRegisterForEvent_OnGamePhaseLoadGameData(FOnGamePhaseLoadGameDataDelegate::FDelegate&& Delegate, FGuid& TaskId, const FString& DebugName = FString());
	
	// Phase for finding, joining or hosting sessions
	void CallOrRegisterForEvent_OnGamePhaseInitiate(FOnGamePhaseInitiateDelegate::FDelegate&& Delegate, FGuid& TaskId, const FString& DebugName = FString());
	
	// Phase for choosing settings, such as a lobby or setup ui
	void CallOrRegisterForEvent_OnGamePhaseSetup(FOnGamePhaseSetupDelegate::FDelegate&& Delegate, FGuid& TaskId, const FString& DebugName = FString());
	
	// Phase for preloading assets and data required for game
	void CallOrRegisterForEvent_OnGamePhasePreLoad(FOnGamePhasePreloadDelegate::FDelegate&& Delegate, FGuid& TaskId, const FString& DebugName = FString());
	
	// Phase for any configuration that needs to occur after PreGameplay asset loading
	void CallOrRegisterForEvent_OnGamePhasePreGameplay(FOnGamePhasePreGameplayDelegate::FDelegate&& Delegate, FGuid& TaskId, const FString& DebugName = FString());
	
	// Phase actual gameplay takes place
	void CallOrRegisterForEvent_OnGamePhaseGameplay(FOnGamePhaseGameplayDelegate::FDelegate&& Delegate, FGuid& TaskId, const FString& DebugName = FString());
	
	// Phase for displaying post game statistics and menus
	void CallOrRegisterForEvent_OnGamePhasePostGameplay(FOnGamePhasePostGameplayDelegate::FDelegate&& Delegate, FGuid& TaskId, const FString& DebugName = FString());

	/** Phase event handlers **/
	FOnGamePhaseLoadGameDataDelegate& OnGamePhaseStartupChecked();
	FOnGamePhaseInitiateDelegate& OnGamePhaseInitiateChecked();
	FOnGamePhaseSetupDelegate& OnGamePhaseSetupChecked();
	FOnGamePhasePreloadDelegate& OnGamePhasePreloadChecked();
	FOnGamePhasePreGameplayDelegate& OnGamePhasePreGameplayChecked();
	FOnGamePhaseGameplayDelegate& OnGamePhaseGameplayChecked();
	FOnGamePhasePostGameplayDelegate& OnGamePhasePostGameplayChecked();
	FOnGamePhaseAbortDelegate& OnGamePhaseAbortChecked();
	FOnGamePhaseChangedDelegate& OnGamePhaseChangedChecked();

protected:
	virtual void SetGamePhaseState(ERTSCore_GamePhaseState NewPhase);
	virtual void StartGamePhaseTimer();
	virtual void GamePhaseTimer();
	virtual bool ShouldStartNextPhase() const;
	virtual void StartNextPhase();
	virtual void RestartGamePhaseSequence();		
	virtual void RegisterPhaseTask(ERTSCore_GamePhaseState Phase, FGuid& TaskId, const FString& DebugName = FString());
	
	virtual void OnGameDataLoaded();
	virtual void OnGamePhaseGameplayEvent();
	virtual void OnLoadMainMenuEvent();
	virtual void OnLoadTeamManagerEvent();

	UPROPERTY()
	TMap<FGuid, FPhaseRegistrationsData> PhaseRegistrationsData;

	UPROPERTY()
	FTimerHandle TimerHandle_PhaseTimer;

	UPROPERTY(ReplicatedUsing = OnRep_GamePhaseState)
	ERTSCore_GamePhaseState GamePhaseState = ERTSCore_GamePhaseState::Loading;

	/** Game Phase Delegates **/	
	FOnGamePhaseLoadGameDataDelegate OnGamePhaseLoadGameData;
	FOnGamePhaseInitiateDelegate OnGamePhaseInitiate;
	FOnGamePhaseSetupDelegate OnGamePhaseSetup;
	FOnGamePhasePreloadDelegate OnGamePhasePreload;
	FOnGamePhasePreGameplayDelegate OnGamePhasePreGameplay;
	FOnGamePhaseGameplayDelegate OnGamePhaseGameplay;
	FOnGamePhasePostGameplayDelegate OnGamePhasePostGameplay;
	FOnGamePhaseAbortDelegate OnGamePhaseAbort;
	FOnGamePhaseChangedDelegate OnGamePhaseChanged;

private:
	UFUNCTION()
	void OnRep_GamePhaseState();
	
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	bool bDebug;
#endif
	
#if WITH_EDITOR
	void DebugGamePhase() const;
	void DebugGamePhaseTask() const;
#endif

};