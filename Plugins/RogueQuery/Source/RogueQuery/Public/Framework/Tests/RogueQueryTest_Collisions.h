// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueQueryTest.h"
#include "RogueQueryTest_Collisions.generated.h"

DECLARE_DELEGATE_RetVal_NineParams(float, FRunTraceSignature, FHitResult&, const FVector&, const FVector&, const float, AActor*, UWorld*, enum ECollisionChannel, const FCollisionQueryParams&, const FVector&);

UCLASS(MinimalAPI)
class URogueQueryTest_Collisions : public URogueQueryTest
{
	GENERATED_BODY()

public:
	URogueQueryTest_Collisions(const FObjectInitializer& ObjectInitializer);
	virtual void PostLoad() override;

	/** Z offset from source */
	UPROPERTY(EditDefaultsOnly, Category=Trace)
	FAIDataProviderIntValue TraceCount;

	/** invert scoring, higher scores for furthest from collision */
	UPROPERTY(EditDefaultsOnly, Category=Trace)
	FAIDataProviderBoolValue InvertScore;

	/** Z offset from source */
	UPROPERTY(EditDefaultsOnly, Category=Trace)
	FAIDataProviderFloatValue MaxTraceDistance;
	
	/** trace data */
	UPROPERTY(EditDefaultsOnly, Category=Trace, AdvancedDisplay)
	FRogueQueryTraceData TraceData;

	/** Z offset from item */
	UPROPERTY(EditDefaultsOnly, Category=Trace, AdvancedDisplay)
	FAIDataProviderFloatValue ItemHeightOffset;

	/** Z offset from source */
	UPROPERTY(EditDefaultsOnly, Category=Trace, AdvancedDisplay)
	FAIDataProviderFloatValue SourceHeightOffset;

	virtual void RunTest(FRogueQueryInstance& QueryInstance) const override;

protected:
	float RunLineTrace(FHitResult& HitResult, const FVector& ItemPos, const FVector& Direction, const float Distance, AActor* ItemActor, UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params, const FVector& Extent) const;
	float RunBoxTrace(FHitResult& HitResult, const FVector& ItemPos, const FVector& Direction, const float Distance, AActor* ItemActor, UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params, const FVector& Extent) const;
	float RunSphereTrace(FHitResult& HitResult, const FVector& ItemPos, const FVector& Direction, const float Distance, AActor* ItemActor, UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params, const FVector& Extent) const;
	float RunCapsuleTrace(FHitResult& HitResult, const FVector& ItemPos, const FVector& Direction, const float Distance, AActor* ItemActor, UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params, const FVector& Extent) const;
};
