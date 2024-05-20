// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Data/RTSAi_DataTypes.h"
#include "RTSAi_BTTask_FireWeapon.generated.h"

/**
 * 
 */
UCLASS()
class RTSAI_API URTSAi_BTTask_FireWeapon : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	explicit URTSAi_BTTask_FireWeapon(const FObjectInitializer& ObjectInitializer);
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	bool CheckFiringCone(const UBehaviorTreeComponent& OwnerComp) const;
};
