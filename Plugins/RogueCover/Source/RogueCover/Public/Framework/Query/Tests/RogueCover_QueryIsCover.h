// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EnvironmentQuery/EnvQueryTest.h"
#include "RogueCover_QueryIsCover.generated.h"


UCLASS()
class ROGUECOVER_API URogueCover_QueryIsCover : public UEnvQueryTest
{
	GENERATED_UCLASS_BODY()

	/** Trace data */
	UPROPERTY(EditDefaultsOnly, Category = Trace)
	FEnvTraceData TraceData;

	/** Context: other end of trace test */
	UPROPERTY(EditDefaultsOnly, Category = Trace)
	TSubclassOf<UEnvQueryContext> Context;

	/** Distance offset from querier */
	UPROPERTY(EditDefaultsOnly, Category = Trace, AdvancedDisplay)
	FAIDataProviderFloatValue ContextHorizontalDistanceOffset;

	/** Distance offset from querier */
	UPROPERTY(EditDefaultsOnly, Category = Trace, AdvancedDisplay)
	FAIDataProviderFloatValue ContextVerticalDistanceOffset;

	/** Display debug info */
	UPROPERTY(EditDefaultsOnly, Category = Trace, AdvancedDisplay)
	FAIDataProviderBoolValue DebugData;

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	
protected:
	bool RunLineTraceTo(const FVector& ItemPos, const FVector& ContextPos, const AActor* ItemActor, const UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params, bool Debug = false) const;

	static FVector PerpendicularClockwise(FVector V);
};
