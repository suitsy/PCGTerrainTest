// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UI/MultiplayerMenu/RTSCore_GameDisplayListWidget.h"
#include "Components/VerticalBox.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Framework/Data/RTSCore_GameDataAsset.h"
#include "Framework/Data/RTSCore_StaticGameData.h"
#include "Framework/Debug/RTSCore_Debug.h"
#include "Framework/UI/MultiplayerMenu/RTSCore_GameDisplayWidget.h"


void URTSCore_GameDisplayListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	LoadGameList();	
}

TArray<UWidget*> URTSCore_GameDisplayListWidget::GetGameListWidgetList() const
{
	if(GameList)
	{
		return GameList->GetAllChildren();
	}

	return TArray<UWidget*>();
}

void URTSCore_GameDisplayListWidget::LoadGameList()
{
	if(GameList)
	{
		GameList->ClearChildren();

		if(UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
		{
			TArray<FPrimaryAssetId> GameDataAssets;
			const FPrimaryAssetType GroupAssetType(RTS_DATA_ASSET_TYPE_GAMEDATA_TEXT);			
			AssetManager->GetPrimaryAssetIdList(GroupAssetType, GameDataAssets);

			if(GameDataAssets.Num() > 0)
			{
				const TArray<FName> Bundles;
				const FStreamableDelegate GroupDataLoadedDelegate = FStreamableDelegate::CreateUObject(this, &URTSCore_GameDisplayListWidget::OnGameDataLoaded, GameDataAssets);
				AssetManager->LoadPrimaryAssets(GameDataAssets, Bundles, GroupDataLoadedDelegate);
			}
			else
			{
				UE_LOG(LogRTSCore, Error, TEXT("[USGameDisplayList::LoadGameList] - Failed to Get GameDataAssets Assets"));
			}
		}
	}
}

void URTSCore_GameDisplayListWidget::OnGameDataLoaded(TArray<FPrimaryAssetId> GameDataAssets)
{
	if(!GetOwningPlayer() || !GetOwningPlayer()->GetWorld())
	{
		return;
	}

	if(const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		for (int i = 0; i < GameDataAssets.Num(); ++i)
		{
			if(const URTSCore_GameDataAsset* GameData = Cast<URTSCore_GameDataAsset>(AssetManager->GetPrimaryAssetObject(GameDataAssets[i])))
			{
				if(GameData->bIncludeGameList)
				{
					if(GameData->GameDisplayWidgetClass.IsValid())
					{
						if(URTSCore_GameDisplayWidget* GameDisplay = CreateWidget<URTSCore_GameDisplayWidget>(GetOwningPlayer()->GetWorld(), GameData->GameDisplayWidgetClass.LoadSynchronous()))
						{
							GameDisplay->SetGameData(GameDataAssets[i]);
							GameList->AddChildToVerticalBox(GameDisplay);
						}
					}
				}
			}
		}
	}

	OnGameListCreation();
}

void URTSCore_GameDisplayListWidget::OnGameListCreation()
{
	OnGameListCreated.Broadcast();

	TArray<UWidget*> DisplayWidgets = GetGameListWidgetList();
	for (int i = 0; i < DisplayWidgets.Num(); ++i)
	{
		if(URTSCore_GameDisplayWidget* DisplayWidget = Cast<URTSCore_GameDisplayWidget>(DisplayWidgets[i]))
		{
			DisplayWidget->OnGameDisplaySelected.AddDynamic(this, &URTSCore_GameDisplayListWidget::OnGameDisplaySelection);
		}
	}
	
}

void URTSCore_GameDisplayListWidget::OnGameDisplaySelection(const FPrimaryAssetId& SelectedGameData)
{
	OnGameSelected.Broadcast(SelectedGameData);
}
