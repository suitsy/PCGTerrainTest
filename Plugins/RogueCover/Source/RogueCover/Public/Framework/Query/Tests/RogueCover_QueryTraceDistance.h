// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "RogueCover_QueryTraceDistance.generated.h"

/**
 * 
 */
UCLASS()
class ROGUECOVER_API URogueCover_QueryTraceDistance : public UEnvQueryTest
{
	GENERATED_UCLASS_BODY()
	
	/** trace data */
	UPROPERTY(EditDefaultsOnly, Category = Trace)
	FEnvTraceData TraceData;

	/** context: other end of trace test */
	UPROPERTY(EditDefaultsOnly, Category = Trace)
	TSubclassOf<UEnvQueryContext> Context;

	/** max trace distance */
	UPROPERTY(EditDefaultsOnly, Category = Trace)
	FAIDataProviderFloatValue MaxTraceDistance;

	UPROPERTY(EditDefaultsOnly, Category = Trace)
	FAIDataProviderFloatValue VerticalOffset;

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

protected:
	bool RunLineTraceTo(const FVector& ItemPos, const FVector& ContextPos, const AActor* ItemActor, const UWorld* World,
		enum ECollisionChannel Channel, const FCollisionQueryParams& Params) const;
};
