// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Managers/RTSCore_GameState.h"

#include "Framework/Components/RTSCore_GameStateComponent.h"
#include "Framework/Data/RTSCore_GameDataAsset.h"
#include "Framework/Debug/RTSCore_Debug.h"
#include "Framework/Managers/RTSCore_GameInstance.h"
#include "Framework/Player/RTSCore_PlayerController.h"
#include "Framework/Settings/RTSCore_DeveloperSettings.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

ARTSCore_GameState::ARTSCore_GameState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	GameData = nullptr;
	bDebug = false;
}

void ARTSCore_GameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);	

#if WITH_EDITOR
	DebugGamePhase();
#endif
}

void ARTSCore_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, GameData);
	DOREPLIFETIME(ThisClass, GamePhaseState);
}

void ARTSCore_GameState::SetGameData(URTSCore_GameDataAsset* NewGameData)
{
	if(HasAuthority() && NewGameData != nullptr)
	{		
		GameData = NewGameData;
		OnGameDataLoaded();				
	}
}

void ARTSCore_GameState::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority() && GetWorld())
	{		
		// Bind to game phase state changes
		if(const URTSCore_GameInstance* GameInstance = Cast<URTSCore_GameInstance>(GetWorld()->GetGameInstance()))
		{
			SetGamePhaseState(GameInstance->GetGamePhaseState());
		}
		
		// Register a task for the Startup game phase state to allow waiting for game data
		CallOrRegisterForEvent_OnGamePhaseLoadGameData(FOnGamePhaseLoadGameDataDelegate::FDelegate::CreateUObject(this, &ThisClass::OnGameDataLoaded), WaitGameDataTaskId, FString("[GameState][LoadGameData] WaitForGameData"));
		
		if(!WaitGameDataTaskId.IsValid())
		{
			UE_LOG(LogRTSCore, Error, TEXT("[%s] Failed to register wait game data game phase task!"), *GetNameSafe(this));
		}
		
		/** Init Game Phase System **/
		if(GetGamePhaseState() != ERTSCore_GamePhaseState::LoadGameData)
		{
			SetGamePhaseState(ERTSCore_GamePhaseState::LoadGameData);
		}

		// Start game phase loop
		StartGamePhaseTimer();
	}
}

void ARTSCore_GameState::ResetGamePhaseTaskRegistry()
{
	PhaseRegistrationsData.Empty();
}

ERTSCore_GamePhaseState ARTSCore_GameState::GetGamePhaseState() const
{
	return GamePhaseState;
}

void ARTSCore_GameState::SetGamePhaseState(const ERTSCore_GamePhaseState NewPhase)
{
	if(HasAuthority())
	{
		// Ensure we have a reference to the game instance
		if(URTSCore_GameInstance* GameInstance = Cast<URTSCore_GameInstance>(GetGameInstance()))
		{
			// Set current game phase on game instance if different
			if(GameInstance->GetGamePhaseState() != NewPhase)
			{
				GameInstance->SetGamePhaseState(NewPhase);
			}

			// Set local game phase state for replication if not already set
			if(GamePhaseState != NewPhase)
			{
				GamePhaseState = NewPhase;
				OnRep_GamePhaseState();
			}
		}
	}
}

void ARTSCore_GameState::RegistrationPhaseTaskComplete(const FGuid RegistrationId)
{
	if(!HasAuthority())
	{
		return;
	}
	
	// Find the phase task registration 
	if(FPhaseRegistrationsData* DataPtr = PhaseRegistrationsData.Find(RegistrationId))
	{		
		// Assign task complete
		DataPtr->bIsCompleted = true;

#if WITH_EDITOR
		if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
		{
			if(CoreSettings->DebugRTSCore && CoreSettings->DebugGamePhase)
			{
				UE_LOG(LogRTSCore, Log, TEXT("[PhaseTaskRegister] Phase Task Complete [%s]"), *DataPtr->DebugText);
				for(auto It = PhaseRegistrationsData.CreateConstIterator(); It; ++It)
				{
					if(!It.Value().bIsCompleted)
					{
						UE_LOG(LogRTSCore, Warning, TEXT("[PhaseTaskRegister] Outstanding [%s]"), *It.Value().DebugText);
					}			
				}
			}
		}
#endif	
	}	
}

void ARTSCore_GameState::RegistrationPhaseTaskFailed(const FGuid RegistrationId)
{
	if(!HasAuthority())
	{
		return;
	}
	
	RestartGamePhaseSequence();
}

void ARTSCore_GameState::CallOrRegisterForEvent_OnGamePhaseLoadGameData(FOnGamePhaseLoadGameDataDelegate::FDelegate&& Delegate, FGuid& TaskId, const FString& DebugName)
{
	RegisterPhaseTask(ERTSCore_GamePhaseState::LoadGameData, TaskId, DebugName);
	if (GetGamePhaseState() >= ERTSCore_GamePhaseState::LoadGameData)
	{
		// Phase has passed or is current phase, execute immediately (server or client dependent on caller)
		Delegate.Execute();
	}
	else
	{
		// Add registration delegate to the phase delegate (server or client dependent on caller)
		OnGamePhaseLoadGameData.Add(MoveTemp(Delegate));
	}
}

void ARTSCore_GameState::CallOrRegisterForEvent_OnGamePhaseInitiate(FOnGamePhaseInitiateDelegate::FDelegate&& Delegate, FGuid& TaskId, const FString& DebugName)
{
	RegisterPhaseTask(ERTSCore_GamePhaseState::Initiate, TaskId, DebugName);
	if (GetGamePhaseState() >= ERTSCore_GamePhaseState::Initiate)
	{
		// Phase has passed or is current phase, execute immediately (server or client dependent on caller)
		Delegate.Execute();
	}
	else
	{
		// Add registration delegate to the phase delegate (server or client dependent on caller)
		OnGamePhaseInitiate.Add(MoveTemp(Delegate));
	}
}

void ARTSCore_GameState::CallOrRegisterForEvent_OnGamePhaseSetup(FOnGamePhaseSetupDelegate::FDelegate&& Delegate, FGuid& TaskId, const FString& DebugName)
{
	RegisterPhaseTask(ERTSCore_GamePhaseState::Setup, TaskId, DebugName);
	if (GetGamePhaseState() >= ERTSCore_GamePhaseState::Setup)
	{
		// Phase has passed or is current phase, execute immediately (server or client dependent on caller)
		Delegate.Execute();
	}
	else
	{
		// Add registration delegate to the phase delegate (server or client dependent on caller)
		OnGamePhaseSetup.Add(MoveTemp(Delegate));
	}
}

void ARTSCore_GameState::CallOrRegisterForEvent_OnGamePhasePreLoad(FOnGamePhasePreloadDelegate::FDelegate&& Delegate, FGuid& TaskId, const FString& DebugName)
{
	RegisterPhaseTask(ERTSCore_GamePhaseState::PreLoad, TaskId, DebugName);
	if (GetGamePhaseState() >= ERTSCore_GamePhaseState::PreLoad)
	{
		// Phase has passed or is current phase, execute immediately (server or client dependent on caller)
		Delegate.Execute();
	}
	else
	{
		// Add registration delegate to the phase delegate (server or client dependent on caller)
		OnGamePhasePreload.Add(MoveTemp(Delegate));
	}
}

void ARTSCore_GameState::CallOrRegisterForEvent_OnGamePhasePreGameplay(FOnGamePhasePreloadDelegate::FDelegate&& Delegate, FGuid& TaskId, const FString& DebugName)
{
	RegisterPhaseTask(ERTSCore_GamePhaseState::PreGameplay, TaskId, DebugName);
	if (GetGamePhaseState() >= ERTSCore_GamePhaseState::PreGameplay)
	{
		// Phase has passed or is current phase, execute immediately (server or client dependent on caller)
		Delegate.Execute();
	}
	else
	{
		// Add registration delegate to the phase delegate (server or client dependent on caller)
		OnGamePhasePreGameplay.Add(MoveTemp(Delegate));
	}
}

void ARTSCore_GameState::CallOrRegisterForEvent_OnGamePhaseGameplay(FOnGamePhaseGameplayDelegate::FDelegate&& Delegate, FGuid& TaskId, const FString& DebugName)
{
	RegisterPhaseTask(ERTSCore_GamePhaseState::Gameplay, TaskId, DebugName);
	if (GetGamePhaseState() >= ERTSCore_GamePhaseState::Gameplay)
	{
		// Phase has passed or is current phase, execute immediately (server or client dependent on caller)
		Delegate.Execute();
	}
	else
	{
		// Add registration delegate to the phase delegate (server or client dependent on caller)
		OnGamePhaseGameplay.Add(MoveTemp(Delegate));
	}
}

void ARTSCore_GameState::CallOrRegisterForEvent_OnGamePhasePostGameplay(FOnGamePhasePostGameplayDelegate::FDelegate&& Delegate, FGuid& TaskId, const FString& DebugName)
{
	RegisterPhaseTask(ERTSCore_GamePhaseState::PostGameplay, TaskId, DebugName);
	if (GetGamePhaseState() >= ERTSCore_GamePhaseState::PostGameplay)
	{
		// Phase has passed or is current phase, execute immediately (server or client dependent on caller)
		Delegate.Execute();
	}
	else
	{
		// Add registration delegate to the phase delegate (server or client dependent on caller)
		OnGamePhasePostGameplay.Add(MoveTemp(Delegate));
	}
}

FOnGamePhaseLoadGameDataDelegate& ARTSCore_GameState::OnGamePhaseStartupChecked()
{
	FOnGamePhaseLoadGameDataDelegate* Result = &OnGamePhaseLoadGameData;
	check(Result);
	return *Result;
}

FOnGamePhaseInitiateDelegate& ARTSCore_GameState::OnGamePhaseInitiateChecked()
{
	FOnGamePhaseLoadGameDataDelegate* Result = &OnGamePhaseInitiate;
	check(Result);
	return *Result;
}

FOnGamePhaseSetupDelegate& ARTSCore_GameState::OnGamePhaseSetupChecked()
{
	FOnGamePhaseLoadGameDataDelegate* Result = &OnGamePhaseSetup;
	check(Result);
	return *Result;
}

FOnGamePhasePreloadDelegate& ARTSCore_GameState::OnGamePhasePreloadChecked()
{
	FOnGamePhaseLoadGameDataDelegate* Result = &OnGamePhasePreload;
	check(Result);
	return *Result;
}

FOnGamePhasePreGameplayDelegate& ARTSCore_GameState::OnGamePhasePreGameplayChecked()
{
	FOnGamePhaseLoadGameDataDelegate* Result = &OnGamePhasePreGameplay;
	check(Result);
	return *Result;
}

FOnGamePhaseGameplayDelegate& ARTSCore_GameState::OnGamePhaseGameplayChecked()
{
	FOnGamePhaseLoadGameDataDelegate* Result = &OnGamePhaseGameplay;
	check(Result);
	return *Result;
}

FOnGamePhasePostGameplayDelegate& ARTSCore_GameState::OnGamePhasePostGameplayChecked()
{
	FOnGamePhaseLoadGameDataDelegate* Result = &OnGamePhasePostGameplay;
	check(Result);
	return *Result;
}

FOnGamePhaseAbortDelegate& ARTSCore_GameState::OnGamePhaseAbortChecked()
{
	FOnGamePhaseLoadGameDataDelegate* Result = &OnGamePhaseAbort;
	check(Result);
	return *Result;
}

FOnGamePhaseChangedDelegate& ARTSCore_GameState::OnGamePhaseChangedChecked()
{
	FOnGamePhaseChangedDelegate* Result = &OnGamePhaseChanged;
	check(Result);
	return *Result;
}

void ARTSCore_GameState::StartGamePhaseTimer()
{
	if(GetWorld())
	{
		// Ensure the timer is not already active
		if(TimerHandle_PhaseTimer.IsValid())
		{
			// If the timer has been activated in the past ensure its not running
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_PhaseTimer);
		}
		
		// Added delay to allow game to startup and any initial phase registrations to occur
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_PhaseTimer, this, &ARTSCore_GameState::GamePhaseTimer, 0.5f, true, 1.f);
	}
}

void ARTSCore_GameState::GamePhaseTimer()
{
	if(ShouldStartNextPhase())
	{
		StartNextPhase();
	}
}

bool ARTSCore_GameState::ShouldStartNextPhase() const
{
	// Check if all registered tasks for the current phase have completed
	for (auto It = PhaseRegistrationsData.CreateConstIterator(); It; ++It)
	{
		// Check if any of the current game phase state tasks are not completed
		if(It.Value().GamePhase == GetGamePhaseState())
		{
			if (!It.Value().bIsCompleted)
			{
				// Wait until all registrations complete their tasks for the current phase
				return false;
			}
		}
	}

	return true;
}

void ARTSCore_GameState::StartNextPhase()
{
	// Increment the phase
	const int32 NextPhaseIndex = static_cast<int32>(GetGamePhaseState()) + 1;
	
	// Check if the next phase is valid
	if (NextPhaseIndex >= static_cast<int32>(ERTSCore_GamePhaseState::Abort))
	{
		// Should never transition automatically to abort or out of range state
		//@TODO Might be able to go back to lobby from end here on end of game or abort
		SetGamePhaseState(ERTSCore_GamePhaseState::LoadGameData);
	}
	else
	{		
		// Move to next phase
		SetGamePhaseState(static_cast<ERTSCore_GamePhaseState>(NextPhaseIndex));
	}
}

void ARTSCore_GameState::RestartGamePhaseSequence()
{
	if(HasAuthority())
	{
		PhaseRegistrationsData.Empty();
		SetGamePhaseState(ERTSCore_GamePhaseState::LoadGameData);
	}
}

void ARTSCore_GameState::RegisterPhaseTask(const ERTSCore_GamePhaseState Phase, FGuid& TaskId, const FString& DebugName)
{	
	if(HasAuthority())
	{
		// If a valid task id was not sent create one
		if(!TaskId.IsValid())
		{
			TaskId = FGuid::NewGuid();
		}

		// Register task in the game phase task registry 
		PhaseRegistrationsData.Add(TaskId, FPhaseRegistrationsData(Phase, DebugName));

#if WITH_EDITOR
		if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
		{
			if(CoreSettings->DebugRTSCore && CoreSettings->DebugGamePhase)
			{
				UE_LOG(LogRTSCore, Log, TEXT("[PhaseTaskRegister] Phase Task Registered [%s]"), *DebugName);
			}
		}
#endif		
	}
}

void ARTSCore_GameState::OnGameDataLoaded()
{
	// Check if we are displaying a menu or hosting/joining a game
	if(HasAuthority() && GameData != nullptr)
	{
		// Register a task for Gameplay phase so the phase is not skipped and needs to be manually ended
		CallOrRegisterForEvent_OnGamePhaseGameplay(FOnGamePhaseGameplayDelegate::FDelegate::CreateUObject(this, &ThisClass::OnGamePhaseGameplayEvent), GameplayTaskId, FString("[GameState][GamePlay] GameplayInProgress "));
		
		// Check game data for required tasks for game phases
		if(GameData->bShowMenu)
		{
			CallOrRegisterForEvent_OnGamePhaseInitiate(FOnGamePhaseInitiateDelegate::FDelegate::CreateUObject(this, &ARTSCore_GameState::OnLoadMainMenuEvent), LoadMainMenuTaskId, FString("[GameState][Initiate] LoadMainMenu"));
			//CallOrRegisterForEvent_OnGamePhaseInitiate(FOnGamePhaseInitiateDelegate::FDelegate::CreateUObject(this, &ARTSCore_GameState::OnLoadMainMenuEvent), UnLoadGameMenuTaskId, FString("UnLoadGameMenu - GameState"));
		}		

		// Create team manager game state component
		if(const URTSCore_DeveloperSettings* Settings = GetDefault<URTSCore_DeveloperSettings>())
		{
			if(Settings->TeamManagerComponent.LoadSynchronous())
			{
				CallOrRegisterForEvent_OnGamePhaseInitiate(FOnGamePhaseInitiateDelegate::FDelegate::CreateUObject(this, &ARTSCore_GameState::OnLoadTeamManagerEvent), LoadTeamManagerTaskId, FString("[GameState][Initiate] Load Team Manager"));
			}
		}
		
		// Register the startup task complete
		if(WaitGameDataTaskId.IsValid())
		{
			RegistrationPhaseTaskComplete(WaitGameDataTaskId);
		}
		else
		{
			UE_LOG(LogRTSCore, Error, TEXT("[%s] Failed to register startup task completed!"), *GetClass()->GetSuperClass()->GetName());		
		}	
	}
}

void ARTSCore_GameState::OnGamePhaseGameplayEvent()
{
	
}

void ARTSCore_GameState::OnLoadMainMenuEvent()
{
	for (int i = 0; i < PlayerArray.Num(); ++i)
	{
		if(PlayerArray[i] != nullptr)
		{
			if(ARTSCore_PlayerController* RTSPlayer = Cast<ARTSCore_PlayerController>(PlayerArray[i]->GetPlayerController()))
			{
				RTSPlayer->CreateMainMenu(UnLoadGameMenuTaskId);
			}
		}
		
	}
	
}

void ARTSCore_GameState::OnLoadTeamManagerEvent()
{
	if(const URTSCore_DeveloperSettings* Settings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(Settings->TeamManagerComponent.LoadSynchronous())
		{
			if(UActorComponent* NewComp = NewObject<UActorComponent>(this, Settings->TeamManagerComponent.LoadSynchronous(), Settings->TeamManagerComponent.LoadSynchronous()->GetFName()))
			{
				NewComp->RegisterComponent();
			}
		}
	}

	RegistrationPhaseTaskComplete(LoadTeamManagerTaskId);
}


void ARTSCore_GameState::OnRep_GamePhaseState()
{
	OnGamePhaseChanged.Broadcast(GamePhaseState);
	
	switch (GamePhaseState)
	{
		case ERTSCore_GamePhaseState::LoadGameData:
			OnGamePhaseLoadGameData.Broadcast();
			break;
		case ERTSCore_GamePhaseState::Initiate:
			OnGamePhaseInitiate.Broadcast();
			break;
		case ERTSCore_GamePhaseState::Setup:
			OnGamePhaseSetup.Broadcast();
			break;
		case ERTSCore_GamePhaseState::PreLoad:
			OnGamePhasePreload.Broadcast();
			break;
		case ERTSCore_GamePhaseState::PreGameplay:
			OnGamePhasePreGameplay.Broadcast();
			break;
		case ERTSCore_GamePhaseState::Gameplay:
			OnGamePhaseGameplay.Broadcast();
			break;
		case ERTSCore_GamePhaseState::PostGameplay:
			OnGamePhasePostGameplay.Broadcast();
			break;
		case ERTSCore_GamePhaseState::Abort:
			OnGamePhaseAbort.Broadcast();
			break;
		default: ;
	}

#if WITH_EDITOR
	DebugGamePhaseTask();
#endif
}


#if WITH_EDITOR

void ARTSCore_GameState::DebugGamePhase() const
{
	if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(CoreSettings->DebugRTSCore && CoreSettings->DebugGamePhase)
		{
			RTSCORE_PRINT_TICK(FColor::Emerald, FString::Printf(TEXT("GamePhase: %s"), *UEnum::GetValueAsString(GetGamePhaseState())));
		}
	}	
}

void ARTSCore_GameState::DebugGamePhaseTask() const
{
	if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{		
		if(CoreSettings->DebugRTSCore && CoreSettings->DebugGamePhase)
		{
			TArray<FPhaseRegistrationsData> Tasks;
			PhaseRegistrationsData.GenerateValueArray(Tasks);
			for (int i = 0; i < Tasks.Num(); ++i)
			{
				if(Tasks[i].GamePhase == GetGamePhaseState())
				{
					RTSCORE_PRINT_TIME(7.f, FColor::Silver, FString::Printf(TEXT("Task: %s"), *Tasks[i].DebugText));
				}
			}
		}
	}
}

#endif

