// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RTSCore_GamePhaseInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class RTSCORE_API URTSCore_GamePhaseInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RTSCORE_API IRTSCore_GamePhaseInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual void SetTaskId(const FGuid TaskId) = 0;
};
