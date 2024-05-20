// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/RTSCore_DataTypes.h"
#include "UObject/Object.h"
#include "RTSEntities_GroupStateManager.generated.h"

class ARTSEntities_Group;
class URTSEntities_StateMachine;

/**
 * 
 */
UCLASS()
class RTSENTITIES_API URTSEntities_GroupStateManager : public UObject
{
	GENERATED_BODY()

public:
	URTSEntities_GroupStateManager();
	void Init();	
	void StartStateMachine();
	void ExitStateMachine() const;

	// State notifies
	void PushGroupBehaviourStateChange(const ERTSCore_BehaviourState NewState) const;		

protected:	
	UFUNCTION()
	void UpdateStateMachine() const;

	

	UPROPERTY()
	FTimerHandle Handle_StateMachineTimer;

	UPROPERTY()
	URTSEntities_StateMachine* StateMachine;

	UPROPERTY()
	ARTSEntities_Group* OwningGroup;
};
