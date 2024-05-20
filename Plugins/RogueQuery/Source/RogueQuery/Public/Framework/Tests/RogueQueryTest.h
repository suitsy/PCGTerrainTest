// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataProviders/AIDataProvider.h"
#include "Framework/Data/RogueQuery_DataTypes.h"
#include "RogueQueryTest.generated.h"

/**
 * 
 */
UCLASS(Abstract, MinimalAPI, BlueprintType)
class URogueQueryTest : public UDataAsset
{
	GENERATED_BODY()

public:
	URogueQueryTest(const FObjectInitializer& ObjectInitializer);
	virtual void PostLoad() override;
	
	/** The purpose of this test.  Should it be used for filtering possible results, scoring them, or both? */
	UPROPERTY(EditDefaultsOnly, Category=Test)
	TEnumAsByte<ERogueTestPurpose::Type> TestPurpose;

	/** Does this test filter out results that are below a lower limit, above an upper limit, or both?  Or does it just look for a matching value? */
	UPROPERTY(EditDefaultsOnly, Category=Filter)
	TEnumAsByte<ERogueTestFilterType::Type> FilterType;

	/** Minimum limit (inclusive) of valid values for the raw test value. Lower values will be discarded as invalid. */
	UPROPERTY(EditDefaultsOnly, Category=Filter)
	FAIDataProviderFloatValue FloatValueMin;

	/** Maximum limit (inclusive) of valid values for the raw test value. Higher values will be discarded as invalid. */
	UPROPERTY(EditDefaultsOnly, Category=Filter)
	FAIDataProviderFloatValue FloatValueMax;

	/** The shape of the curve equation to apply to the normalized score before multiplying by factor. */
	UPROPERTY(EditDefaultsOnly, Category=Score)
	TEnumAsByte<ERogueTestScoreEquation::Type> ScoringEquation;

	/** The weight (factor) by which to multiply the normalized score after the scoring equation is applied. */
	UPROPERTY(EditDefaultsOnly, Category=Score, Meta=(ClampMin="0.001", UIMin="0.001"))
	FAIDataProviderFloatValue ScoringFactor;

	/** Specifies how to determine value span used to normalize scores */
	UPROPERTY(EditDefaultsOnly, Category = Score)
	TEnumAsByte<ERogueTestScoreNormalization::Type> NormalizationType; 

	/** How should the lower bound for normalization of the raw test value before applying the scoring formula be determined?
		Should it use the lowest value found (tested), the lower threshold for filtering, or a separate specified normalization minimum? */
	UPROPERTY(EditDefaultsOnly, Category=Score)
	TEnumAsByte<ERogueQueryTestClamping::Type> ClampMinType;

	/** How should the upper bound for normalization of the raw test value before applying the scoring formula be determined?
		Should it use the highest value found (tested), the upper threshold for filtering, or a separate specified normalization maximum? */
	UPROPERTY(EditDefaultsOnly, Category=Score)
	TEnumAsByte<ERogueQueryTestClamping::Type> ClampMaxType;

	/** Minimum value to use to normalize the raw test value before applying scoring formula. */
	UPROPERTY(EditDefaultsOnly, Category=Score)
	FAIDataProviderFloatValue ScoreClampMin;

	/** Maximum value to use to normalize the raw test value before applying scoring formula. */
	UPROPERTY(EditDefaultsOnly, Category=Score)
	FAIDataProviderFloatValue ScoreClampMax;

	/** Determines filtering operator when context returns multiple items */
	UPROPERTY(EditDefaultsOnly, Category=Filter, AdvancedDisplay)
	TEnumAsByte<ERogueTestFilterOperator::Type> MultipleContextFilterOp;

	/** Determines scoring operator when context returns multiple items */
	UPROPERTY(EditDefaultsOnly, Category = Score, AdvancedDisplay)
	TEnumAsByte<ERogueTestScoreOperator::Type> MultipleContextScoreOp;

	virtual void RunTest(FRogueQueryInstance& QueryInstance) const { checkNoEntry(); }
	void NormalizeItemScores(FRogueQueryInstance& QueryInstance);
	FORCEINLINE bool IsScoring() const { return (TestPurpose != ERogueTestPurpose::Filter); } 
	FORCEINLINE bool IsFiltering() const { return (TestPurpose != ERogueTestPurpose::Score); }
	ROGUEQUERY_API FVector GetItemLocation(FRogueQueryInstance& QueryInstance, int32 ItemIndex) const;
};
