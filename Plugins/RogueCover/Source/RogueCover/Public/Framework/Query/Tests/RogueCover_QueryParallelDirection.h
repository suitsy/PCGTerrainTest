// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "RogueCover_QueryParallelDirection.generated.h"

/**
 * 
 */
UCLASS()
class ROGUECOVER_API URogueCover_QueryParallelDirection : public UEnvQueryTest
{
	GENERATED_UCLASS_BODY()

	/** Context: other end of trace test */
	UPROPERTY(EditDefaultsOnly, Category = Trace)
	TSubclassOf<UEnvQueryContext> Context;

	/** Max trace distance */
	UPROPERTY(EditDefaultsOnly, Category = Trace)
	FAIDataProviderFloatValue MaxEpsilon;

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
};
