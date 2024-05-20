// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "RTSAi_BTTask_ReloadWeapon.generated.h"

/**
 * 
 */
UCLASS()
class RTSAI_API URTSAi_BTTask_ReloadWeapon : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	explicit URTSAi_BTTask_ReloadWeapon(const FObjectInitializer& ObjectInitializer);
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
