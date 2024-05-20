// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "RTSAi_Service_ValidateCover.generated.h"

/**
 * 
 */
UCLASS()
class RTSAI_API URTSAi_Service_ValidateCover : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	URTSAi_Service_ValidateCover();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
