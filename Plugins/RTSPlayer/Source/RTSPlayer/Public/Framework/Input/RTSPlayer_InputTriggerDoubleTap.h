﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputTriggers.h"
#include "RTSPlayer_InputTriggerDoubleTap.generated.h"

/**
 * 
 */
UCLASS(NotBlueprintable, MinimalAPI, meta = (DisplayName = "Double Tap", NotInputConfigurable = "true"))
class URTSPlayer_InputTriggerDoubleTap : public UInputTrigger
{
	GENERATED_BODY()

public: 
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Trigger Settings", meta = (DisplayThumbnail = "false"))
	float Delay = .5f;
	
protected:
	virtual ETriggerState UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue ModifiedValue, float DeltaTime) override;
 
private:
	float LastTappedTime = 0.f;
};
