// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Framework/Data/RTSEntities_DataTypes.h"
#include "Framework/Interfaces/RTSCore_AiControllerInterface.h"
#include "Framework/Interfaces/RTSCore_AiStateInterface.h"
#include "RTSEntities_AiControllerBase.generated.h"

class URTSEntities_Entity;
class IRTSCore_TeamManagerInterface;
class URTSEntities_AiDataAsset;
class UAISenseConfig_Sight;

UCLASS()
class RTSENTITIES_API ARTSEntities_AiControllerBase : public AAIController, public IRTSCore_AiStateInterface, public IRTSCore_AiControllerInterface
{
	GENERATED_BODY()

public:
	explicit ARTSEntities_AiControllerBase(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void InitAiForGameplay();

	/** Entity Component **/
	bool HasEntityComponent();
	virtual ARTSEntities_Group* GetEntityGroup();

	UPROPERTY()
	URTSEntities_Entity* EntityComponent;
	/** End Entity Component **/

	/** Ai Data **/
public:
	virtual void SetAiData(const FPrimaryAssetId& InAiDataAssetId) { AiDataAssetId = InAiDataAssetId; }	
	virtual URTSEntities_AiDataAsset* GetAiData();

protected:	
	UPROPERTY()
	FPrimaryAssetId AiDataAssetId;
	
	/** End Ai Data **/

	// IRTSCore_AiControllerInterface
	virtual FCollisionShape GetCollisionShape() const override; 
	virtual ERTSCore_EntityType GetEntityType() override;	
	// End IRTSCore_AiControllerInterface

	
	/** Ai Behaviour **/
public:
	// IRTSCore_AiStateInterface
	virtual int32 GetState(const ERTSCore_StateCategory Category) const override;
	virtual void SetState(const ERTSCore_StateCategory Category, const int32 NewState) override;
	// End IRTSCore_AiStateInterface
protected:
	virtual void InitialiseAiBehaviour();	
	virtual void SetDefaultState();
	bool IsActive() const { return GetState(ERTSCore_StateCategory::Active) == static_cast<int32>(ERTSCore_ActiveState::Active); }	

	UPROPERTY()
	UBehaviorTree* BehaviourTreeAsset;	
	/** End Ai Behaviour **/


	/** Ai Perception **/
public:
	float GetLastSeenTargetTime() const { return LastSeenTargetTime; }
	virtual bool HasLineOfSight(const FVector& Location) const; 
	
protected:
	virtual void InitialisePerceptionSight();	
	virtual void StartUpdatingPerception();
	virtual void StopUpdatingPerception();
	virtual void ReportHostiles(const TArray<AActor*>& Hostiles);

	UFUNCTION()
	void UpdatePerceivedActors(const TArray<AActor*>& UpdatedActors);

	UFUNCTION()
	void UpdatePerception();	

	UPROPERTY()
	UAISenseConfig_Sight* SightConfig;
		
	UPROPERTY()
	TMap<AActor*, float> PerceivedEntities;

	UPROPERTY()
	FTimerHandle Handle_UpdatingPerception;

	UPROPERTY(Replicated)
	float LastSeenTargetTime;


		
private:
#if WITH_EDITOR
	void DebugTargets(const AActor* Other) const;
#endif
	
	/** End Ai Perception **/
};