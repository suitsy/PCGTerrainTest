// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "RogueQuery_Settings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Rogue Settings"))
class ROGUEQUERY_API URogueQuery_Settings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	URogueQuery_Settings();

	// Debugging
	UPROPERTY(Config, EditAnywhere, Category="RogueQuery|Debugging")
	uint8 DebugQuery:1;
	
	UPROPERTY(Config, EditAnywhere, Category="RogueQuery|Debugging")
	uint8 DebugDrawQuery:1;
	
	UPROPERTY(Config, EditAnywhere, Category="RogueQuery|Debugging")
	uint8 DebugDrawQueryGenerator:1;
	
	UPROPERTY(Config, EditAnywhere, Category="RogueQuery|Debugging")
	uint8 DisableTickDebug:1;
};
