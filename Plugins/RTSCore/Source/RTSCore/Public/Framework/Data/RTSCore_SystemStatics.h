// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RTSCore_SystemStatics.generated.h"

/**
 * 
 */
UCLASS()
class RTSCORE_API URTSCore_SystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	// Utility Functions
	static void GetTerrainPosition(UWorld* WorldContext, FVector& TerrainPosition);
	static void GetTerrainTransform(UWorld* WorldContext, FTransform& TerrainTransform);
	static void GetTerrainLocationAndNormal(UWorld* WorldContext, FVector& Location, FVector& Normal);
};
