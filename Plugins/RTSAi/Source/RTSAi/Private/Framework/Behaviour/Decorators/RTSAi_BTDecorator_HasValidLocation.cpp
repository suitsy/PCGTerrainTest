// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Behaviour/Decorators/RTSAi_BTDecorator_HasValidLocation.h"
#include "BehaviorTree/BlackboardComponent.h"

URTSAi_BTDecorator_HasValidLocation::URTSAi_BTDecorator_HasValidLocation()
{
	NodeName = TEXT("Has Target");
}

bool URTSAi_BTDecorator_HasValidLocation::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const bool bResult = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	if(OwnerComp.GetBlackboardComponent())
	{
		return OwnerComp.GetBlackboardComponent()->GetValueAsVector(GetSelectedBlackboardKey()) != FVector::ZeroVector && bResult;
	}	

	return false;
}
