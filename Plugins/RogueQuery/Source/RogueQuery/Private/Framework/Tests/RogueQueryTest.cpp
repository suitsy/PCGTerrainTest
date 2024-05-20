// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Tests/RogueQueryTest.h"

URogueQueryTest::URogueQueryTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	TestPurpose = ERogueTestPurpose::FilterAndScore;
	FilterType = ERogueTestFilterType::Range;
	ScoringEquation = ERogueTestScoreEquation::Linear;
	ClampMinType = ERogueQueryTestClamping::None;
	ClampMaxType = ERogueQueryTestClamping::None;
	ScoringFactor.DefaultValue = 1.0f;
	NormalizationType = ERogueTestScoreNormalization::Type::Absolute;
}

void URogueQueryTest::PostLoad()
{
	UObject::PostLoad();

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		if (ScoringEquation == ERogueTestScoreEquation::Linear)
		{
			// we can still address this by flipping the ScoringEquation to 
			ScoringEquation = ERogueTestScoreEquation::InverseLinear;
			UE_LOG(LogRogueQuery, Verbose, TEXT("%s using a AIDataProvider for ScoringFactor. Due to a bug-fix the effect will be flipped so ScoringEquation has been automatically flipped from Linear to InverseLinear for this EQS Test instance.")
				, *GetPathName());
		}
		else if (ScoringEquation == ERogueTestScoreEquation::InverseLinear)
		{
			// we can still address this by flipping the ScoringEquation to 
			ScoringEquation = ERogueTestScoreEquation::Linear;
			UE_LOG(LogRogueQuery, Verbose, TEXT("%s using a AIDataProvider for ScoringFactor. Due to a bug-fix the effect will be flipped so ScoringEquation has been automatically flipped from InverseLinear to Linear for this EQS Test instance.")
				, *GetPathName());
		}
		else
		{
			UE_LOG(LogRogueQuery, Warning, TEXT("%s using a AIDataProvider for ScoringFactor. Due to a bug-fix the effect will be flipped and an automated change could not be performed. Please address manually.")
				, *GetPathName());
		}
	}
}

void URogueQueryTest::NormalizeItemScores(FRogueQueryInstance& QueryInstance)
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr || !IsScoring() || QueryInstance.Items.Num() == 0)
	{
		return;
	}

	ScoringFactor.BindData(QueryOwner, QueryInstance.QueryId);
	const float ScoringFactorValue = ScoringFactor.GetValue();

	float MinScore = (NormalizationType == ERogueTestScoreNormalization::Absolute) ? 0 : BIG_NUMBER;
	float MaxScore = -BIG_NUMBER;

	if (ClampMinType == ERogueQueryTestClamping::FilterThreshold)
	{
		FloatValueMin.BindData(QueryOwner, QueryInstance.QueryId);
		MinScore = FloatValueMin.GetValue();
	}
	else if (ClampMinType == ERogueQueryTestClamping::SpecifiedValue)
	{
		ScoreClampMin.BindData(QueryOwner, QueryInstance.QueryId);
		MinScore = ScoreClampMin.GetValue();
	}

	if (ClampMaxType == ERogueQueryTestClamping::FilterThreshold)
	{
		FloatValueMax.BindData(QueryOwner, QueryInstance.QueryId);
		MaxScore = FloatValueMax.GetValue();
	}
	else if (ClampMaxType == ERogueQueryTestClamping::SpecifiedValue)
	{
		ScoreClampMax.BindData(QueryOwner, QueryInstance.QueryId);
		MaxScore = ScoreClampMax.GetValue();
	}

	FRogueQueryItemDetails* DetailInfo = QueryInstance.ItemDetails.GetData();
	if ((ClampMinType == ERogueQueryTestClamping::None) ||
		(ClampMaxType == ERogueQueryTestClamping::None)
	   )
	{
		for (int32 ItemIndex = 0; ItemIndex < QueryInstance.Items.Num(); ItemIndex++, DetailInfo++)
		{
			if (!QueryInstance.Items[ItemIndex].IsValid())
			{
				continue;
			}

			float TestValue = DetailInfo->TestResults[QueryInstance.CurrentTest];
			if (TestValue != -MAX_FLT)
			{
				if (ClampMinType == ERogueQueryTestClamping::None)
				{
					MinScore = FMath::Min(MinScore, TestValue);
				}

				if (ClampMaxType == ERogueQueryTestClamping::None)
				{
					MaxScore = FMath::Max(MaxScore, TestValue);
				}
			}
		}
	}

	DetailInfo = QueryInstance.ItemDetails.GetData();

	if (MinScore != MaxScore)
	{
		const float TargetScore = MaxScore;
		const float ValueSpan = FMath::Max(FMath::Abs(TargetScore - MinScore), FMath::Abs(TargetScore - MaxScore));
		const float AbsoluteWeight = FMath::Abs(ScoringFactorValue);

		for (int32 ItemIndex = 0; ItemIndex < QueryInstance.ItemDetails.Num(); ItemIndex++, DetailInfo++)
		{
			if (QueryInstance.Items[ItemIndex].IsValid() == false)
			{
				continue;
			}

			float WeightedScore = 0.0f;

			float& TestValue = DetailInfo->TestResults[QueryInstance.CurrentTest];
			if (TestValue != -MAX_FLT)
			{
				const float ClampedScore = FMath::Clamp(TestValue, MinScore, MaxScore);
				const float NormalizedScore = (ScoringFactorValue >= 0) 
					? (1.f - (FMath::Abs(TargetScore - ClampedScore) / ValueSpan))
					: (FMath::Abs(TargetScore - ClampedScore) / ValueSpan);

				switch (ScoringEquation)
				{
					case ERogueTestScoreEquation::Linear:
						WeightedScore = AbsoluteWeight * NormalizedScore;
						break;

					case ERogueTestScoreEquation::InverseLinear:
					{
						// For now, we're avoiding having a separate flag for flipping the direction of the curve
						// because we don't have usage cases yet and want to avoid too complex UI.  If we decide
						// to add that flag later, we'll need to remove this option, since it should just be "mirror
						// curve" plus "Linear".
						float InverseNormalizedScore = (1.0f - NormalizedScore);
						WeightedScore = AbsoluteWeight * InverseNormalizedScore;
						break;
					}

					case ERogueTestScoreEquation::Square:
						WeightedScore = AbsoluteWeight * (NormalizedScore * NormalizedScore);
						break;

					case ERogueTestScoreEquation::SquareRoot:
						WeightedScore = AbsoluteWeight * FMath::Sqrt(NormalizedScore);
						break;

					case ERogueTestScoreEquation::Constant:
						// I know, it's not "constant".  It's "Constant, or zero".  The tooltip should explain that.
						WeightedScore = (NormalizedScore > 0) ? AbsoluteWeight : 0.0f;
						break;
						
					default:
						break;
				}
			}
			else
			{
				// Do NOT clear TestValue to 0, because the SkippedItemValue is used to display "SKIP" when debugging.
				// TestValue = 0.0f;
				WeightedScore = 0.0f;
			}

			QueryInstance.Items[ItemIndex].Score += WeightedScore;
		}
	}
}

FVector URogueQueryTest::GetItemLocation(FRogueQueryInstance& QueryInstance, const int32 ItemIndex) const
{
	return QueryInstance.Items[ItemIndex].Location;
}
