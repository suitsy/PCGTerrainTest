// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Framework/Data/RTSEntities_DataTypes.h"
#include "RTSEntities_StateMachine.generated.h"

class URogueCover_Point;
struct FRTSAi_TargetData;
class URTSEntities_GroupStateManager;
class ARTSEntities_Group;

UENUM()
enum ERTSEntities_BehaviourTriggers
{
	ToSafe = 1,
	ToCautious = 2,
	ToCombat = 3
};

UCLASS()
class RTSENTITIES_API URTSEntities_StateMachine : public UObject
{
	GENERATED_BODY()

public:
	URTSEntities_StateMachine();
	virtual void Init(ARTSEntities_Group* InGroup = nullptr);
	virtual void EnterStateMachine();
	virtual URTSEntities_StateMachine* GetMetaState(const TSubclassOf<URTSEntities_StateMachine> Class);
	void UpdateStateMachine();
	void ExitStateMachine();
	void SendTrigger(const int32 Trigger);

	virtual float GetLastSeenEnemyTime() const;
	virtual bool HasKnownTargets() const;
	virtual void UpdateKnownTargets();
	virtual void SetEnemyLocation();
	virtual void AssignCombatTargets();
	virtual void AssignCoverPositions(TArray<URogueCover_Point*> Positions);
	virtual void TakeCover();
	
protected:
	virtual void OnEnter();
    virtual void OnUpdate();
    virtual void OnExit();

	void LoadMetaState(URTSEntities_StateMachine* MetaState);
	void AddTransition(URTSEntities_StateMachine* From, URTSEntities_StateMachine* To, int32 Trigger) const;	
	void ChangeMetaState(const URTSEntities_StateMachine* MetaState);

	void TargetSeenEnemy(TArray<AActor*>& MembersRequiringTargets) const;
	void TargetLastKnownLocation(TArray<AActor*>& MembersRequiringTargets) const;
	void UpdateNavigationState() const;

	bool IsNavigating() const;
	
	UPROPERTY()
	URTSEntities_StateMachine* CurrentMetaState;
	
	UPROPERTY()
	URTSEntities_StateMachine* DefaultMetaState;

	UPROPERTY()
	URTSEntities_StateMachine* Parent;

	UPROPERTY()
	TMap<TSubclassOf<URTSEntities_StateMachine>, URTSEntities_StateMachine*> MetaStates;

	UPROPERTY()
	TMap<int, URTSEntities_StateMachine*> Transitions;

	UPROPERTY()
	ARTSEntities_Group* OwningGroup;

	UPROPERTY()
	float StateTimeout = 0.f;

	UPROPERTY()
	bool bForced = false;

	UPROPERTY()
	FVector EnemyLocation = FVector::ZeroVector;
};

namespace ERTSEntities_EQSExecute
{
	template <class T, class FuncType>
	void ExecuteEnvironmentQuery(FEnvQueryRequest QueryRequest, EEnvQueryRunMode::Type RunMode,	T* Obj, FuncType ReturnFunc)
	{
		if(ReturnFunc != nullptr)
		{
			QueryRequest.Execute(RunMode, Obj, ReturnFunc);
		}
	}
}
