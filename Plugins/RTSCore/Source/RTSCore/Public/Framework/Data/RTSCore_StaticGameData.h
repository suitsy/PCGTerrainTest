#pragma once

#include "NativeGameplayTags.h"


// Trace used for terrain finding, will pass through all assets to the terrain
#define RTS_TRACE_CHANNEL_TERRAIN				ECC_GameTraceChannel5
#define RTS_TRACE_CHANNEL_SELECTABLE			ECC_GameTraceChannel4
#define RTS_TRACE_CHANNEL_ENTITIES				ECC_GameTraceChannel3


// Asset manager data types
#define RTS_DATA_ASSET_TYPE_ENTITYDATA			FName(TEXT("EntityData"))
#define RTS_DATA_ASSET_TYPE_GROUPDATA			FName(TEXT("GroupData"))
#define RTS_DATA_ASSET_TYPE_LOADOUTDATA			FName(TEXT("LoadoutData"))
#define RTS_DATA_ASSET_TYPE_FORMATION			FName(TEXT("FormationData"))
#define RTS_DATA_ASSET_TYPE_GAMEDATA			FName(TEXT("GameData"))
#define RTS_DATA_ASSET_TYPE_GAMEDATA_TEXT		TEXT("GameData")
#define RTS_DATA_ASSET_TYPE_GAMEDATA_DEFAULT	FName(TEXT("DA_GameData_Default"))
#define RTS_DATA_ASSET_TYPE_AIDATA				FName(TEXT("AiData"))
#define RTS_DATA_ASSET_TYPE_EQUIPMENT			FName(TEXT("EquipmentData"))
#define RTS_DATA_ASSET_TYPE_QUERYDATA			FName(TEXT("QueryData"))

#define RTS_DATA_ASSET_TYPE_TEAMDATA			FName(TEXT("TeamData"))
#define RTS_DATA_ASSET_TYPE_TEAMDATA_TEXT		TEXT("TeamData")
#define RTS_DATA_ASSET_TYPE_TEAMDATA_DEFAULT	FName(TEXT("DA_TeamData_Default"))

// Multiplayer settings
#define RTS_MP_SETTINGS_GAMEMODE				TEXT("GAMEMODE")

// Asset bundles
#define RTS_DATA_ASSET_BUNDLE_GAME				FName(TEXT("Game"))

// Material parameter references
#define RTS_MAT_PARAM_NAME_TEXTURE				FName(TEXT("Texture"))
#define RTS_MAT_PARAM_NAME_HOVER				FName(TEXT("Hover_Animate"))
#define RTS_MAT_PARAM_NAME_TEX_ALPHA			FName(TEXT("TexAlpha"))
#define RTS_MAT_PARAM_NAME_TEX_ALPHA_HOVER		FName(TEXT("TexAlphaHover"))
#define RTS_MAT_PARAM_NAME_TEX_SCALE			FName(TEXT("TexScale"))
#define RTS_MAT_PARAM_NAME_TEX_HOVERSCALE		FName(TEXT("TexScaleHover"))
#define RTS_MAT_PARAM_NAME_TEX_SHIFTX			FName(TEXT("Shift_X"))
#define RTS_MAT_PARAM_NAME_TEX_SHIFTY			FName(TEXT("Shift_Y"))


namespace RTSCore_UILayerTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_LAYER_GAME);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_LAYER_GAMEMENU);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_LAYER_MENU);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_LAYER_MODAL);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_LAYER_DEBUG);
}

// Asset manager bundles
//#define RTSP_DATA_ASSET_BUNDLE_INITLOAD				FName(TEXT("InitLoad"))

