﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "RTSAi_BTDecorator_HasValidLocation.generated.h"

/**
 * 
 */
UCLASS()
class RTSAI_API URTSAi_BTDecorator_HasValidLocation : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()

	URTSAi_BTDecorator_HasValidLocation();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};