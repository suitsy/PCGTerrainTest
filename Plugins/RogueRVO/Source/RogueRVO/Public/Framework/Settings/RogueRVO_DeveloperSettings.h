// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "RogueRVO_DeveloperSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Rogue RVO Settings"))
class ROGUERVO_API URogueRVO_DeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	URogueRVO_DeveloperSettings();

	// Debugging
	UPROPERTY(Config, EditAnywhere, Category=Debugging)
	bool DebugEnabled = false;
};
