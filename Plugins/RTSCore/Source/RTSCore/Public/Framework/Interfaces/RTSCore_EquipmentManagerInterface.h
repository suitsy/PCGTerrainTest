// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RTSCore_EquipmentManagerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class RTSCORE_API URTSCore_EquipmentManagerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RTSCORE_API IRTSCore_EquipmentManagerInterface
{
	GENERATED_BODY()

public:
	virtual void AddEquipmentComponent(APlayerController* NewOwningPlayer, AActor* Entity) = 0;
};
