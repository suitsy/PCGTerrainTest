// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSCore_StaticGameData.h"
#include "Engine/DataAsset.h"
#include "RTSCore_GameDataAsset.generated.h"

class UCommonActivatableWidget;
class UCommonUserWidget;

/**
 *  RTSCore GameData asset defines available game types and modes and all related data,
 *  including display data
 */
UCLASS()
class RTSCORE_API URTSCore_GameDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Type of this item, set in native parent class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Asset Settings")
	FPrimaryAssetType DataType;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(DataType, GetFName());
	}

	/** The specific map to load */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=GameData, meta=(AllowedTypes="Map"))
	FPrimaryAssetId MapID;

	/** Extra arguments passed as URL options to the game */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=GameData)
	TMap<FString, FString> ExtraArgs;

	/** Maximum players able to join the game */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Teams, meta=(AllowedTypes=TeamData))
	TArray<FPrimaryAssetId> AvailableTeams;

	/** Maximum players able to join the game */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=GameData)
	int32 MaxPlayerCount;

	/** The name of the map **/
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = GameData)
	FText GameMap;

	/** The name of the game type **/
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = GameData)
	FText GameName;

	/** Description of the game type **/
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = GameData)
	FText GameDesc;

	/** Image for the game button **/
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = GameData)
	TSoftObjectPtr<UTexture2D> GameImage;

	/** If true, this will show the game menu on loading */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=GameData)
	bool bShowMenu = true;

	/** If true, this game will show in the game list when hosting a game */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=GameData)
	bool bIncludeGameList = true;

	/** UI class that is the multiplayer main menu **/
	UPROPERTY(EditAnywhere, Category = UI)
	TSoftClassPtr<UCommonActivatableWidget> MultiplayerMenuClass;

	/** UI class that is the game type display widget **/
	UPROPERTY(EditAnywhere, Category = UI)
	TSoftClassPtr<UCommonUserWidget> GameDisplayWidgetClass;
};
