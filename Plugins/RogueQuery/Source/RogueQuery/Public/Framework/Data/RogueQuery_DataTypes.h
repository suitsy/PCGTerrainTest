// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Navigation/NavigationTypes.h"
#include "RogueQuery_DataTypes.generated.h"

class URogueQueryGenerator;
class URogueQueryTest;
class ANavigationData;
class UNavigationQueryFilter;
class URogueQuery;

DECLARE_LOG_CATEGORY_EXTERN(LogRogueQuery, Log, All);

UENUM()
enum class ERogueQueryTrace : uint8
{
	None,
	Navigation,
	GeometryByChannel,
	GeometryByProfile,
	NavigationOverLedges
};

UENUM()
namespace ERogueQueryTraceShape
{
	enum Type : int
	{
		Line,
		Box,
		Sphere,
		Capsule,
	};
}

UENUM()
namespace ERogueQueryOverlapShape
{
	enum Type : int
	{
		Box,
		Sphere,
		Capsule,
	};
}

namespace ERogueItemStatus
{
	enum Type
	{
		Passed,
		Failed,
	};
}

UENUM(BlueprintType)
namespace ERogueQueryStatus
{
	enum Type : int
	{
		Queued,
		Processing,
		Success,
		Failed,
		Aborted,
		OwnerLost,
		MissingParam,
	};
}

UENUM()
namespace ERogueQueryRunMode
{
	enum Type : int
	{
		SingleResult	UMETA(Tooltip="Pick first item with the best score", DisplayName="Single Best Item"),
		RandomBest5Pct	UMETA(Tooltip="Pick random item with score 95% .. 100% of max", DisplayName="Single Random Item from Best 5%"),
		RandomBest25Pct	UMETA(Tooltip="Pick random item with score 75% .. 100% of max", DisplayName="Single Random Item from Best 25%"),
		AllMatching		UMETA(Tooltip="Get all items that match conditions"),
	};
}

UENUM()
namespace ERogueTestPurpose
{
	enum Type : int
	{
		Filter UMETA(DisplayName="Filter Only"),
		Score UMETA(DisplayName="Score Only"),
		FilterAndScore UMETA(DisplayName="Filter and Score")
	};
}

UENUM()
namespace ERogueTestScoreEquation
{
	enum Type : int
	{
		Linear,
		Square,
		InverseLinear,
		SquareRoot,
		Constant
	};
}

UENUM()
namespace ERogueTestScoreNormalization
{
	enum Type : uint8
	{
		// Use 0 as the base of normalization range.
		Absolute,
		// Use lowest item score as the base of normalization range.
		RelativeToScores
	};
}

UENUM()
namespace ERogueTestScoreOperator
{
	enum Type : int
	{
		AverageScore	UMETA(Tooltip = "Use average score from all contexts"),
		MinScore		UMETA(Tooltip = "Use minimum score from all contexts"),
		MaxScore		UMETA(Tooltip = "Use maximum score from all contexts"),
		Multiply		UMETA(Tooltip = "Multiply scores from all contexts"),
	};
}

UENUM()
namespace ERogueTestFilterOperator
{
	enum Type : int
	{
		AllPass			UMETA(Tooltip = "All contexts must pass condition"),
		AnyPass			UMETA(Tooltip = "At least one context must pass condition"),
	};
}

UENUM()
namespace ERogueQueryTestClamping
{
	enum Type : int
	{
		None,			
		/** Clamp to value specified in test. */
		SpecifiedValue,
		/** Clamp to test's filter threshold. */
		FilterThreshold
	};
}

UENUM()
namespace ERogueTestFilterType
{
	enum Type : int
	{
		/** For numeric tests. */
		Minimum,
		/** For numeric tests. */
		Maximum,
		/** For numeric tests. */
		Range,
		/** For Boolean tests. */
		Match
	};
}

USTRUCT()
struct FRogueQueryTraceData
{
	GENERATED_USTRUCT_BODY()

	FRogueQueryTraceData() :
		VersionNum(0), ProjectDown(1024.0f), ProjectUp(1024.0f), ExtentX(10.0f), ExtentY(10.0f), ExtentZ(10.0f),
		PostProjectionVerticalOffset(0.0f),	TraceChannel(TraceTypeQuery1), SerializedChannel(ECC_WorldStatic), TraceProfileName(NAME_None),
		TraceShape(ERogueQueryTraceShape::Line), TraceMode(ERogueQueryTrace::None),
		bTraceComplex(false), bOnlyBlockingHits(true),
		bCanTraceOnNavMesh(true), bCanTraceOnGeometry(true), bCanDisableTrace(true), bCanProjectDown(false)
	{
	}
	
	/** version number for updates */
	UPROPERTY()
	int32 VersionNum;

	/** navigation filter for tracing */
	UPROPERTY(EditDefaultsOnly, Category=Trace)
	TSubclassOf<UNavigationQueryFilter> NavigationFilter;
	
	/** search height: below point */
	UPROPERTY(EditDefaultsOnly, Category=Trace, meta=(UIMin=0, ClampMin=0))
	float ProjectDown;

	/** search height: above point */
	UPROPERTY(EditDefaultsOnly, Category=Trace, meta=(UIMin=0, ClampMin=0))
	float ProjectUp;

	/** shape parameter for trace */
	UPROPERTY(EditDefaultsOnly, Category=Trace, meta=(UIMin=0, ClampMin=0))
	float ExtentX;

	/** shape parameter for trace */
	UPROPERTY(EditDefaultsOnly, Category=Trace, meta=(UIMin=0, ClampMin=0))
	float ExtentY;

	/** shape parameter for trace */
	UPROPERTY(EditDefaultsOnly, Category=Trace, meta=(UIMin=0, ClampMin=0))
	float ExtentZ;

	/** this value will be added to resulting location's Z axis. Can be useful when 
	 *	projecting points to navigation since navmesh is just an approximation of level 
	 *	geometry and items may end up being under collide-able geometry which would 
	 *	for example falsify visibility tests.*/
	UPROPERTY(EditDefaultsOnly, Category=Trace)
	float PostProjectionVerticalOffset;

	/** geometry trace channel */
	UPROPERTY(EditDefaultsOnly, Category=Trace)
	TEnumAsByte<enum ETraceTypeQuery> TraceChannel;

	/** geometry trace channel for serialization purposes */
	UPROPERTY(EditDefaultsOnly, Category=Trace)
	TEnumAsByte<enum ECollisionChannel> SerializedChannel;

	/** geometry trace profile */
	UPROPERTY(EditDefaultsOnly, Category = Trace)
	FName TraceProfileName;

	/** shape used for geometry tracing */
	UPROPERTY(EditDefaultsOnly, Category=Trace)
	TEnumAsByte<ERogueQueryTraceShape::Type> TraceShape;

	/** shape used for geometry tracing */
	UPROPERTY(EditDefaultsOnly, Category=Trace)
	ERogueQueryTrace TraceMode;

	/** if set, trace will run on complex collisions */
	UPROPERTY(EditDefaultsOnly, Category=Trace, AdvancedDisplay)
	uint32 bTraceComplex : 1;

	/** if set, trace will look only for blocking hits */
	UPROPERTY(EditDefaultsOnly, Category=Trace, AdvancedDisplay)
	uint32 bOnlyBlockingHits : 1;

	/** if set, editor will allow picking navmesh trace */
	UPROPERTY(EditDefaultsOnly, Category=Trace)
	uint32 bCanTraceOnNavMesh : 1;

	/** if set, editor will allow picking geometry trace */
	UPROPERTY(EditDefaultsOnly, Category=Trace)
	uint32 bCanTraceOnGeometry : 1;

	/** if set, editor will allow  */
	UPROPERTY(EditDefaultsOnly, Category=Trace)
	uint32 bCanDisableTrace : 1;

	/** if set, editor show height up/down properties for projection */
	UPROPERTY(EditDefaultsOnly, Category=Trace)
	uint32 bCanProjectDown : 1;

	void SetGeometryOnly();
	void SetNavmeshOnly();	
	void OnPostLoad();
};



struct FRogueQueryItem
{
	FRogueQueryItem() : Score(0.0f), Location(FVector::ZeroVector), bIsDiscarded(false) {}
	FRogueQueryItem(const FVector& InLocation) : Score(0.0f), Location(InLocation), bIsDiscarded(false) {}
	FRogueQueryItem(const float InScore) : Score(InScore), Location(FVector::ZeroVector), bIsDiscarded(false) {}
	FRogueQueryItem(const float InScore, const FVector& InLocation) : Score(InScore), Location(InLocation), bIsDiscarded(false) {}
	
	/** total score of item */
	float Score;

	/** raw data offset */
	FVector Location;

	/** has this item been discarded? */
	uint32 bIsDiscarded:1;

	FORCEINLINE bool IsValid() const { return !Location.ContainsNaN() && !bIsDiscarded; }
	FORCEINLINE void Discard() { bIsDiscarded = true; }

	bool operator<(const FRogueQueryItem& Other) const
	{
		// sort by validity
		if (IsValid() != Other.IsValid())
		{
			// self not valid = less important
			return !IsValid();
		}
		
		// sort by score if not equal
		return Score < Other.Score;
	}	
};

/** Detailed information about item, used by tests */
struct FRogueQueryItemDetails
{
	/** Results assigned by option's tests, before any modifications */
	TArray<float> TestResults;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	/** Results assigned by option's tests, after applying modifiers, normalization and weight */
	TArray<float> TestWeightedScores;

	int32 FailedTestIndex;
	int32 ItemIndex;
	FString FailedDescription;
#endif 

	FRogueQueryItemDetails() {}
	FRogueQueryItemDetails(int32 NumTests, int32 InItemIndex)
	{
		TestResults.AddZeroed(NumTests);
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		TestWeightedScores.AddZeroed(NumTests);
		ItemIndex = InItemIndex;
		FailedTestIndex = INDEX_NONE;
#endif
	}

	FORCEINLINE uint32 GetAllocatedSize() const
	{
		return sizeof(*this) +
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			TestWeightedScores.GetAllocatedSize() +
#endif
			TestResults.GetAllocatedSize();
	}
};

USTRUCT(BlueprintType)
struct FRogueQueryResultFilter
{
	GENERATED_BODY()
	
public:
	FRogueQueryResultFilter(): bUseResultCount(0), ResultCount(1), RangeMin(0.f), RangeMax(1.f) {}
	FRogueQueryResultFilter(const bool bUseCount, const int32 Count): bUseResultCount(bUseCount), ResultCount(Count), RangeMin(0.f), RangeMax(1.f) {}
	FRogueQueryResultFilter(const float Min, const float Max): bUseResultCount(0), ResultCount(1), RangeMin(Min), RangeMax(Max) {}
	FRogueQueryResultFilter(const bool bUseCount, const int32 Count, const float Min, const float Max):
	bUseResultCount(bUseCount), ResultCount(Count), RangeMin(Min), RangeMax(Max) {}

	/** set when testing final condition of an option */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bUseResultCount : 1;
	
	/** number of positions desired for query result, items outside range will be dropped  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ResultCount;

	/** Normalised range minimum for returned scores, items outside range will be dropped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin=0.f, ClampMax=1.f))
	float RangeMin;

	/** Normalised range maximum for returned scores, items outside range will be dropped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin=0.f, ClampMax=1.f))
	float RangeMax;
};

USTRUCT(BlueprintType)
struct FRogueQueryResult
{
	GENERATED_BODY()

public:
	FRogueQueryResult() : QueryId(0), Status(ERogueQueryStatus::Queued) {}
	bool IsValid() const { return Items.Num() > 0; }
	/** instance ID */
	UPROPERTY(BlueprintReadOnly, Category = RogueQuery)
	int32 QueryId;

	TArray<FNavLocation> GeneratedPoints;
	TArray<FRogueQueryItem> Items;	

	FORCEINLINE bool IsProcessing() const { return Status > ERogueQueryStatus::Queued; }
	FORCEINLINE bool IsFinished() const { return Status > ERogueQueryStatus::Processing; }
	FORCEINLINE bool IsAborted() const { return Status == ERogueQueryStatus::Aborted; }
	FORCEINLINE bool IsSuccessful() const { return Status == ERogueQueryStatus::Success; }
	FORCEINLINE void MarkAsProcessing() { Status = ERogueQueryStatus::Processing; }
	FORCEINLINE void MarkAsMissingParam() { Status = ERogueQueryStatus::MissingParam; }
	FORCEINLINE void MarkAsAborted() { Status = ERogueQueryStatus::Aborted; }
	FORCEINLINE void MarkAsFailed() { Status = ERogueQueryStatus::Failed; }
	FORCEINLINE void MarkAsFinishedWithoutIssues() { Status = ERogueQueryStatus::Success; }
	FORCEINLINE void MarkAsOwnerLost() { Status = ERogueQueryStatus::OwnerLost; }

	// Debug
	FORCEINLINE bool ShowDebugGrid() const { return DebugGrid; }
	FORCEINLINE void SetDebugGridShown() { IsShowingDebugGrid = true; }
	FORCEINLINE bool IsDebugGridShown() const { return IsShowingDebugGrid; }

private:
	/** query status */
	ERogueQueryStatus::Type Status;

	/** show debug grid */
	uint8 DebugGrid:1 = true;

	/** show debug grid */
	uint8 IsShowingDebugGrid:1 = false;
};

USTRUCT(BlueprintType)
struct FRogueQueryCustomData
{
	GENERATED_BODY()

	FRogueQueryCustomData():
		IsEnabled(false),
		Offset(FVector(0.f, -1.f, 0.f)),
		SpaceBetween(200.f),
		NumPositions(0)
	{}

	FRogueQueryCustomData(const bool bUseCustomData):
		IsEnabled(bUseCustomData),
		Offset(FVector(0.f, -1.f, 0.f)),
		SpaceBetween(200.f),
		NumPositions(0)
	{}
	
	bool IsValid() const { return IsEnabled; }

	/** vector to calculate position offsets */
	UPROPERTY(EditDefaultsOnly, Category=CustomData, meta=(DisplayName="Formation Offset"))
	uint8 IsEnabled:1;

	/** vector to calculate position offsets */
	UPROPERTY(EditDefaultsOnly, Category=CustomData, meta=(DisplayName="Formation Offset"))
	FVector Offset;

	/** spacing between each position */
	UPROPERTY(EditDefaultsOnly, Category=CustomData)
	float SpaceBetween;

	/** number of positions required */
	UPROPERTY(EditDefaultsOnly, Category=CustomData)
	int32 NumPositions;
};

//DECLARE_DELEGATE_OneParam(FRogueQueryFinishedDelegate, TSharedPtr<FRogueQueryResult>);
//DECLARE_MULTICAST_DELEGATE_OneParam(FRogueQueryFinishedDelegate, TSharedPtr<FRogueQueryResult>);
DECLARE_DELEGATE_OneParam(FRogueQueryFinishedSignature, const struct FRogueQueryInstance&);

USTRUCT()
struct FRogueQueryInstance : public FRogueQueryResult
{
	GENERATED_BODY()

public:
	FRogueQueryInstance():
		World(nullptr),
		Query(nullptr),
		SourceLocation(FVector::ZeroVector),
		NumValidItems(0),
		DebugActor(nullptr), 
		CurrentTest(0),
		TestIndex(0),
		Generator(nullptr),
		bFoundSingleResult(0),
		bPassOnSingleResult(0),
		bInvertScores(0), 
		CurrentTestStartingItem(0)
	{}
	FRogueQueryInstance(const FRogueQueryFinishedSignature& QueryFinishDelegate):
		Owner(nullptr),
		World(nullptr),
		RogueQueryFinished(QueryFinishDelegate),
		Query(nullptr),
		SourceLocation(FVector::ZeroVector),
		NumValidItems(0),
		DebugActor(nullptr),
		CurrentTest(0),
		TestIndex(0),
		Generator(nullptr),
		bFoundSingleResult(0),
		bPassOnSingleResult(0),
		bInvertScores(0),
		CurrentTestStartingItem(0)
	{}
	bool IsValid() const { return Owner != nullptr && World != nullptr && RogueQueryFinished.IsBound(); }	

	/** Object responsible for this query instance. */
	UPROPERTY()
	TObjectPtr<UObject> Owner;
	
	/** world owning this query instance */
	UPROPERTY()
	UWorld* World;

	/** observer's delegate */
	FRogueQueryFinishedSignature RogueQueryFinished;

	/** query */
	UPROPERTY()
	URogueQuery* Query;

	UPROPERTY()
	FVector SourceLocation;

	/** number of valid items on list */
	UPROPERTY()
	int32 NumValidItems;

	/** actor reference when using debug actor */
	UPROPERTY()
	AActor* DebugActor;

	/** actor reference when using debug actor */
	UPROPERTY()
	FRogueQueryResultFilter ResultFilter;

	/** currently processed test */
	int32 CurrentTest;
	
	/** index of query option, that generated items */
	UPROPERTY(BlueprintReadOnly, Category = "EQS")
	int32 TestIndex;

	/** list of item details */
	TArray<FRogueQueryItemDetails> ItemDetails;

	/** custom data **/
	FRogueQueryCustomData CustomData;

	/** generator for source points */
	UPROPERTY()
	URogueQueryGenerator* Generator;

	/** list of tests */
	TArray<URogueQueryTest*> Tests;

	/** used to breaking from item iterator loops */
	uint8 bFoundSingleResult : 1;

	/** set when testing final condition of an option */
	uint8 bPassOnSingleResult : 1;

	/** invert the final normalised score - only when normalised */
	uint8 bInvertScores : 1;

	/** execute single step of query */
	ROGUEQUERY_API void Execute();

	/** generate query items */
	void GenerateItems();
	void InitItems(TArray<FNavLocation>& Generated);

	/** perform tests */
	void PerformTests();

	/** normalize total score in range 0..1 */
	void NormalizeScores();

	/** sort all scores, from highest to lowest */
	void SortScores();

	/** prepare item data after generator has finished */
	void FinishExecution();

	/** update costs and flags after test has finished */
	void FinalizeTest();
	
	/** final pass on items of finished query */
	void FinalizeQuery();

	/** pick one of items with score equal or higher than specified */
	void PickRandomItemOfScoreAtLeast(float MinScore);

	/** filter results based on filter request */
	void FilterResults();

	int32 CurrentTestStartingItem;

	bool operator==(const FRogueQueryInstance& Other) const
	{
		return QueryId == Other.QueryId;
	}

#if CPP || UE_BUILD_DOCS
	/** Note that this iterator is for read-only purposes. Please use FItemIterator for regular item iteration 
	 *	while performing EQS testing and scoring */
	struct FConstItemIterator
	{
		FConstItemIterator(FRogueQueryInstance& QueryInstance, int32 StartingItemIndex = INDEX_NONE)
			: Instance(QueryInstance)
			, CurrentItem(StartingItemIndex != INDEX_NONE ? StartingItemIndex : QueryInstance.CurrentTestStartingItem)
		{
			if (StartingItemIndex != INDEX_NONE)
			{
				CurrentItem = StartingItemIndex;
			}
			else
			{
				CurrentItem = QueryInstance.CurrentTestStartingItem;
				if (Instance.Items.IsValidIndex(CurrentItem) == false || Instance.Items[CurrentItem].IsValid() == false)
				{
					++(*this);
				}
			}
		}

		int32 GetIndex() const
		{
			return CurrentItem;
		}

		FORCEINLINE explicit operator bool() const
		{
			return CurrentItem < Instance.Items.Num() && !Instance.bFoundSingleResult;
		}

		void operator++()
		{
			++CurrentItem;
			for (; CurrentItem < Instance.Items.Num() && !Instance.Items[CurrentItem].IsValid(); ++CurrentItem)
				;
		}

	protected:

		FRogueQueryInstance& Instance;
		int32 CurrentItem;
	};

	struct FItemIterator : public FConstItemIterator
	{
		ROGUEQUERY_API FItemIterator(const URogueQueryTest* QueryTest, FRogueQueryInstance& QueryInstance, int32 StartingItemIndex = INDEX_NONE);

		~FItemIterator()
		{
			Instance.CurrentTestStartingItem = CurrentItem;
		}

		/** Filter and score an item - used by tests working on float values
		 *  (can be called multiple times for single item when processing contexts with multiple entries)
		 *  NOTE: The Score is the raw score, before clamping, normalizing, and multiplying by weight.  The FilterMin
		 *  and FilterMax values are ONLY used for filtering (if any).
		 */
		void SetScore(ERogueTestPurpose::Type TestPurpose, ERogueTestFilterType::Type FilterType, float Score, float FilterMin, float FilterMax)
		{
			if (bForced)
			{
				return;
			}

			bool bPassedTest = true;

			if (TestPurpose != ERogueTestPurpose::Score)	// May need to filter results!
			{
				switch (FilterType)
				{
					case ERogueTestFilterType::Maximum:
						bPassedTest = (Score <= FilterMax);
						if(!bPassedTest)
						{
							UE_LOG(LogRogueQuery, Log, TEXT("Value %f is above maximum value set to %f"), Score, FilterMax);
						}
						break;

					case ERogueTestFilterType::Minimum:
						bPassedTest = (Score >= FilterMin);
						if(!bPassedTest)
						{
							UE_LOG(LogRogueQuery, Log, TEXT("Value %f is below minimum value set to %f"), Score, FilterMin);
						}
						break;

					case ERogueTestFilterType::Range:
						bPassedTest = (Score >= FilterMin) && (Score <= FilterMax);
						if(!bPassedTest)
						{
							UE_LOG(LogRogueQuery, Log, TEXT("Value %f is out of range set to (%f, %f)"), Score, FilterMin, FilterMax);
						}
						break;

					case ERogueTestFilterType::Match:
						UE_LOG(LogRogueQuery, Error, TEXT("Filtering Type set to 'Match' for floating point test.  Will consider test as failed in all cases."));
						bPassedTest = false;
						break;

					default:
						UE_LOG(LogRogueQuery, Error, TEXT("Filtering Type set to invalid value for floating point test.  Will consider test as failed in all cases."));
						bPassedTest = false;
						break;
				}
			}

			if (bPassedTest)
			{
				SetScoreInternal(Score);
				NumPassedForItem++;
			}

			NumTestsForItem++;
		}

		/** Filter and score an item - used by tests working on bool values
		 *  (can be called multiple times for single item when processing contexts with multiple entries)
		 */
		void SetScore(ERogueTestPurpose::Type TestPurpose, ERogueTestFilterType::Type FilterType, bool bScore, bool bExpected)
		{
			if (bForced)
			{
				return;
			}

			bool bPassedTest = true;
			switch (FilterType)
			{
				case ERogueTestFilterType::Match:
					bPassedTest = (bScore == bExpected);
					if(!bPassedTest)
					{
						UE_LOG(LogRogueQuery, Log, TEXT("Boolean score doesn't match (expected %s and got %s)"), bExpected ? TEXT("TRUE") : TEXT("FALSE"), bScore ? TEXT("TRUE") : TEXT("FALSE"));
					}
					break;

				case ERogueTestFilterType::Maximum:
					UE_LOG(LogRogueQuery, Error, TEXT("Filtering Type set to 'Maximum' for boolean test.  Will consider test as failed in all cases."));
					bPassedTest = false;
					break;

				case ERogueTestFilterType::Minimum:
					UE_LOG(LogRogueQuery, Error, TEXT("Filtering Type set to 'Minimum' for boolean test.  Will consider test as failed in all cases."));
					bPassedTest = false;
					break;

				case ERogueTestFilterType::Range:
					UE_LOG(LogRogueQuery, Error, TEXT("Filtering Type set to 'Range' for boolean test.  Will consider test as failed in all cases."));
					bPassedTest = false;
					break;

				default:
					UE_LOG(LogRogueQuery, Error, TEXT("Filtering Type set to invalid value for boolean test.  Will consider test as failed in all cases."));
					bPassedTest = false;
					break;
			}

			if (bPassedTest || !bIsFiltering)
			{
				// even if the item's result is different than expected
				// but we're not filtering those items out, we still want
				// to treat this as successful test, just with different score
				SetScoreInternal(bPassedTest ? 1.0f : 0.f);
				NumPassedForItem++;
			}

			NumTestsForItem++;
		}

		/** Force state and score of item
		 *  Any following SetScore calls for current item will be ignored
		 */
		void ForceItemState(const ERogueItemStatus::Type InStatus, const float Score = -MAX_flt)
		{
			bForced = true;
			bPassed = (InStatus == ERogueItemStatus::Passed);
			ItemScore = Score;
		}

		/** Disables time slicing for this iterator, use with caution! */
		FItemIterator& IgnoreTimeLimit()
		{
			Deadline = -1.0f;
			return *this;
		}

		FORCEINLINE explicit operator bool() const
		{
			return CurrentItem < Instance.Items.Num() && !Instance.bFoundSingleResult && (Deadline < 0 || FPlatformTime::Seconds() < Deadline);
		}

		void operator++()
		{
			StoreTestResult();
			if (!Instance.bFoundSingleResult)
			{
				InitItemScore();
				FConstItemIterator::operator++();
			}
		}

	protected:

		double Deadline;
		float ItemScore;
		int16 NumPassedForItem;
		int16 NumTestsForItem;
		uint8 CachedFilterOp;
		uint8 CachedScoreOp;
		uint8 bPassed : 1;
		uint8 bForced : 1;
		uint8 bIsFiltering : 1;

		void InitItemScore()
		{
			NumPassedForItem = 0;
			NumTestsForItem = 0;
			ItemScore = 0.0f;
			bPassed = true;
			bForced = false;
		}

		void HandleFailedTestResult();
		void StoreTestResult();
		void SetScoreInternal(float Score);
		void CheckItemPassed();
	};
	typedef FItemIterator ItemIterator;
#endif

protected:
	bool IsInSingleItemFinalSearch() const { return !!bPassOnSingleResult; }
	void PickSingleItem(int32 ItemIndex);
};

USTRUCT()
struct FRogueQueryRequest
{
	GENERATED_BODY()
	
public:
	FRogueQueryRequest():
		Owner(nullptr),
		QueryData(nullptr),
		Location(FVector::ZeroVector),
		CustomData(FRogueQueryCustomData()),
		DebugActor(nullptr)
	{}
	
	FRogueQueryRequest(
		const TObjectPtr<UObject> InOwner,
		URogueQuery* Query,
		const FVector& InLocation,
		const FRogueQueryCustomData& InCustomData
	):
		Owner(InOwner),
		QueryData(Query),
		Location(InLocation),
		CustomData(InCustomData),
		DebugActor(nullptr)
	{}
	
	FRogueQueryRequest(
		URogueQuery* Query,
		const FVector& InLocation,
		const FRogueQueryCustomData& InCustomData
	):
		Owner(nullptr),
		QueryData(Query),
		Location(InLocation),
		CustomData(InCustomData),
		DebugActor(nullptr)
	{}
	
	FRogueQueryRequest(
		URogueQuery* Query,
		const FVector& InLocation,
		const FRogueQueryCustomData& InCustomData,
		AActor* InActor
	):
		Owner(nullptr),
		QueryData(Query),
		Location(InLocation),
		CustomData(InCustomData),
		DebugActor(InActor)
	{}
	
	FRogueQueryRequest(
		const TObjectPtr<UObject> InOwner,
		URogueQuery* Query,
		const FVector& InLocation,
		const FRogueQueryCustomData& InCustomData,
		AActor* InActor
	):
		Owner(InOwner),
		QueryData(Query),
		Location(InLocation),
		CustomData(InCustomData),
		DebugActor(InActor)
	{}
	
	FRogueQueryRequest(
		URogueQuery* Query,
		const FVector& InLocation,
		const FRogueQueryCustomData& InCustomData,
		AActor* InActor,
		const FRogueQueryResultFilter& NewResultFilter
	):
		Owner(nullptr),
		QueryData(Query),
		Location(InLocation),
		CustomData(InCustomData),
		DebugActor(InActor),
		ResultFilter(NewResultFilter)
	{}
	
	FRogueQueryRequest(
		const TObjectPtr<UObject> InOwner,
		URogueQuery* Query,
		const FVector& InLocation,
		const FRogueQueryCustomData& InCustomData,
		AActor* InActor,
		const FRogueQueryResultFilter& NewResultFilter
	):
		Owner(InOwner),
		QueryData(Query),
		Location(InLocation),
		CustomData(InCustomData),
		DebugActor(InActor),
		ResultFilter(NewResultFilter)
	{}
	
	bool IsValid() const { return QueryData != nullptr; }
	
	UPROPERTY()
	TObjectPtr<UObject> Owner;
	
	UPROPERTY()
	TObjectPtr<URogueQuery> QueryData;

	/** Location source of the query in the world */
	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRogueQueryCustomData CustomData;

	UPROPERTY()
	AActor* DebugActor;

	UPROPERTY()
	FRogueQueryResultFilter ResultFilter;
};
