// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/RTSCore_DataTypes.h"
#include "GameFramework/HUD.h"
#include "RTSCore_Hud.generated.h"

class UCommonActivatableWidget;

UCLASS()
class RTSCORE_API ARTSCore_Hud : public AHUD
{
	GENERATED_BODY()

public:
	explicit ARTSCore_Hud(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void BeginPlay() override;
	virtual void OnGamePhaseChanged(const ERTSCore_GamePhaseState NewState);
	virtual void ShowMainMenu(const TSoftClassPtr<UCommonActivatableWidget>& MenuClass, const FGuid TaskId);

	void LoadUIConfiguration(const FRTSCore_UIConfiguration& UIConfiguration) const;

protected:
	void SyncHudGamePhase();
	
	UPROPERTY()
	TArray<FRTSCore_UIConfiguration> PlayerUIConfigurations;

	UPROPERTY()
	ERTSCore_GamePhaseState GamePhaseState = ERTSCore_GamePhaseState::Loading;
};