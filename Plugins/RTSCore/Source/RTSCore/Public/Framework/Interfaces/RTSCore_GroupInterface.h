// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RTSCore_GroupInterface.generated.h"

class AAIController;

// This class does not need to be modified.
UINTERFACE()
class RTSCORE_API URTSCore_GroupInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RTSCORE_API IRTSCore_GroupInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void ReportHostiles(AAIController* Reporter, TArray<AActor*> Hostiles) = 0;
};
