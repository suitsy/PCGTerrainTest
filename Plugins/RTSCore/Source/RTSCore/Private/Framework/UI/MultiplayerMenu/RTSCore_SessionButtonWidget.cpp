// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UI/MultiplayerMenu/RTSCore_SessionButtonWidget.h"
#include "CommonTextBlock.h"

void URTSCore_SessionButtonWidget::SetTextDisplays(const FText& Game, const FText& Map, const FText& Ping,
														  const FText& PlayerCount, const FText& MaxPlayerCount)
{
	if(ButtonTextBlock)
	{
		SetButtonText(Game);
	}

	if(MapText)
	{
		MapText->SetText(Map);
	}

	if(PingText)
	{
		PingText->SetText(Ping);
	}

	if(PlayerCountText)
	{
		PlayerCountText->SetText(PlayerCount);
	}

	if(MaxPlayerCountText)
	{
		MaxPlayerCountText->SetText(MaxPlayerCount);
	}
}
