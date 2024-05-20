// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/RogueQuery_DataTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "RogueQuerySubsystem.generated.h"

class URogueQuery_Settings;
class ANavigationData;
/**
 * 
 */
UCLASS()
class ROGUEQUERY_API URogueQuerySubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	URogueQuerySubsystem();
	
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void Reset();
	virtual void Query(const FRogueQueryRequest& QueryRequest, FRogueQueryFinishedSignature const& FinishDelegate, const bool bShowGeneratorDebug = false);

protected:
	TSharedPtr<FRogueQueryInstance> CreateQueryInstance(const FRogueQueryRequest& QueryRequest); 
	void ProcessQueryQueue();
	void ExecuteQuery(TSharedPtr<FRogueQueryInstance>& QueryInstancePtr);
	void OnQueryFinished(const TSharedPtr<FRogueQueryInstance> QueryInstancePtr);
	
	/** queries received */
	TArray<TSharedPtr<FRogueQueryInstance>> QueryQueue;

	UPROPERTY()
	int32 QuerySlotMaxThreshold = 1;

	UPROPERTY()
	int32 QuerySlots = 0;

	/** next ID for running query */
	int32 NextQueryID;

	
#if WITH_EDITOR
	void Debug_StartProcessQueue(const int32 AvailableSlots) const;
	void Debug_ExecuteQuery(const TSharedPtr<FRogueQueryInstance>& QueryInstancePtr) const;
	void Debug_QueryFinished(const TSharedPtr<FRogueQueryInstance>& QueryInstancePtr, const bool Status) const;
#endif	
};
