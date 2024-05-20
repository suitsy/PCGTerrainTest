// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "RogueCover_QueryCoverOccupied.generated.h"

/**
 * 
 */
UCLASS()
class ROGUECOVER_API URogueCover_QueryCoverOccupied : public UEnvQueryTest
{
	GENERATED_UCLASS_BODY()
	
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
};
