// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UI/MultiplayerMenu/RTSCore_GameDisplayWidget.h"
#include "Framework/UI/MultiplayerMenu/RTSCore_GameButtonWidget.h"


void URTSCore_GameDisplayWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if(GameButtonWidget)
	{
		GameButtonWidget->OnClicked().AddUObject(this, &URTSCore_GameDisplayWidget::OnGameSelection);
	}
}

void URTSCore_GameDisplayWidget::SetGameData(const FPrimaryAssetId& Data)
{
	if(Data.IsValid() && GameButtonWidget)
	{
		GameDataId = Data;
		GameButtonWidget->SetGameData(Data);
	}
}

void URTSCore_GameDisplayWidget::OnGameSelection()
{
	OnGameDisplaySelected.Broadcast(GameDataId);
}
