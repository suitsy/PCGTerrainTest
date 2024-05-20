// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/RTSCore_DataTypes.h"
#include "UObject/Interface.h"
#include "RTSCore_AiControllerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class RTSCORE_API URTSCore_AiControllerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RTSCORE_API IRTSCore_AiControllerInterface
{
	GENERATED_BODY()

public:
	virtual FCollisionShape GetCollisionShape() const = 0;
	virtual ERTSCore_EntityType GetEntityType() = 0;
};
