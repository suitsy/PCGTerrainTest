// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Components/RTSEntities_LoadoutManager.h"
#include "Framework/Data/RTSEntities_GroupDataAsset.h"
#include "Framework/Data/RTSEntities_PlayerLoadoutDataAsset.h"
#include "Engine/AssetManager.h"
#include "Framework/Entities/Components/RTSEntities_Entity.h"
#include "Framework/Data/RTSCore_StaticGameData.h"
#include "Framework/Data/RTSEntities_EntityDataAsset.h"
#include "Framework/Debug/RTSEntities_Debug.h"
#include "Framework/Managers/RTSCore_GameMode.h"
#include "Framework/Managers/RTSCore_GameState.h"
#include "Framework/Player/RTSEntities_PlayerController.h"
#include "GameFramework/PlayerState.h"


URTSEntities_LoadoutManager::URTSEntities_LoadoutManager(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer), PlayerDataLoadAttempts(100)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URTSEntities_LoadoutManager::BeginPlay()
{
	Super::BeginPlay();

	if(!HasAuthority() || !GetWorld())
	{
		return;
	}
	
	// Listen for new players logging in
	if(ARTSCore_GameMode* RTSCoreGameMode = GetGameMode<ARTSCore_GameMode>())
	{
		RTSCoreGameMode->OnGameModePlayerInitialized.AddUObject(this, &ThisClass::OnPlayerInitialized);
	}

	// Register this component with the preload game phase state
	if(ARTSCore_GameState* RTSGameState = Cast<ARTSCore_GameState>(GetWorld()->GetGameState()))
	{
		RTSGameState->CallOrRegisterForEvent_OnGamePhasePreLoad(FOnGamePhasePreloadDelegate::FDelegate::CreateUObject(this, &ThisClass::PreloadEntityData), PreloadTaskId, FString("[LoadoutManagerComp][PreLoad] LoadPlayerLoadouts"));
	}
}

void URTSEntities_LoadoutManager::OnPlayerInitialized(AController* Controller)
{
	if(ARTSEntities_PlayerController* PlayerController = Cast<ARTSEntities_PlayerController>(Controller))
	{
		if(RegisteredPlayers.Find(PlayerController) == nullptr)
		{
			RegisteredPlayers.Add(PlayerController, false);
		}
	}
	
	if(const ARTSCore_GameState* RTSGameState = GetGameState<ARTSCore_GameState>())
	{
		if(RTSGameState && RTSGameState->GetGamePhaseState() > ERTSCore_GamePhaseState::PreLoad)
		{			
			InitialisePlayerLoadoutData();
		}
	}	
}

void URTSEntities_LoadoutManager::RegisterPlayersToLoad()
{
	if(const AGameStateBase* GameState = GetGameState<AGameStateBase>())
	{
		// Register players loadout loading
		for (int i = 0; i < GameState->PlayerArray.Num(); ++i)
		{
			if(!GameState->PlayerArray[i])
			{
				continue;
			}
			
			if(ARTSEntities_PlayerController* PlayerController = Cast<ARTSEntities_PlayerController>(GameState->PlayerArray[i]->GetPlayerController()))
			{
				if(RegisteredPlayers.Find(PlayerController) == nullptr)
				{
					RegisteredPlayers.Add(PlayerController, false);
				}
			}
		}		
	}
}

void URTSEntities_LoadoutManager::PreloadEntityData()
{
	if(!HasAuthority())
	{
		return;
	}
	
	if(UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		AllFormationDataAssets.Empty();
		const FPrimaryAssetType FormationAssetType(RTS_DATA_ASSET_TYPE_FORMATION);	
		AssetManager->GetPrimaryAssetIdList(FormationAssetType, AllFormationDataAssets);

		if(AllFormationDataAssets.Num() > 0)
		{
			const TArray<FName> Bundles;
			const FStreamableDelegate FormationDataLoadedDelegate = FStreamableDelegate::CreateUObject(this, &URTSEntities_LoadoutManager::OnEntityDataLoaded);
			AssetManager->LoadPrimaryAssets(AllFormationDataAssets, Bundles, FormationDataLoadedDelegate);
		}
		else
		{
			UE_LOG(LogRTSEntities, Error, TEXT("[%s] - Failed to Get RTSFormationData Assets"), *GetClass()->GetSuperClass()->GetName());
		}
	}
}

void URTSEntities_LoadoutManager::InitialisePlayerLoadoutData()
{
	if(!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	// Ensure all loadout array is reset
	//AllLoadoutDataAssets.Empty();

	for (auto It = RegisteredPlayers.CreateConstIterator(); It; ++It)
	{
		// Check player is valid and not already loaded
		if(!It.Key() || It.Value())
		{
			continue;
		}
		
		// Bind to OnPlayerEntitiesCreated delegate
		It.Key()->OnPlayerEntitiesLoaded.AddUObject(this, &URTSEntities_LoadoutManager::OnPlayerLoadoutLoaded);

		// Begin loading player loadout assets
		const FPrimaryAssetId PlayerLoadoutAsset = GetPlayerLoadout(It.Key());
		if(PlayerLoadoutAsset.IsValid())
		{
			LoadPlayerLoadout(It.Key(), PlayerLoadoutAsset);
		}		
	}
}

FPrimaryAssetId URTSEntities_LoadoutManager::GetPlayerLoadout(const ARTSEntities_PlayerController* PlayerController)
{
	const FPrimaryAssetId PlayerLoadoutAsset = PlayerController->GetPlayerLoadout();
	if(PlayerLoadoutAsset.IsValid())
	{
		// Add player loadout to all loadouts array
		AllLoadoutDataAssets.AddUnique(PlayerLoadoutAsset);

		return PlayerLoadoutAsset;
	}	

	return FPrimaryAssetId();
}

void URTSEntities_LoadoutManager::LoadPlayerLoadout(ARTSEntities_PlayerController* PlayerController, const FPrimaryAssetId& PlayerLoadoutAsset)
{
	if(PlayerLoadoutAsset.IsValid())
	{
		// Load player loadout asset
		if(UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
		{
			const TArray<FName> Bundles;
			const FStreamableDelegate LoadedDelegate = FStreamableDelegate::CreateUObject(this, &URTSEntities_LoadoutManager::OnPlayerLoadoutDataLoaded, PlayerController, PlayerLoadoutAsset);
			AssetManager->LoadPrimaryAsset(PlayerLoadoutAsset, Bundles, LoadedDelegate);
		}
	}
	else
	{
		UE_LOG(LogRTSEntities, Error, TEXT("[URTSEntities_LoadoutManager::LoadPlayerLoadout] Failed to get player Loadout Asset"));
	}	
}

bool URTSEntities_LoadoutManager::AreAllPlayerLoadoutsCreated() const
{
	for (auto It = RegisteredPlayers.CreateConstIterator(); It; ++It)
	{
		if(!It.Value())
		{
			return false;
		}
	}

	return true;
}

void URTSEntities_LoadoutManager::OnEntityDataLoaded()
{
	if(!HasAuthority())
	{
		return;
	}	

	RegisterPlayersToLoad();
	InitialisePlayerLoadoutData();
}

void URTSEntities_LoadoutManager::OnPlayerLoadoutDataLoaded(ARTSEntities_PlayerController* PlayerController, FPrimaryAssetId PlayerLoadout)
{
	if(!HasAuthority())
	{
		return;
	}
	
	if(UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		// Cast our player loadout data class from the data asset id
		if(URTSEntities_PlayerLoadoutDataAsset* PlayerLoadoutData = Cast<URTSEntities_PlayerLoadoutDataAsset>(AssetManager->GetPrimaryAssetObject(PlayerLoadout)))
		{
			// Load all the group data assets
			if(PlayerLoadoutData->Groups.Num() > 0)
			{
				const TArray<FName> Bundles;
				const FStreamableDelegate LoadedDelegate = FStreamableDelegate::CreateUObject(this, &URTSEntities_LoadoutManager::OnPlayerGroupDataLoaded, PlayerController, PlayerLoadoutData->Groups);
				AssetManager->LoadPrimaryAssets(PlayerLoadoutData->Groups, Bundles, LoadedDelegate);
			}
			else
			{
				UE_LOG(LogRTSEntities, Error, TEXT("[URTSEntities_LoadoutManager::OnPlayerLoadoutDataLoaded] - Failed to get player GroupData Assets"));
			}
		}		
	}
}

void URTSEntities_LoadoutManager::OnPlayerGroupDataLoaded(ARTSEntities_PlayerController* PlayerController,	const TArray<FPrimaryAssetId> PlayerGroupAssets)
{
	if(!HasAuthority())
    {
    	return;
    }    	
    	
	if(UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		// Build an array of all the individual entity data assets
		TArray<FPrimaryAssetId> AllEntityDataAssets;
		for (int i = 0; i < PlayerGroupAssets.Num(); ++i)
		{
			if(URTSEntities_GroupDataAsset* PlayerGroupData = Cast<URTSEntities_GroupDataAsset>(AssetManager->GetPrimaryAssetObject(PlayerGroupAssets[i])))
			{
				// Gather all the entity data assets
				TArray<FPrimaryAssetId> GroupEntityData;
				PlayerGroupData->GetGroupEntityData(GroupEntityData);

				// Append group assets to all entity data list
				if(GroupEntityData.Num() > 0)
				{					
					AllEntityDataAssets.Append(GroupEntityData);
				}
			}
		}

		// Load all the entity data assets
		if(AllEntityDataAssets.Num() > 0)
		{
			const TArray<FName> Bundles;
			const FStreamableDelegate LoadedDelegate = FStreamableDelegate::CreateUObject(this, &URTSEntities_LoadoutManager::OnPlayerEntityDataLoaded, PlayerController, PlayerGroupAssets, AllEntityDataAssets);
			AssetManager->LoadPrimaryAssets(AllEntityDataAssets, Bundles, LoadedDelegate);
		}
		else
		{
			UE_LOG(LogRTSEntities, Error, TEXT("[URTSEntities_LoadoutManager::OnPlayerLoadoutDataLoaded] - Failed to get player GroupData Assets"));
		}
	}
}

void URTSEntities_LoadoutManager::OnPlayerEntityDataLoaded(ARTSEntities_PlayerController* PlayerController, const TArray<FPrimaryAssetId> PlayerGroupAssets, const TArray<FPrimaryAssetId> PlayerEntityAssets)
{
	if(!HasAuthority())
	{
		return;
	}

	if(UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		TArray<FPrimaryAssetId> EntityAiAssets;
		for (int i = 0; i < PlayerEntityAssets.Num(); ++i)
		{
			if(const URTSEntities_EntityDataAsset* EntityData = Cast<URTSEntities_EntityDataAsset>(AssetManager->GetPrimaryAssetObject(PlayerEntityAssets[i])))
			{
				EntityAiAssets.Add(EntityData->AiDataAssetId);
			}
		}	
	
		// Load all the entity data assets
		if(EntityAiAssets.Num() > 0)
		{
			const TArray<FName> Bundles;
			const FStreamableDelegate LoadedDelegate = FStreamableDelegate::CreateUObject(this, &URTSEntities_LoadoutManager::OnAllPlayerEntityDataLoaded, PlayerController, PlayerGroupAssets, PlayerDataLoadAttempts);
			AssetManager->LoadPrimaryAssets(EntityAiAssets, Bundles, LoadedDelegate);
		}
		else
		{
			UE_LOG(LogRTSEntities, Error, TEXT("[URTSEntities_LoadoutManager::OnPlayerEntityDataLoaded] - Failed to get player EntityData Assets"));
		}
	}	
}

void URTSEntities_LoadoutManager::OnAllPlayerEntityDataLoaded(ARTSEntities_PlayerController* PlayerController, const TArray<FPrimaryAssetId> PlayerGroupAssets, int32 AttemptsLeft)
{
	if(!HasAuthority())
	{
		return;
	}
	
	// Create player group actors and spawn entities
	if(PlayerController)
	{
		// Find or add entry for player, set loadout loaded to true
		bool& bIsRegistered = RegisteredPlayers.FindOrAdd(PlayerController);
		bIsRegistered = false;
		
		// Check player is ready to setup loadout
		if(PlayerController->HasTeamsInitiated() && !bIsRegistered)
		{
			PlayerController->CreateGroupsEQS(PlayerGroupAssets);			
		}
		else if (AttemptsLeft > 0)
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ThisClass::OnAllPlayerEntityDataLoaded, PlayerController, PlayerGroupAssets, AttemptsLeft - 1));
		}
		else
		{
			UE_LOG(LogRTSEntities, Error, TEXT("[URTSEntities_LoadoutManager::OnAllPlayerEntityDataLoaded] - Player Entity Loading Timed Out!"));
		}
	}	
}

void URTSEntities_LoadoutManager::OnPlayerLoadoutLoaded(const ARTSEntities_PlayerController* PlayerController)
{
	if(!HasAuthority() || !GetWorld())
	{
		return;
	}
	
	// Check we have a reference for the loaded player
	if(bool* PlayerRef = RegisteredPlayers.Find(PlayerController))
	{
		// Set register player load state to true
		*PlayerRef = true;

		// Check if all players have loaded
		if(AreAllPlayerLoadoutsCreated())
		{
			// Ensure we have a reference to the game state
			if(ARTSCore_GameState* RTSGameState = Cast<ARTSCore_GameState>(GetWorld()->GetGameState()))
			{
				// Register the load loadouts task complete
				RTSGameState->RegistrationPhaseTaskComplete(PreloadTaskId);
			}
		}
	}
}

