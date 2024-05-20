// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UI/MultiplayerMenu/RTSCore_HostSessionWidget.h"
#include "CommonGameInstance.h"
#include "CommonTextBlock.h"
#include "CommonUserSubsystem.h"
#include "PrimaryGameLayout.h"
#include "Engine/AssetManager.h"
#include "Framework/Data/RTSCore_GameDataAsset.h"
#include "Framework/Data/RTSCore_StaticGameData.h"
#include "Framework/Debug/RTSCore_Debug.h"
#include "Framework/Player/RTSCore_PlayerController.h"
#include "Framework/UI/MultiplayerMenu/RTSCore_ButtonBase.h"
#include "Framework/UI/MultiplayerMenu/RTSCore_GameDisplayListWidget.h"
#include "Framework/UI/MultiplayerMenu/RTSCore_GameDisplayWidget.h"

class UCommonUserSubsystem;

void URTSCore_HostSessionWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	NetMode = ECommonSessionOnlineMode::Online;
	InitTextDisplays();

	if(ChangeNetModeButton)
	{
		ChangeNetModeButton->OnClicked().AddUObject(this, &URTSCore_HostSessionWidget::OnNetworkModeButtonClicked);
	}

	if(ChangeGameSettingDifficultyButton)
	{
		ChangeGameSettingDifficultyButton->OnClicked().AddUObject(this, &URTSCore_HostSessionWidget::OnGameSettingDifficultyChanged);
	}

	if(ChangeGameSettingVictoryConditionButton)
	{
		ChangeGameSettingVictoryConditionButton->OnClicked().AddUObject(this, &URTSCore_HostSessionWidget::OnGameSettingVictoryConditionChanged);
	}

	if(LaunchButton)
	{
		LaunchButton->OnClicked().AddUObject(this, &URTSCore_HostSessionWidget::OnLaunchGame);
	}

	if(BackButton)
	{
		BackButton->OnClicked().AddUObject(this, &URTSCore_HostSessionWidget::OnBackMenu);
	}	

	if(GameList)
	{
		GameList->OnGameListCreated.AddDynamic(this, &URTSCore_HostSessionWidget::OnGameListCreated);
		GameList->OnGameSelected.AddDynamic(this, &URTSCore_HostSessionWidget::OnGameSelected);
	}
}

void URTSCore_HostSessionWidget::InitTextDisplays() const
{
	if(NetworkModeText)
	{
		NetworkModeText->SetText(UEnum::GetDisplayValueAsText(NetMode));
	}
	
	if(GameSettingDifficultyText)
	{
		GameSettingDifficultyText->SetText(UEnum::GetDisplayValueAsText(GameSetting_Difficulty));
	}
	
	if(GameSettingVictoryConditionText)
	{
		GameSettingVictoryConditionText->SetText(UEnum::GetDisplayValueAsText(GameSetting_VictoryCondition));
	}
}

bool URTSCore_HostSessionWidget::AttemptOnlineLogin()
{
	if(GetOwningPlayer() && GetOwningPlayer()->GetWorld())
	{
		if(const UCommonGameInstance* GameInstance = Cast<UCommonGameInstance>(GetOwningPlayer()->GetWorld()->GetGameInstance()))
		{
			if(UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>())
			{
				UserSubsystem->OnUserInitializeComplete.AddDynamic(this, &URTSCore_HostSessionWidget::OnUserOnlineLogin);
				return UserSubsystem->TryToLoginForOnlinePlay(0);
			}
		}
	}
	
	return false;
}

void URTSCore_HostSessionWidget::HostSession()
{
	if(!GetOwningPlayer() || !GetOwningPlayer()->GetWorld())
	{
		return;
	}

	if(const UCommonGameInstance* GameInstance = Cast<UCommonGameInstance>(GetOwningPlayer()->GetWorld()->GetGameInstance()))
	{
		if(UCommonSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UCommonSessionSubsystem>())
		{
			SessionSubsystem->OnCreateSessionCompleteEvent.AddUObject(this, &URTSCore_HostSessionWidget::OnSessionCreated);
			UCommonSession_HostSessionRequest* Request = CreateHostingRequest();
			SessionSubsystem->HostSession(GetOwningPlayer(), Request);
		}
	}
}

UCommonSession_HostSessionRequest* URTSCore_HostSessionWidget::CreateHostingRequest() const
{
	UCommonSession_HostSessionRequest* Request = NewObject<UCommonSession_HostSessionRequest>();

	if(GameDataId.IsValid())
	{
		if (const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
		{
			if(const URTSCore_GameDataAsset* GameData = Cast<URTSCore_GameDataAsset>(AssetManager->GetPrimaryAssetObject(GameDataId)))
			{
				const FString GameDataName = GameDataId.PrimaryAssetName.ToString();
				Request->ModeNameForAdvertisement = GameDataName;
				Request->OnlineMode = ECommonSessionOnlineMode::Online;
				Request->bUseLobbies = true;
				Request->MapID = GameData->MapID;				
				Request->ExtraArgs = GameData->ExtraArgs;
				Request->ExtraArgs.Add(RTS_MP_SETTINGS_GAMEMODE, GameDataName);
				Request->MaxPlayerCount = GameData->MaxPlayerCount;
			}
		}
	}

	return Request;
}

void URTSCore_HostSessionWidget::OnUserOnlineLogin(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error,
                                                ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext)
{
	if(bSuccess)
	{
		HostSession();
	}
	else
	{
		UE_LOG(LogRTSCore, Error, TEXT("[URTSCore_HostSessionWidget::OnUserOnlineLogin] Failed to Login Online"));
	}
}

void URTSCore_HostSessionWidget::OnSessionCreated(const FOnlineResultInformation& Result)
{
	if(Result.bWasSuccessful)
	{
		UE_LOG(LogRTSCore, Warning, TEXT("[URTSCore_HostSessionWidget::OnSessionCreated] Session Created"));
	}  
	else
	{
		UE_LOG(LogRTSCore, Error, TEXT("[URTSCore_HostSessionWidget::OnSessionCreated] Failed to Create Session"));
	}
}

void URTSCore_HostSessionWidget::OnLaunchGame()
{
	if(AttemptOnlineLogin())
	{
		if(ARTSCore_PlayerController* RTSCorePlayer = Cast<ARTSCore_PlayerController>(GetOwningPlayer()))
		{
			RTSCorePlayer->ReportClientGamePhaseTaskComplete(HostMenuTaskId);
		}
	}
}

void URTSCore_HostSessionWidget::OnBackMenu()
{
	if(const UWorld* WorldContext = GetWorld())
	{
		if (UPrimaryGameLayout* RootLayout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(WorldContext))
		{
			RootLayout->FindAndRemoveWidgetFromLayer(this);
		}
	}
}

void URTSCore_HostSessionWidget::OnGameListCreated()
{
	TArray<UWidget*> DisplayWidgets = GameList->GetGameListWidgetList();
	if(DisplayWidgets.IsValidIndex(0))
	{
		if(const URTSCore_GameDisplayWidget* DisplayWidget = Cast<URTSCore_GameDisplayWidget>(DisplayWidgets[0]))
		{
			const FPrimaryAssetId& DisplayGameData = DisplayWidget->GetGameDataAsset();
			OnGameSelected(DisplayGameData);
		}
	}
}

void URTSCore_HostSessionWidget::OnGameSelected(const FPrimaryAssetId& SelectedGameData)
{
	if(!SelectedGameData.IsValid())
	{
		return;
	}

	GameDataId = SelectedGameData;

	if (const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		if(const URTSCore_GameDataAsset* GameData = Cast<URTSCore_GameDataAsset>(AssetManager->GetPrimaryAssetObject(SelectedGameData)))
		{
			if(SelectedGameTitleText && GameData)
			{
				SelectedGameTitleText->SetText(GameData->GameName);
			}
		}
	}
}

void URTSCore_HostSessionWidget::OnGameSettingDifficultyChanged()
{
	// Increment difficulty setting
	GameSetting_Difficulty = static_cast<ERTSCore_GameSetting_Difficulty>((GameSetting_Difficulty + 1) % RTS_GAMESETTING_DIFFICULTY_COUNT);

	// Update text
	if(GameSettingDifficultyText)
	{
		GameSettingDifficultyText->SetText(UEnum::GetDisplayValueAsText(GameSetting_Difficulty));
	}
}

void URTSCore_HostSessionWidget::OnGameSettingVictoryConditionChanged()
{
	// Increment victory setting
	GameSetting_VictoryCondition = static_cast<ERTSCore_GameSetting_VictoryCondition>((GameSetting_VictoryCondition + 1) % RTS_GAMESETTING_VICTORYCOND_COUNT);
	
	// Update text
	if(GameSettingVictoryConditionText)
	{
		GameSettingVictoryConditionText->SetText(UEnum::GetDisplayValueAsText(GameSetting_VictoryCondition));
	}	
}

void URTSCore_HostSessionWidget::OnNetworkModeButtonClicked()
{
	// Increment net-mode setting
	NetMode = static_cast<ECommonSessionOnlineMode>((static_cast<int>(NetMode) + 1) % 3);

	if(NetworkModeText)
	{
		NetworkModeText->SetText(UEnum::GetDisplayValueAsText(NetMode));
	}
}
