// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueQueryTest.h"
#include "RogueQueryTest_Distance.generated.h"

UENUM()
namespace ERogueTestDistance
{
	enum Type : int
	{
		Distance3D,
		Distance2D,
		DistanceZ,
		DistanceAbsoluteZ UMETA(DisplayName = "Distance Z (Absolute)")
	};
}

UCLASS(BlueprintType, Blueprintable)
class ROGUEQUERY_API URogueQueryTest_Distance : public URogueQueryTest
{
	GENERATED_BODY()

public:
	URogueQueryTest_Distance(const FObjectInitializer& ObjectInitializer);
	
	/** testing mode */
	UPROPERTY(EditDefaultsOnly, Category=Distance)
	TEnumAsByte<ERogueTestDistance::Type> TestMode;

	virtual void RunTest(FRogueQueryInstance& QueryInstance) const override;

protected:
	FORCEINLINE float CalcDistance3D(const FVector& PosA, const FVector& PosB) const
	{
		// Static cast this to a float, for EQS scoring purposes float precision is OK.
		return static_cast<float>(FVector::Distance(PosA, PosB));
	}

	FORCEINLINE float CalcDistance2D(const FVector& PosA, const FVector& PosB) const
	{
		// Static cast this to a float, for EQS scoring purposes float precision is OK.
		return static_cast<float>(FVector::Dist2D(PosA, PosB));
	}

	FORCEINLINE float CalcDistanceZ(const FVector& PosA, const FVector& PosB) const
	{
		// Static cast this to a float, for EQS scoring purposes float precision is OK.
		return static_cast<float>(PosB.Z - PosA.Z);
	}

	FORCEINLINE float CalcDistanceAbsoluteZ(const FVector& PosA, const FVector& PosB) const
	{
		// Static cast this to a float, for EQS scoring purposes float precision is OK.
		return static_cast<float>(FMath::Abs(PosB.Z - PosA.Z));
	}
};
