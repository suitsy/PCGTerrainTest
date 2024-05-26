// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Managers/RTSCore_GameMode.h"

#include "EngineUtils.h"
#include "Engine/AssetManager.h"
#include "Framework/Data/RTSCore_GameDataAsset.h"
#include "Framework/Data/RTSCore_StaticGameData.h"
#include "Framework/Debug/RTSCore_Debug.h"
#include "Framework/Managers/RTSCore_GameInstance.h"
#include "Framework/Managers/RTSCore_GameState.h"
#include "Framework/Player/RTSCore_PlayerController.h"
#include "Framework/Player/RTSCore_PlayerStart.h"
#include "Kismet/GameplayStatics.h"

ARTSCore_GameMode::ARTSCore_GameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{	
}

void ARTSCore_GameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	SessionPlayers.Empty();
}

void ARTSCore_GameMode::InitGameState()
{
	Super::InitGameState();

	// Check if this load phase is a new game or a newly hosted lobby
	if(ARTSCore_GameState* RTSGameState = Cast<ARTSCore_GameState>(GameState))
	{
		// Ensure game phase tasks are reset, clearing from game mode to ensure not delayed so do not reset game mode tasks
		RTSGameState->ResetGamePhaseTaskRegistry();
		RTSGameState->CallOrRegisterForEvent_OnGamePhaseLoadGameData(FOnGamePhaseLoadGameDataDelegate::FDelegate::CreateUObject(this, &ThisClass::InitGameData), GameDataTaskLoadedId, FString("[GameMode][LoadGameData] LoadGameData"));
		RTSGameState->CallOrRegisterForEvent_OnGamePhasePreGameplay(FOnGamePhasePreGameplayDelegate::FDelegate::CreateUObject(this, &ThisClass::OnPreGameplay), PreGameplayTaskId, FString("[GameMode][PreGameplay] InitPlayers"));		
	
		
		// This is for the initial game phase only
		/*switch (GetGamePhaseState())
		{
		case ERTSCore_GamePhaseState::NewGame:
				RTSGameState->CallOrRegisterForEvent_OnGamePhaseLoadGameData(FOnGamePhaseLoadGameDataDelegate::FDelegate::CreateUObject(this, &ThisClass::InitGameData), GameDataTaskLoadedId, FString("LoadGameData - GameMode"));
				break;
			case ERTSCore_GamePhaseState::LoadGameData:
				break;
		case ERTSCore_GamePhaseState::Initiate:
				// If we are starting at Initiate phase, indicates we are hosting or joining a game, need to check for existing game data
				RTSGameState->CallOrRegisterForEvent_OnGamePhaseInitiate(FOnGamePhaseInitiateDelegate::FDelegate::CreateUObject(this, &ThisClass::InitGameData), GameDataTaskLoadedId, FString("InitGameData - GameMode"));
				break;
			case ERTSCore_GamePhaseState::Setup:
				break;
			case ERTSCore_GamePhaseState::PreLoad:				
				break;
			case ERTSCore_GamePhaseState::PreGameplay:
				break;
			case ERTSCore_GamePhaseState::Gameplay:
				break;
			case ERTSCore_GamePhaseState::PostGameplay:
				break;
			case ERTSCore_GamePhaseState::Abort:
				break;
			default: ;
		}*/
	}
}

void ARTSCore_GameMode::InitGameData()
{
	// Wait for the next frame to give time to initialize startup settings, game state etc
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::HandleGameDataAssignment);
}

void ARTSCore_GameMode::GenericPlayerInitialization(AController* NewPlayer)
{
	Super::GenericPlayerInitialization(NewPlayer);

	if(NewPlayer)
	{
		OnGameModePlayerInitialized.Broadcast(NewPlayer);

		if(ARTSCore_PlayerController* Player = Cast<ARTSCore_PlayerController>(NewPlayer))
		{
			
			SessionPlayers.AddUnique(Player);
		}
	}
}

void ARTSCore_GameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	if(GetGamePhaseState() >= ERTSCore_GamePhaseState::PreGameplay)
	{
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}

bool ARTSCore_GameMode::PlayerCanRestart_Implementation(APlayerController* Player)
{
	if(GetGamePhaseState() >= ERTSCore_GamePhaseState::PreGameplay)
	{
		return Super::PlayerCanRestart_Implementation(Player);
	}

	return false;
}

void ARTSCore_GameMode::FailedToRestartPlayer(AController* NewPlayer)
{
	if(GetGamePhaseState() >= ERTSCore_GamePhaseState::PreGameplay)
	{
		if (APlayerController* NewPC = Cast<APlayerController>(NewPlayer))
		{
			if (PlayerCanRestart(NewPC))
			{
				RequestPlayerRestartNextFrame(NewPlayer, false);	
			}
			else
			{
				UE_LOG(LogRTSCore, Log, TEXT("FailedToRestartPlayer(%s) and PlayerCanRestart returned false, so we're not going to try again."), *GetPathNameSafe(NewPlayer));
			}
		}
	}
	else
	{
		RequestPlayerRestartNextFrame(NewPlayer, false);	
	}
	
}

void ARTSCore_GameMode::RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset)
{
	if (bForceReset && (Controller != nullptr))
	{
		Controller->Reset();
	}

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		GetWorldTimerManager().SetTimerForNextTick(PC, &APlayerController::ServerRestartPlayer_Implementation);
	}
}

AActor* ARTSCore_GameMode::ChoosePlayerStart_Implementation(AController* NewPlayer)
{
	if(GetWorld())
	{
		for (TActorIterator<ARTSCore_PlayerStart> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			// Check if the player can claim the player start
			ARTSCore_PlayerStart* PlayerStart = *ActorItr;
			if(PlayerStart->TryClaim(NewPlayer))
			{
				if(ARTSCore_PlayerController* Player = Cast<ARTSCore_PlayerController>(NewPlayer))
				{
					const FVector StartLocation = PlayerStart->GetActorLocation();
					Player->SetPlayerStartLocation(StartLocation);
				}
				
				return PlayerStart;
			}
		}
	}
	
	return Super::ChoosePlayerStart_Implementation(NewPlayer);
}

void ARTSCore_GameMode::RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot)
{
	
	
	Super::RestartPlayerAtPlayerStart(NewPlayer, StartSpot);
}

ERTSCore_GamePhaseState ARTSCore_GameMode::GetGamePhaseState() const
{
	// Ensure we have a reference to the game instance
	if(const URTSCore_GameInstance* GameInstance = Cast<URTSCore_GameInstance>(GetGameInstance()))
	{
		// Get current game phase on game instance
		return GameInstance->GetGamePhaseState();
	}

	return ERTSCore_GamePhaseState::LoadGameData;
}

void ARTSCore_GameMode::HandleGameDataAssignment()
{
	// Check options string from multiplayer hosting
	if (!CurrentGameDataAssetId.IsValid() && UGameplayStatics::HasOption(OptionsString, RTS_MP_SETTINGS_GAMEMODE))
	{
		const FString GameDataFromOptions = UGameplayStatics::ParseOption(OptionsString, RTS_MP_SETTINGS_GAMEMODE);
		CurrentGameDataAssetId = FPrimaryAssetId(FPrimaryAssetType(RTS_DATA_ASSET_TYPE_GAMEDATA), FName(*GameDataFromOptions));
	}

// Fallback to default data
if (!CurrentGameDataAssetId.IsValid())
{
	CurrentGameDataAssetId = FPrimaryAssetId(FPrimaryAssetType(RTS_DATA_ASSET_TYPE_GAMEDATA_TEXT), RTS_DATA_ASSET_TYPE_GAMEDATA_DEFAULT);
}

	if(CurrentGameDataAssetId.IsValid())
	{
		OnGameDataAssignment();
	}
	else
	{
		UE_LOG(LogRTSCore, Error, TEXT("[%s] Failed to identify game data!"), *GetNameSafe(this));
	}
}

void ARTSCore_GameMode::OnGameDataAssignment()
{
	if(UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		if(CurrentGameDataAssetId.IsValid())
		{
			TArray<FName> Bundles;
			Bundles.Add(RTS_DATA_ASSET_BUNDLE_GAME);
			const FStreamableDelegate GroupDataLoadedDelegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnGameDataLoaded);
			AssetManager->LoadPrimaryAsset(CurrentGameDataAssetId, Bundles, GroupDataLoadedDelegate);
		}
	}
}

void ARTSCore_GameMode::OnGameDataLoaded()
{
	if(GameState && CurrentGameDataAssetId.IsValid())
	{
		if(ARTSCore_GameState* RTSGameState = Cast<ARTSCore_GameState>(GameState))
		{			
			if(URTSCore_GameDataAsset* GameData = GetGameData())
			{
				RTSGameState->SetGameData(GameData);
				RTSGameState->RegistrationPhaseTaskComplete(GameDataTaskLoadedId);
			}
		}
	}
}

URTSCore_GameDataAsset* ARTSCore_GameMode::GetGameData() const
{
	if(CurrentGameDataAssetId.IsValid())
	{
		if (const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
		{
			return Cast<URTSCore_GameDataAsset>(AssetManager->GetPrimaryAssetObject(CurrentGameDataAssetId));
		}
	}

	return nullptr;
}

void ARTSCore_GameMode::InitialisePlayers()
{
	// Init player data
	for (int i = 0; i < SessionPlayers.Num(); ++i)
	{
		if ((SessionPlayers[i] != nullptr) && (SessionPlayers[i]->GetPawn() == nullptr))
		{
			if(const URTSCore_GameDataAsset* GameData = GetGameData())
			{
				if (PlayerCanRestart(SessionPlayers[i]))
				{
					RestartPlayer(SessionPlayers[i]);
				}
			}
		}
	}
}

void ARTSCore_GameMode::OnPreGameplay()
{
	InitialisePlayers();
	
	if(ARTSCore_GameState* RTSGameState = Cast<ARTSCore_GameState>(GameState))
	{
		RTSGameState->RegistrationPhaseTaskComplete(PreGameplayTaskId);
	}
}