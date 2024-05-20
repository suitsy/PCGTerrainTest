// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UI/MultiplayerMenu/RTSCore_GameButtonWidget.h"
#include "CommonTextBlock.h"
#include "Engine/AssetManager.h"
#include "Framework/Data/RTSCore_GameDataAsset.h"

void URTSCore_GameButtonWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void URTSCore_GameButtonWidget::SetGameData(const FPrimaryAssetId& GameDataAssetId)
{
	if(GameDataAssetId.IsValid())
	{
		if (const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
		{
			if(const URTSCore_GameDataAsset* GameData = Cast<URTSCore_GameDataAsset>(AssetManager->GetPrimaryAssetObject(GameDataAssetId)))
			{
				InitButton();
				UpdateButtonText(GameData->GameMap, GameData->GameName, GameData->GameDesc);
				if(GameData->GameImage.LoadSynchronous())
				{
					ButtonTexture = GameData->GameImage.LoadSynchronous();
					SetButtonSettings();
				}
			}
		}
	}
}

void URTSCore_GameButtonWidget::UpdateButtonText(const FText& GameMapText, const FText& GameNameText,
	const FText& GameDescText)
{
	if(ButtonTextBlock)
	{
		ButtonText = GameMapText;
		SetButtonText(GameMapText);
	}

	if(Name)
	{
		Name->SetText(GameNameText);
	}

	if(Desc)
	{
		Desc->SetText(GameDescText);
	}
}
