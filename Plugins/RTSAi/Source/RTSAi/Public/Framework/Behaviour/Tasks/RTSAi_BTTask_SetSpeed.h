// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Data/RTSAi_DataTypes.h"
#include "RTSAi_BTTask_SetSpeed.generated.h"

/**
 * 
 */
UCLASS()
class RTSAI_API URTSAi_BTTask_SetSpeed : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	explicit URTSAi_BTTask_SetSpeed(const FObjectInitializer& ObjectInitializer);
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY()
	ERTSAi_SpeedState Speed;
};
