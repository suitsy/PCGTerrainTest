// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "Framework/Data/RTSEntities_DataTypes.h"
#include "RTSEntities_DeveloperSettings.generated.h"


class URogueQuery;
class URTSCore_GameStateComponent;
class UEnvQuery;
class URTSEntities_DebugKnownTarget;
class URTSEntities_DebugSelected;
class URTSEntities_DebugSelectedDisplayWidget;
class URTSEntities_BoxSelectWidget;

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="RTS Entities Settings"))
class RTSENTITIES_API URTSEntities_DeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	URTSEntities_DeveloperSettings();	
	
	UPROPERTY(Config, EditDefaultsOnly, Category = Entities)
	TMap<ERTSCore_EntityClass, TSoftClassPtr<APawn>> EntitiesMap;

	UPROPERTY(Config, EditDefaultsOnly, Category = Entities)
	bool bAllowGroupSelectionOnly = false;

	UPROPERTY(Config, EditDefaultsOnly, Category = Entities)
	bool bAllowSubGroups = false;

	UPROPERTY(Config, EditDefaultsOnly, Category = "Command System")
	int32 CommandHistorySize = 9;

	UPROPERTY(Config, EditAnywhere, Category = "Command System")
	float PreviewInterval = 0.2f;

	UPROPERTY(Config, EditDefaultsOnly, Category=Input)
	TSoftClassPtr<URTSEntities_BoxSelectWidget> BoxSelectionClass;

	UPROPERTY(Config, EditDefaultsOnly, Category=Debugging)
	TSoftClassPtr<URTSEntities_DebugSelected> DebugSelectedWidgetClass;

	UPROPERTY(Config, EditDefaultsOnly, Category=Debugging)
	TSoftClassPtr<URTSEntities_DebugSelectedDisplayWidget> DebugSelectedDisplayWidgetClass;
	
	UPROPERTY(Config, EditDefaultsOnly, Category=Debugging)
	TSoftClassPtr<URTSEntities_DebugKnownTarget> DebugKnownTargetsWidgetClass;

	UPROPERTY(Config, EditDefaultsOnly, Category="Environment Queries")
	TSoftClassPtr<UEnvQuery> QueryCoverPositions;
	
	UPROPERTY(Config, EditDefaultsOnly, Category = "Rogue Queries", meta = (AllowedTypes = "QueryData"))
	FPrimaryAssetId FormationPositionsQuery;
};
