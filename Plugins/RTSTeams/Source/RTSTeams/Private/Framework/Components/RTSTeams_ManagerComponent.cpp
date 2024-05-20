// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Components/RTSTeams_ManagerComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Framework/Data/RTSCore_GameDataAsset.h"
#include "Framework/Data/RTSTeams_DataAsset.h"
#include "Framework/Data/RTSTeams_Info.h"
#include "Framework/Debug/RTSTeams_Debug.h"
#include "Framework/Managers/RTSCore_GameMode.h"
#include "Framework/Managers/RTSCore_GameState.h"
#include "Framework/Player/RTSTeams_PlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"


URTSTeams_ManagerComponent::URTSTeams_ManagerComponent(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer), DefaultTeam(nullptr)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URTSTeams_ManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Teams, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DefaultTeam, SharedParams);	
}

void URTSTeams_ManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URTSTeams_ManagerComponent::OnRegister()
{
	Super::OnRegister();

	// Register this component with the game phase state pre load to load the team assets
	if(HasAuthority())
	{
		if(ARTSCore_GameState* RTSGameState = Cast<ARTSCore_GameState>(GetWorld()->GetGameState()))
		{
			RTSGameState->CallOrRegisterForEvent_OnGamePhaseInitiate(FOnGamePhasePreloadDelegate::FDelegate::CreateUObject(this, &ThisClass::OnGamePhaseInitiate), LoadTeamsTaskId, FString("[TeamsManagerComp][Initiate] LoadTeamAssets"));
		}
	}	
}

uint8 URTSTeams_ManagerComponent::GetPlayerTeam(const ARTSTeams_PlayerState* PlayerState) const
{
	if(PlayerState != nullptr)
	{
		return PlayerState->GetTeamId();
	}

	return FGenericTeamId::NoTeam;
}

ARTSTeams_Info* URTSTeams_ManagerComponent::GetTeamInfo(const uint8 TeamId) const
{
	for (int i = 0; i < Teams.Num(); ++i)
	{
		if(Teams[i]->GetTeamId() == TeamId)
		{
			return Teams[i];
		}
	}

	return nullptr;
}

URTSTeams_DataAsset* URTSTeams_ManagerComponent::GetTeamDisplayAsset(const uint8 TeamId) const
{
	if(const ARTSTeams_Info* TeamInfo = GetTeamInfo(TeamId))
	{
		return TeamInfo->GetTeamDisplayAsset();
	}

	return nullptr;
}

void URTSTeams_ManagerComponent::OnGamePhaseInitiate()
{	
	if(!HasAuthority())
	{
		return;
	}

	if(const ARTSCore_GameState* GameState = GetGameState<ARTSCore_GameState>())
	{
		// Check we have a valid game data asset
		if(URTSCore_GameDataAsset* GameData = GameState->GetGameData())
		{
			if(GameData->AvailableTeams.Num() == 0)
			{
				UE_LOG(LogRTSTeams, Error, TEXT("[%s] - No available Team Data Assets, assign Team Data in Game Data Asset!"), *GetClass()->GetName());
			}
		
			for (int i = 0; i < GameData->AvailableTeams.Num(); ++i)
			{
				if(GameData->AvailableTeams[i].IsValid())
				{
					TeamsData.Add(i, GameData->AvailableTeams[i]);
				}
			}

			// Add default team to teams data
			const FPrimaryAssetId DefaultTeamDataAssetId = FPrimaryAssetId(FPrimaryAssetType(RTS_DATA_ASSET_TYPE_TEAMDATA_TEXT), RTS_DATA_ASSET_TYPE_TEAMDATA_DEFAULT);
			if(DefaultTeamDataAssetId.IsValid())
			{
				TeamsData.Add(FGenericTeamId::NoTeam, DefaultTeamDataAssetId);
			}

			if(UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
			{
				TArray<FPrimaryAssetId> TeamsDataAssets;
				TeamsData.GenerateValueArray(TeamsDataAssets);
			
				if(TeamsDataAssets.Num() > 0)
				{
					const TArray<FName> Bundles;
					const FStreamableDelegate LoadedDelegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnTeamsDataLoaded);
					AssetManager->LoadPrimaryAssets(TeamsDataAssets, Bundles, LoadedDelegate);
				}
			}
		}
	}
}

void URTSTeams_ManagerComponent::OnTeamsDataLoaded()
{
	if(HasAuthority())
	{
		CreateTeams();
		AssignPlayersToTeam();

		if(ARTSCore_GameState* GameState = GetGameState<ARTSCore_GameState>())
		{
			GameState->RegistrationPhaseTaskComplete(LoadTeamsTaskId);
		}
	}
}

void URTSTeams_ManagerComponent::CreateTeams()
{
	if(HasAuthority())
	{
		// Create game teams
		for(auto It = TeamsData.CreateConstIterator(); It; ++It)
		{
			CreateTeam(It.Key(), It.Value());
		}
	}
}

void URTSTeams_ManagerComponent::CreateTeam(const uint8 TeamId, const FPrimaryAssetId& TeamDataAssetId)
{
	if(!HasAuthority())
	{
		return;
	}	
	
	if(const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		if(URTSTeams_DataAsset* TeamsDataAsset = Cast<URTSTeams_DataAsset>(AssetManager->GetPrimaryAssetObject(TeamDataAssetId)))
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
			if(ARTSTeams_Info* NewTeamInfo = GetWorld()->SpawnActor<ARTSTeams_Info>(ARTSTeams_Info::StaticClass(), SpawnInfo))
			{
				NewTeamInfo->SetTeamId(TeamId);
				NewTeamInfo->SetTeamDisplayAsset(TeamsDataAsset);

				if(TeamId == FGenericTeamId::NoTeam)
				{
					DefaultTeam = NewTeamInfo;
					MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DefaultTeam, this);
				}
				else
				{
					Teams.Add(NewTeamInfo);
					MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Teams, this);
				}
			}
		}
	}	
}

bool URTSTeams_ManagerComponent::HasAvailableSlot(const uint8 TeamId) const
{
	for (int i = 0; i < Teams.Num(); ++i)
	{
		if(Teams[i]->GetTeamId() == TeamId)
		{
			return Teams[i]->HasAvailableSlot();
		}
	}

	return false;
}

void URTSTeams_ManagerComponent::AssignPlayersToTeam()
{
	if(!HasAuthority())
	{
		return;
	}

	// Listen for new players logging in
	if(ARTSCore_GameMode* RTSCoreGameMode = GetGameMode<ARTSCore_GameMode>())
	{
		RTSCoreGameMode->OnGameModePlayerInitialized.AddUObject(this, &ThisClass::OnPlayerInitialized);
	}

	// Build array of team index's so we know what teams have been assigned to players
	TArray<uint8> TeamsIndexArray;
	for (const auto& TeamData : TeamsData)
	{
		if(TeamData.Key != FGenericTeamId::NoTeam)
		{
			TeamsIndexArray.Add(TeamData.Key);
		}
	}		
	
	// Assign connected players to teams
	if(const AGameStateBase* GameState = GetGameState<AGameStateBase>())
	{		
		for (APlayerState* PS : GameState->PlayerArray)
		{
			if (ARTSTeams_PlayerState* Teams_PlayerState = Cast<ARTSTeams_PlayerState>(PS))
			{
				AssignConnectedPlayerTeam(Teams_PlayerState, TeamsIndexArray, 100);						
			}
		}		
	}
}

void URTSTeams_ManagerComponent::AssignConnectedPlayerTeam(ARTSTeams_PlayerState* Teams_PlayerState, TArray<uint8> TeamsIndexArray, int32 AttemptsLeft) const
{
	if(Teams_PlayerState->IsInitialised())
	{
		if (Teams_PlayerState->IsOnlyASpectator())
		{
			Teams_PlayerState->SetGenericTeamId(FGenericTeamId::NoTeam);
		}
		else
		{
			const uint8 TeamId = GetPlayerTeam(Teams_PlayerState);
					
			if(TeamId != FGenericTeamId::NoTeam && HasAvailableSlot(TeamId))
			{
				TeamsIndexArray.RemoveAtSwap(TeamId);
				AssignPlayerToTeam(Teams_PlayerState, TeamId);
			}
			else
			{
				// Assign player a random available team
				if(TeamsIndexArray.Num() > 0)
				{
					AssignPlayerRandomTeam(Teams_PlayerState, TeamsIndexArray);
				}
				else
				{
					AssignPlayerToTeam(Teams_PlayerState, FGenericTeamId::NoTeam);
				}
			}
		}			
	}
	else if(AttemptsLeft > 0)
	{
		/*AttemptsLeft--;
		auto NextTickCallback = [this, Teams_PlayerState, &TeamsIndexArray, AttemptsLeft]()
		{
			AssignConnectedPlayerTeam(Teams_PlayerState, TeamsIndexArray, AttemptsLeft);
		};*/
		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ThisClass::AssignConnectedPlayerTeam, Teams_PlayerState, TeamsIndexArray, AttemptsLeft - 1) );  //FTimerDelegate::CreateLambda(NextTickCallback));		
	}
	else
	{
		UE_LOG(LogRTSTeams, Error, TEXT("[URTSTeams_ManagerComponent::AssignConnectedPlayerTeam] - Assigning Player Team Timed Out!"));			
	}	
}

void URTSTeams_ManagerComponent::AssignPlayerToTeam(ARTSTeams_PlayerState* Teams_PlayerState, const uint8 TeamId) const
{
	if(TeamId == FGenericTeamId::NoTeam)
	{
		// Assign default team
		DefaultTeam->AssignTeamMember(Teams_PlayerState);
		Teams_PlayerState->SetGenericTeamId(FGenericTeamId(TeamId));
	}
	else
	{
		// Assign from teams
		for (int i = 0; i < Teams.Num(); ++i)
		{
			if(Teams[i]->GetTeamId() == TeamId)
			{
				Teams[i]->AssignTeamMember(Teams_PlayerState);
				Teams_PlayerState->SetGenericTeamId(FGenericTeamId(TeamId));
				return;
			}
		}
	}
}

void URTSTeams_ManagerComponent::AssignConnectingPlayerTeam(ARTSTeams_PlayerState* PlayerState) const
{
	if(!HasAuthority())
	{
		return;
	}

	// Build array of team index's so we know what teams have been assigned to players
	TArray<uint8> TeamsIndexArray;
	for (const auto& TeamData : TeamsData)
	{
		if(TeamData.Key != FGenericTeamId::NoTeam)
		{
			TeamsIndexArray.Add(TeamData.Key);
		}
	}

	AssignConnectedPlayerTeam(PlayerState, TeamsIndexArray, 100);
}

void URTSTeams_ManagerComponent::AssignPlayerRandomTeam(ARTSTeams_PlayerState* PlayerState, TArray<uint8>& TeamsIndexArray) const
{
	// Assign player a random available team
	while (TeamsIndexArray.Num() > 0)
	{
		// Get a random index from the array
		const int32 RandomIndex = FMath::RandRange(0, TeamsIndexArray.Num() - 1);
		if(TeamsIndexArray.IsValidIndex(RandomIndex))
		{
			// Get the team id at the random team index
			const uint8 RandomTeamId = TeamsIndexArray[RandomIndex];
        
			// Check if the random index is used
			if(HasAvailableSlot(RandomTeamId))
			{
				// Assign the random team
				AssignPlayerToTeam(PlayerState, RandomTeamId);
				return;
			}
			else
			{
				// Remove the used index from the array
				TeamsIndexArray.RemoveAtSwap(RandomIndex);
			}
		}
		else
		{
			// Fallback to avoid infinite loop if no valid random index is found, should never occur
			break;
		}
	}

	// If no team assigned or error assign default team
	AssignPlayerToTeam(PlayerState, FGenericTeamId::NoTeam);
}

void URTSTeams_ManagerComponent::OnPlayerInitialized(AController* NewPlayer)
{
	if(!HasAuthority())
	{
		return;
	}
	
	if(NewPlayer)
	{
		if (ARTSTeams_PlayerState* RTSPlayerState = Cast<ARTSTeams_PlayerState>(NewPlayer->PlayerState))
		{
			AssignConnectingPlayerTeam(RTSPlayerState);
		}
	}
}

void URTSTeams_ManagerComponent::ApplyTeamDataToMaterial(const uint8 TeamId, UMaterialInstanceDynamic* MaterialInstance)
{	
	if(URTSTeams_DataAsset* TeamData = GetTeamDisplayAsset(TeamId))
	{
		TeamData->ApplyToMaterial(MaterialInstance);
	}	
}

void URTSTeams_ManagerComponent::ApplyTeamDataToDecalComponent(const uint8 TeamId, UDecalComponent* DecalComponent)
{
	if(URTSTeams_DataAsset* TeamData = GetTeamDisplayAsset(TeamId))
	{
		TeamData->ApplyToDecalComponent(DecalComponent);
	}
}

void URTSTeams_ManagerComponent::ApplyTeamDataToMeshComponent(const uint8 TeamId, UMeshComponent* MeshComponent)
{
	if(URTSTeams_DataAsset* TeamData = GetTeamDisplayAsset(TeamId))
	{
		TeamData->ApplyToMeshComponent(MeshComponent);
	}
}

void URTSTeams_ManagerComponent::ApplyTeamDataToNiagaraComponent(const uint8 TeamId, UNiagaraComponent* NiagaraComponent)
{
	if(URTSTeams_DataAsset* TeamData = GetTeamDisplayAsset(TeamId))
	{
		TeamData->ApplyToNiagaraComponent(NiagaraComponent);
	}
}

void URTSTeams_ManagerComponent::ApplyTeamDataToActor(const uint8 TeamId, AActor* TargetActor, bool bIncludeChildActors)
{
	if(URTSTeams_DataAsset* TeamData = GetTeamDisplayAsset(TeamId))
	{
		TeamData->ApplyToActor(TargetActor, bIncludeChildActors);
	}
}
