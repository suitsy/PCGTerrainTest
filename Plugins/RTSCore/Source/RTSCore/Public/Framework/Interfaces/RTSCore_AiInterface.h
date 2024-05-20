// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RTSCore_AiInterface.generated.h"

class USplineComponent;
// This class does not need to be modified.
UINTERFACE()
class RTSCORE_API URTSCore_AiInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RTSCORE_API IRTSCore_AiInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void SetObservationLocation(const FVector& Location) = 0;	
	virtual void PlayMontage(UAnimMontage* MontageToPlay) = 0;
	virtual void Crouch() = 0;
	virtual void Stand() = 0;
};
