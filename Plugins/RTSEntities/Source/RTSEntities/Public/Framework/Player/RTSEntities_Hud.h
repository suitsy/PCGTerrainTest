// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Player/RTSCore_Hud.h"
#include "RTSEntities_Hud.generated.h"

class URTSEntities_DebugSelected;

/**
 * 
 */
UCLASS()
class RTSENTITIES_API ARTSEntities_Hud : public ARTSCore_Hud
{
	GENERATED_BODY()

public:
	explicit ARTSEntities_Hud(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void ShowSelectedDebug(const TSoftClassPtr<URTSEntities_DebugSelected>& UIClass);
	
protected:
	// Debug UI
	
};
