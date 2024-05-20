// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "Framework/Data/RTSCore_DataTypes.h"
#include "RTSCore_DeveloperSettings.generated.h"

class UEnvQuery;
class URTSCore_GameStateComponent;
class UCommonActivatableWidget;

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="RTS Core Settings"))
class RTSCORE_API URTSCore_DeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	URTSCore_DeveloperSettings();

	// UI Configuration
	UPROPERTY(Config, EditAnywhere, Category=UI)
	TArray<FRTSCore_UIConfiguration> UIConfigurations;

	// Team Settings
	UPROPERTY(Config, EditDefaultsOnly, Category = Teams)
	TSoftClassPtr<URTSCore_GameStateComponent> TeamManagerComponent;

	// Debugging
	UPROPERTY(Config, EditAnywhere, Category=Debugging)
	bool DebugRTSCore = false;

	UPROPERTY(Config, EditAnywhere, Category=Debugging)
	bool DebugRTSCharacter = false;

	UPROPERTY(Config, EditAnywhere, Category=Debugging)
	bool DebugRTSEntities = false;

	UPROPERTY(Config, EditAnywhere, Category=Debugging)
	bool DebugRTSPlayer = false;

	UPROPERTY(Config, EditAnywhere, Category=Debugging)
	bool DebugRTSTeams = false;

	UPROPERTY(Config, EditAnywhere, Category=Debugging)
	bool DebugRTSAi = false;

	UPROPERTY(Config, EditAnywhere, Category=Debugging)
	bool DebugRTSVehicles = false;

	// Debugging Elements
	UPROPERTY(Config, EditAnywhere, Category="Debug|Core")
	bool DebugGamePhase = false;
	
	UPROPERTY(Config, EditAnywhere, Category="Debug|Player")
	bool DebugPlayerPawn = false;
	
	UPROPERTY(Config, EditAnywhere, Category="Debug|Player")
	bool DebugPlayerInput = false;
	
	UPROPERTY(Config, EditAnywhere, Category="Debug|Ai")
	bool DebugAiCombat = false;
	
	UPROPERTY(Config, EditAnywhere, Category="Debug|Entities")
	bool DebugEntityNavigation = false;

	UPROPERTY(Config, EditAnywhere, Category="Debug|Entities")
	bool DebugEntityFormations = false;

	UPROPERTY(Config, EditAnywhere, Category="Debug|Ai")
	bool DebugAiCollision = false;

	UPROPERTY(Config, EditAnywhere, Category="Debug|Ai")
	bool DebugAiRVO = false;

	UPROPERTY(Config, EditAnywhere, Category="Debug|Vehicles")
	bool DebugRTSVehicle_Input = false;
};
