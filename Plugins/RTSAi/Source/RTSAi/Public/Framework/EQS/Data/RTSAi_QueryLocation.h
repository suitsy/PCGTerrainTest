// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "RTSAi_QueryLocation.generated.h"

/**
 * 
 */
UCLASS()
class RTSAI_API URTSAi_QueryLocation : public UObject
{
	GENERATED_BODY()

	URTSAi_QueryLocation();
	explicit URTSAi_QueryLocation(const FObjectInitializer& ObjectInitializer);
	explicit URTSAi_QueryLocation(const FVector& InLocation);

	UPROPERTY()
	FVector Location;
	
	UPROPERTY()
	float RelativeScore = 0;
	
	UPROPERTY()
	URTSAi_QueryLocation* RelativeSource = nullptr;
};
