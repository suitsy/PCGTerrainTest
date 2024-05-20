// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "RTSAi_DeveloperSettings.generated.h"

class ACoverGenerator;


UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="RTS Ai Settings"))
class RTSAI_API URTSAi_DeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	URTSAi_DeveloperSettings();
};
