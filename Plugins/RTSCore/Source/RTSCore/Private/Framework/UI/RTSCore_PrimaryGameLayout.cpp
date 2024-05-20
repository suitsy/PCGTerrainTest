// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UI/RTSCore_PrimaryGameLayout.h"
#include "Framework/Data/RTSCore_StaticGameData.h"

void URTSCore_PrimaryGameLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	RegisterLayer(RTSCore_UILayerTags::TAG_UI_LAYER_GAME, Game_Stack);
	RegisterLayer(RTSCore_UILayerTags::TAG_UI_LAYER_GAMEMENU, GameMenu_Stack);
	RegisterLayer(RTSCore_UILayerTags::TAG_UI_LAYER_MENU, Menu_Stack);
	RegisterLayer(RTSCore_UILayerTags::TAG_UI_LAYER_MODAL, Modal_Stack);
}
