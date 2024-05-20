// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "RTSAi_GroupPositions.generated.h"

/**
 * 
 */
UCLASS()
class RTSAI_API URTSAi_GroupPositions : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	URTSAi_GroupPositions(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, Category = Direction)
	TSubclassOf<UEnvQueryContext> Context;

	/** max trace distance */
	UPROPERTY(EditDefaultsOnly, Category = Trace)
	FAIDataProviderFloatValue MaxTraceDistance;

	UPROPERTY(EditDefaultsOnly, Category = Positions)
	FAIDataProviderFloatValue PositionSpacing;

	UPROPERTY(EditDefaultsOnly, Category = Positions)
	FAIDataProviderIntValue PositionsAmount;

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	virtual FText GetDescriptionDetails() const override;

protected:
	bool RunLineTraceTo(const FVector& ItemPos, const FVector& ContextPos, AActor* ItemActor, UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params) const;
	void CreateSourcePositions(TArray<FVector>& PositionsArray, const FVector& SourcePosition, const float Spacing, const int32 Count) const;
	
	UPROPERTY()
	FEnvTraceData TraceData;

	UPROPERTY()
	FVector Offset = FVector(0.f, 1.f, 0.f);
	
};
