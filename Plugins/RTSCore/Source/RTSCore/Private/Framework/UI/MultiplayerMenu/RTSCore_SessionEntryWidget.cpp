// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UI/MultiplayerMenu/RTSCore_SessionEntryWidget.h"
#include "CommonGameInstance.h"
#include "CommonSessionSubsystem.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Framework/Data/RTSCore_GameDataAsset.h"
#include "Framework/Data/RTSCore_StaticGameData.h"
#include "Framework/Debug/RTSCore_Debug.h"
#include "Framework/UI/MultiplayerMenu/RTSCore_SessionButtonWidget.h"

void URTSCore_SessionEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if(SessionButton)
	{
		SessionButton->OnClicked().AddUObject(this, &URTSCore_SessionEntryWidget::OnSessionSelected);
	}
}

void URTSCore_SessionEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	SessionSearchResult = Cast<UCommonSession_SearchResult>(ListItemObject);

	if(SessionSearchResult != nullptr)
	{
		FString GameDataName;
		bool bDataFound;
		SessionSearchResult->GetStringSetting(RTS_MP_SETTINGS_GAMEMODE, GameDataName, bDataFound);

		if(bDataFound)
		{
			const FPrimaryAssetType Type(RTS_DATA_ASSET_TYPE_GAMEDATA_TEXT);
			GameDataId = FPrimaryAssetId(Type, FName(*GameDataName));

			if(GameDataId.IsValid())
			{
				if(UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
				{
					TArray<FName> Bundles;
					Bundles.Add(FName(""));
					const FStreamableDelegate DataLoadedDelegate = FStreamableDelegate::CreateUObject(this, &URTSCore_SessionEntryWidget::OnGameDataLoaded);
					AssetManager->LoadPrimaryAsset(GameDataId, Bundles, DataLoadedDelegate);
				}
			}
			else
			{
				UE_LOG(LogRTSMultiMenu, Error, TEXT("[%s] - GameDataAssetId is not Valid!"), *GetClass()->GetSuperClass()->GetName());
			}
		}
	}
}

void URTSCore_SessionEntryWidget::OnSessionSelected()
{
	if(!GetOwningPlayer() || !GetOwningPlayer()->GetWorld())
	{
		return;
	}

	if(const UCommonGameInstance* GameInstance = Cast<UCommonGameInstance>(GetOwningPlayer()->GetWorld()->GetGameInstance()))
	{
		if(UCommonSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UCommonSessionSubsystem>())
		{
			JoinSessionHandle = SessionSubsystem->OnJoinSessionCompleteEvent.AddUObject(this, &URTSCore_SessionEntryWidget::OnJoinSessionComplete);
			SessionSubsystem->JoinSession(GetOwningPlayer(), SessionSearchResult);
		}
	}
}

void URTSCore_SessionEntryWidget::OnJoinSessionComplete(const FOnlineResultInformation& Result)
{
	if(!GetOwningPlayer() || !GetOwningPlayer()->GetWorld())
	{
		return;
	}

	if(Result.bWasSuccessful)
	{
		if(const UCommonGameInstance* GameInstance = Cast<UCommonGameInstance>(GetOwningPlayer()->GetWorld()->GetGameInstance()))
		{
			if(UCommonSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UCommonSessionSubsystem>())
			{
				SessionSubsystem->OnJoinSessionCompleteEvent.Remove(JoinSessionHandle);				
			}
		}
	}
}

void URTSCore_SessionEntryWidget::OnGameDataLoaded()
{
	if(GameDataId.IsValid())
	{
		if (const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
		{
			if(const URTSCore_GameDataAsset* GameData = Cast<URTSCore_GameDataAsset>(AssetManager->GetPrimaryAssetObject(GameDataId)))
			{
				if(SessionButton)
				{
					SessionButton->SetTextDisplays(
						GameData->GameName,
						GameData->GameMap,
						FText::AsNumber(SessionSearchResult->GetPingInMs()),
						FText::AsNumber(SessionSearchResult->GetMaxPublicConnections() - SessionSearchResult->GetNumOpenPublicConnections()),
						FText::AsNumber(SessionSearchResult->GetMaxPublicConnections())
					);
				}
			}
		}
	}
}
