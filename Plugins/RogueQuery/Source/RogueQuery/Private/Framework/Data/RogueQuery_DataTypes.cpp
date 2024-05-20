// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Data/RogueQuery_DataTypes.h"
#include "AISystem.h"
#include "Framework/Data/RogueQuery.h"
#include "AI/Navigation/NavAgentInterface.h"
#include "Framework/Generators/RogueQueryGenerator.h"
#include "Framework/Tests/RogueQueryTest.h"

DEFINE_LOG_CATEGORY(LogRogueQuery);


void FRogueQueryTraceData::SetGeometryOnly()
{
	TraceMode = ERogueQueryTrace::GeometryByChannel;
	bCanTraceOnGeometry = true;
	bCanTraceOnNavMesh = false;
	bCanDisableTrace = false;
}

void FRogueQueryTraceData::SetNavmeshOnly()
{
	TraceMode = ERogueQueryTrace::Navigation;
	bCanTraceOnGeometry = false;
	bCanTraceOnNavMesh = true;
	bCanDisableTrace = false;
}

void FRogueQueryTraceData::OnPostLoad()
{
	if (VersionNum == 0)
	{
		// update trace channels
		SerializedChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel.GetValue());
	}

	TraceChannel = UEngineTypes::ConvertToTraceType(SerializedChannel);
	VersionNum = 1;
}

void FRogueQueryInstance::Execute()
{
	UE_LOG(LogRogueQuery, Log, TEXT("[FRogueQueryInstance] Query Execute"));
	
	if (Owner == nullptr)
	{
		MarkAsOwnerLost();
		return;
	}

	check(IsFinished() == false);
	Items.Reset();
	
	GenerateItems();	
	
	if (!Tests.IsValidIndex(0) || Items.Num() <= 0)
	{
		NumValidItems = 0;
		FinalizeQuery();
		return;
	}
	
	PerformTests();	
}

void FRogueQueryInstance::GenerateItems()
{
	if(Generator != nullptr)
	{
		Generator->GenerateItems(*this);
	}
}

void FRogueQueryInstance::PerformTests()
{	
	// Perform all tests
	for (int TestIdx = 0; TestIdx < Tests.Num(); ++TestIdx)
	{
		// Assign test index
		TestIndex = TestIdx;
		
		const bool bDoingLastTest = (CurrentTest >= Tests.Num() - 1);
		if(Tests.IsValidIndex(CurrentTest))
		{
			URogueQueryTest* TestObj = Tests[TestIndex];
			bPassOnSingleResult = (bDoingLastTest && Query->Mode == ERogueQueryRunMode::SingleResult);
			if (bPassOnSingleResult)
			{
				bool bSortTests = false;
				for (int32 i = 0; i < Tests.Num() - 1; ++i)
				{
					if (Tests[i]->TestPurpose != ERogueTestPurpose::Filter)
					{
						// Found one.  We should sort.
						bSortTests = true;
						break;
					}
				}

				if (bSortTests)
				{
					SortScores();
				}
			}

			TestObj->RunTest(*this);
			FinalizeTest();
		}

		// sort results or switch to next option when all tests are performed
		if (IsFinished() == false && (Tests.Num() == TestIndex + 1 || NumValidItems <= 0))
		{				
			if (NumValidItems > 0)
			{
				// found items, sort and finish
				FinalizeQuery();
			}
			else
			{
				// no items here, go to next option or finish			
				if (TestIndex + 1 >= Tests.Num())
				{
					// out of options, finish processing without errors
					FinalizeQuery();
				}
			}
		}
	}	
}

void FRogueQueryInstance::NormalizeScores()
{
	// @note this function assumes results have been already sorted and all first NumValidItems
	// items in Items are valid (and the rest is not).
	check(NumValidItems <= Items.Num())

	float MinScore = 0.f;
	float MaxScore = -BIG_NUMBER;

	FRogueQueryItem* ItemInfo = Items.GetData();
	for (int32 ItemIndex = 0; ItemIndex < NumValidItems; ItemIndex++, ItemInfo++)
	{
		ensure(ItemInfo->IsValid());

		MinScore = FMath::Min(MinScore, ItemInfo->Score);
		MaxScore = FMath::Max(MaxScore, ItemInfo->Score);
	}

	ItemInfo = Items.GetData();
	if (MinScore == MaxScore)
	{
		const float Score = (MinScore == 0.f) ? 0.f : 1.f;
		for (int32 ItemIndex = 0; ItemIndex < NumValidItems; ItemIndex++, ItemInfo++)
		{
			ItemInfo->Score = Score;
		}
	}
	else
	{
		const float ScoreRange = MaxScore - MinScore;
		for (int32 ItemIndex = 0; ItemIndex < NumValidItems; ItemIndex++, ItemInfo++)
		{
			if(bInvertScores)
			{
				ItemInfo->Score = 1.f - (ItemInfo->Score - MinScore) / ScoreRange;
			}
			else
			{
				ItemInfo->Score = (ItemInfo->Score - MinScore) / ScoreRange;
			}
		}
	}
}

void FRogueQueryInstance::SortScores()
{
	Items.Sort(TGreater<FRogueQueryItem>());
}

void FRogueQueryInstance::FinishExecution()
{
	
}

void FRogueQueryInstance::FinalizeTest()
{
	// if it's not the last and final test
	if (IsInSingleItemFinalSearch() == false)
	{
		// do regular normalization
		URogueQueryTest* TestOb = Tests[CurrentTest];
		if(Items.Num() > 0)
		{
			TestOb->NormalizeItemScores(*this);
		}
	}
	else
	{
		ItemDetails.Reset();
	}
}

void FRogueQueryInstance::FinalizeQuery()
{
	if (NumValidItems > 0)
	{
		if (Query->Mode == ERogueQueryRunMode::SingleResult)
		{
			// if last test was not pure condition: sort and pick one of best items
			if (bFoundSingleResult == false && bPassOnSingleResult == false)
			{
				SortScores();
				PickSingleItem(0);
			}
		}
		else if (Query->Mode == ERogueQueryRunMode::RandomBest5Pct || Query->Mode == ERogueQueryRunMode::RandomBest25Pct)
		{
			SortScores();
			const float ScoreRangePct = (Query->Mode == ERogueQueryRunMode::RandomBest5Pct) ? 0.95f : 0.75f;
			PickRandomItemOfScoreAtLeast(Items[0].Score * ScoreRangePct);
		}
		else
		{
			SortScores();

			// remove failed ones from Items
			Items.SetNum(NumValidItems);

			// normalizing after scoring and reducing number of elements to not 
			// do anything for discarded items
			NormalizeScores();
		}

		FilterResults();
		MarkAsFinishedWithoutIssues();
	}
	else
	{
		Items.Reset();
		MarkAsFailed();
	}
}

void FRogueQueryInstance::PickRandomItemOfScoreAtLeast(float MinScore)
{
	// find first valid item with score worse than best range
	int32 NumBestItems = NumValidItems;
	for (int32 ItemIndex = 1; ItemIndex < NumValidItems; ItemIndex++)
	{
		if (Items[ItemIndex].Score < MinScore)
		{
			NumBestItems = ItemIndex;
			break;
		}
	}

	// pick only one, discard others
	PickSingleItem(UAISystem::GetRandomStream().RandHelper(NumBestItems));
}

void FRogueQueryInstance::FilterResults()
{
	if (NumValidItems > 0)
	{
		// Remove values outside of result range
		for (int i = 0; i < Items.Num(); ++i)
		{
			if(Items[i].Score < ResultFilter.RangeMin)
			{
				Items[i].Discard();
			}
			if(Items[i].Score > ResultFilter.RangeMax)
			{
				Items[i].Discard();
			}
		}
		
		// Reduce items to result count if using
		if(ResultFilter.bUseResultCount && Items.Num() > ResultFilter.ResultCount - 1)
		{
			for (int j = 0; j < Items.Num(); ++j)
			//for (int j = Items.Num() - 1; j >= 0; --j)
			{
				if(j > ResultFilter.ResultCount - 1)
				{
					Items[j].Discard();
				}
			}
		}

		for (int k = Items.Num() - 1; k >= 0; --k)
		{
			if(Items[k].bIsDiscarded)
			{
				Items.RemoveAt(k);
			}
		}
	}
}

FRogueQueryInstance::FItemIterator::FItemIterator(const URogueQueryTest* QueryTest, FRogueQueryInstance& QueryInstance,
int32 StartingItemIndex) : FConstItemIterator(QueryInstance, StartingItemIndex)
{
	check(QueryTest);

	CachedFilterOp = QueryTest->MultipleContextFilterOp.GetIntValue();
	CachedScoreOp = QueryTest->MultipleContextScoreOp.GetIntValue();
	bIsFiltering = (QueryTest->TestPurpose == ERogueTestPurpose::Filter) || (QueryTest->TestPurpose == ERogueTestPurpose::FilterAndScore);

	Deadline = FLT_MAX;
	InitItemScore();
}

void FRogueQueryInstance::FItemIterator::HandleFailedTestResult()
{
	ItemScore = -1.f;
	Instance.Items[CurrentItem].Discard();
	Instance.NumValidItems--;
}

void FRogueQueryInstance::FItemIterator::StoreTestResult()
{
	CheckItemPassed();

	if (Instance.IsInSingleItemFinalSearch())
	{
		// handle SingleResult mode
		// this also implies we're not in 'score-only' mode
		if (bPassed)
		{
			if (bForced)
			{
				// store item value in case it's using special "skipped" constant
				Instance.ItemDetails[CurrentItem].TestResults[Instance.CurrentTest] = ItemScore;
			}

			Instance.PickSingleItem(CurrentItem);
			Instance.bFoundSingleResult = true;
		}
		else if (!bPassed)
		{
			HandleFailedTestResult();
		}
	}
	else
	{
		if (!bPassed && bIsFiltering)
		{
			HandleFailedTestResult();
		}
		else if (CachedScoreOp == ERogueTestScoreOperator::AverageScore && !bForced)
		{
			if (NumPassedForItem != 0)
			{
				ItemScore /= NumPassedForItem;
			}
			else
			{
				ItemScore = 0;
			}
		}

		Instance.ItemDetails[CurrentItem].TestResults[Instance.CurrentTest] = ItemScore;
	}
}

void FRogueQueryInstance::FItemIterator::SetScoreInternal(float Score)
{
	switch (CachedScoreOp)
	{
		case ERogueTestScoreOperator::AverageScore:
			ItemScore += Score;
			break;

		case ERogueTestScoreOperator::MinScore:
			if (!NumPassedForItem || ItemScore > Score)
			{
				ItemScore = Score;
			}
			break;

		case ERogueTestScoreOperator::MaxScore:
			if (!NumPassedForItem || ItemScore < Score)
			{
				ItemScore = Score;
			}
			break;
		case ERogueTestScoreOperator::Multiply:
			// ItemScore defaults to 0, so for first test we need to initialize the score, otherwise we end up constantly multiplying by 0
				ItemScore = (NumTestsForItem == 0) ? Score : (ItemScore * Score);
		break;
	}
}

void FRogueQueryInstance::FItemIterator::CheckItemPassed()
{
	if (!bForced)
	{
		if (NumTestsForItem == 0)
		{
			bPassed = false;
		}
		else if (!bIsFiltering)
		{
			bPassed = true;
		}
		else if (CachedFilterOp == ERogueTestFilterOperator::AllPass)
		{
			bPassed = bPassed && (NumPassedForItem == NumTestsForItem);
		}
		else
		{
			bPassed = bPassed && (NumPassedForItem > 0);
		}
	}
}

void FRogueQueryInstance::PickSingleItem(int32 ItemIndex)
{
	check(Items.Num() > 0);

	if (Items.IsValidIndex(ItemIndex) == false)
	{
		UE_LOG(LogRogueQuery, Warning
			, TEXT("Query [%s] tried to pick item %d as best item, but this index is out of scope (num items: %d). Falling back to item 0.")
			, *Query->QueryName.ToString(), ItemIndex, Items.Num());
		ItemIndex = 0;
	}

	FRogueQueryItem BestItem;
	// Copy the score from the actual item rather than just putting "1".  That way, it will correctly show cases where
	// the final filtering test was skipped by an item (and therefore not failed, i.e. passed).
	BestItem.Score = Items[ItemIndex].Score;

	Items.Empty(1);
	Items.Add(BestItem);
	NumValidItems = 1;
}


void FRogueQueryInstance::InitItems(TArray<FNavLocation>& Generated)
{
	GeneratedPoints = Generated;
	for (int i = 0; i < Generated.Num(); ++i)
	{
		Items.Add(FRogueQueryItem(Generated[i].Location));
	}

	NumValidItems = Generated.Num();
	ItemDetails.Reset();
	bFoundSingleResult = false;
}
