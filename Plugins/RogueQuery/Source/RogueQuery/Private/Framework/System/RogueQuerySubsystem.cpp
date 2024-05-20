// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/System/RogueQuerySubsystem.h"
#include "Framework/Data/RogueQuery.h"
#include "Framework/Generators/RogueQueryGenerator.h"
#include "Framework/Settings/RogueQuery_Settings.h"
#include "Framework/Tests/RogueQueryTest.h"

URogueQuerySubsystem::URogueQuerySubsystem(): NextQueryID(0)
{
}

void URogueQuerySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
}

void URogueQuerySubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ProcessQueryQueue();
}

TStatId URogueQuerySubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URogueQuerySubsystem, STATGROUP_Tickables);
}

void URogueQuerySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);	
}

void URogueQuerySubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void URogueQuerySubsystem::Reset()
{
	QueryQueue.Empty();
	QuerySlots = 0;
	NextQueryID = 0;
}

void URogueQuerySubsystem::Query(const FRogueQueryRequest& QueryRequest, FRogueQueryFinishedSignature const& FinishDelegate, const bool bShowGeneratorDebug)
{
	if(QueryRequest.IsValid() && FinishDelegate.IsBound())
	{
		UE_LOG(LogRogueQuery, Log, TEXT("[%s] Query Request Valid"), *GetClass()->GetName());

		const TSharedPtr<FRogueQueryInstance> QueryInstance = CreateQueryInstance(QueryRequest);		
		QueryInstance->QueryId = NextQueryID++;
		QueryRequest.Owner !=nullptr ? QueryInstance->Owner = QueryRequest.Owner: QueryInstance->Owner = FinishDelegate.GetUObject();
		QueryInstance->World = QueryInstance->Owner.Get()->GetWorld();
		QueryInstance->RogueQueryFinished = FinishDelegate;
		QueryInstance->SourceLocation = QueryRequest.Location;
		QueryInstance->CustomData = QueryRequest.CustomData;
		QueryInstance->DebugActor = QueryRequest.DebugActor;
		QueryInstance->ResultFilter = QueryRequest.ResultFilter;
		if(QueryInstance.IsValid())
		{
			QueryQueue.Add(QueryInstance);
			UE_LOG(LogRogueQuery, Log, TEXT("[%s] Query QueryInstance Valid - Query Queue [%d]"), *GetClass()->GetName(), QueryQueue.Num());

			ProcessQueryQueue();				
		}
	}
}

TSharedPtr<FRogueQueryInstance> URogueQuerySubsystem::CreateQueryInstance(const FRogueQueryRequest& QueryRequest)
{
	// Generate query instance from request
	FRogueQueryInstance QueryInstance = FRogueQueryInstance();
    		
	static const UEnum* RunModeEnum = StaticEnum<ERogueQueryRunMode::Type>();
	const FString NewInstanceName = RunModeEnum
		? FString::Printf(TEXT("%s_%s"), *QueryRequest.QueryData->GetFName().ToString(), *RunModeEnum->GetNameStringByValue(QueryRequest.QueryData->Mode))
		: FString::Printf(TEXT("%s_%d"), *QueryRequest.QueryData->GetFName().ToString(), uint8(QueryRequest.QueryData->Mode));
	URogueQuery* LocalQuery = (URogueQuery*)StaticDuplicateObject(QueryRequest.QueryData, this, *NewInstanceName);
	QueryInstance.Query = LocalQuery;

	if(QueryRequest.QueryData->Generator != nullptr)
	{
		URogueQueryGenerator* LocalGenerator = (URogueQueryGenerator*)StaticDuplicateObject(QueryRequest.QueryData->Generator, this);
		QueryInstance.Generator = LocalGenerator;
	}

	for (int32 TestIndex = 0; TestIndex < LocalQuery->Tests.Num(); ++TestIndex)
	{
		URogueQueryTest* Test = LocalQuery->Tests[TestIndex];
		if(Test == nullptr)
		{
			LocalQuery->Tests.RemoveAt(TestIndex, 1, false);
			--TestIndex;
			continue;
		}

		URogueQueryTest* LocalTest = (URogueQueryTest*)StaticDuplicateObject(Test, this);
		LocalQuery->Tests[TestIndex] = LocalTest;
	}

	QueryInstance.Tests = LocalQuery->Tests;
	

	if(QueryInstance.Tests.Num() == 0)
	{
		return nullptr;
	}
	
	// Create a shared pointer to the query instance
	//return MakeShared<FRogueQueryInstance>(QueryInstance);
	// create new instance
	TSharedPtr<FRogueQueryInstance> NewInstance(new FRogueQueryInstance(QueryInstance));
	return NewInstance;
}

void URogueQuerySubsystem::ProcessQueryQueue()
{
	// Check if there are available slots to run more queries
	int32 AvailableSlots = FMath::Max(0, QuerySlotMaxThreshold - QuerySlots);

#if WITH_EDITOR
	Debug_StartProcessQueue(AvailableSlots);
#endif
	
	// Loop through the query queue and run as many queries as there are available slots
	for (int32 i = 0; i < QueryQueue.Num() && AvailableSlots > 0; ++i)
	{
		// Retrieve the query instance from the queue and add to running queries
TSharedPtr<FRogueQueryInstance>& QueryInstancePtr = QueryQueue[i];
if(!QueryInstancePtr.Get()->IsProcessing())
{
	// Set query to processing state
	QueryInstancePtr.Get()->MarkAsProcessing();

	// Execute the query
	ExecuteQuery(QueryInstancePtr);

	// Decrement the available slots
	--AvailableSlots;
	QuerySlots++;
}
	}
}

void URogueQuerySubsystem::ExecuteQuery(TSharedPtr<FRogueQueryInstance>& QueryInstancePtr)
{
#if WITH_EDITOR
	Debug_ExecuteQuery(QueryInstancePtr);
#endif
	
	// Execute the query in a new async thread
URogueQuerySubsystem* ThisPtr = this;
AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ThisPtr, QueryInstancePtr]()
{
	//URogueQuerySubsystem* Subsystem = this;
	QueryInstancePtr.Get()->Execute();
	
	// After the query finishes executing, switch back to the game thread
	AsyncTask(ENamedThreads::GameThread, [ThisPtr, QueryInstancePtr]()
	{			
		// Notify that the query has finished
		ThisPtr->OnQueryFinished(QueryInstancePtr);
	});
});
}

void URogueQuerySubsystem::OnQueryFinished(const TSharedPtr<FRogueQueryInstance> QueryInstancePtr)
{
	/** Game Thread **/	

	// Remove the query instance from the RunningQueries array
	QueryQueue.Remove(QueryInstancePtr);

	// Decrement query slot
	QuerySlots--;
	QuerySlots = FMath::Clamp(QuerySlots, 0, QuerySlotMaxThreshold);

	// Notify owner query complete
	bool bStatus = false;
	if(QueryInstancePtr->Owner != nullptr)
	{
		bStatus = QueryInstancePtr->RogueQueryFinished.ExecuteIfBound(*QueryInstancePtr.Get());
	}

#if WITH_EDITOR
	Debug_QueryFinished(QueryInstancePtr, bStatus);
#endif
}

#if WITH_EDITOR
void URogueQuerySubsystem::Debug_StartProcessQueue(const int32 AvailableSlots) const
{
	if(const URogueQuery_Settings* Settings = GetDefault<URogueQuery_Settings>())
	{
		if(!Settings->DebugQuery || Settings->DisableTickDebug)
		{
			return;
		}
	
		UE_LOG(LogRogueQuery, Log, TEXT("[Rogue Query] Processing Queue Tick - Queries [%d] AvailableSlots [%d]"), QueryQueue.Num(), AvailableSlots);
	}	
}

void URogueQuerySubsystem::Debug_ExecuteQuery(const TSharedPtr<FRogueQueryInstance>& QueryInstancePtr) const
{
	if(const URogueQuery_Settings* Settings = GetDefault<URogueQuery_Settings>())
	{
		if(!Settings->DebugQuery)
		{
			return;
		}

		UE_LOG(LogRogueQuery, Log, TEXT("[Rogue Query] Execute Query [%s]"), *QueryInstancePtr->Query->QueryName.ToString());
	}	
}

void URogueQuerySubsystem::Debug_QueryFinished(const TSharedPtr<FRogueQueryInstance>& QueryInstancePtr, const bool Status) const
{
	if(const URogueQuery_Settings* Settings = GetDefault<URogueQuery_Settings>())
	{
		if(!Settings->DebugQuery)
		{
			return;
		}
	
		UE_LOG(LogRogueQuery, Log, TEXT("[Rogue Query] Execute Finished [%s][%s] Resulted with [%d] Items"), *QueryInstancePtr->Query->QueryName.ToString(), Status ? TEXT("Successfully") : TEXT("With Errors"), QueryInstancePtr->Items.Num());

		for (int i = 0; i < QueryInstancePtr->Items.Num(); ++i)
		{
			if(!QueryInstancePtr->Items[i].bIsDiscarded)
			{
				UE_LOG(LogRogueQuery, Log, TEXT("[Rogue Query] Result [%s][%f]"), *QueryInstancePtr->Items[i].Location.ToString(), QueryInstancePtr->Items[i].Score);
			
				if(Settings->DebugDrawQuery)
				{
					FLinearColor LinearColor = FMath::Lerp(FLinearColor::Red, FLinearColor::Green, QueryInstancePtr->Items[i].Score);
					DrawDebugSphere(GetWorld(), QueryInstancePtr->Items[i].Location, 25.f, 6, LinearColor.ToFColor(true), false, 7.f, 0, 2.f);
				}
			}
		}

		if(Settings->DebugDrawQueryGenerator)
		{
			for (int j = 0; j < QueryInstancePtr->GeneratedPoints.Num(); ++j)
			{
				DrawDebugSphere(GetWorld(), QueryInstancePtr->GeneratedPoints[j].Location, 35.f, 6, FColor::Silver, false, 7.f, 0, 2.f);
			}
		}
	}
}
#endif