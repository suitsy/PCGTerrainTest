// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "RTSAi_BTTask_MoveToCover.generated.h"

UCLASS(config=Game, MinimalAPI)
class URTSAi_BTTask_MoveToCover : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	explicit URTSAi_BTTask_MoveToCover(const FObjectInitializer& ObjectInitializer);
	virtual EBTNodeResult::Type PerformMoveTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
