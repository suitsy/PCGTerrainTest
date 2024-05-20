// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Behaviour/Decorators/RTSAi_BTDecorator_HasCoverLocation.h"
#include "BehaviorTree/BlackboardComponent.h"

URTSAi_BTDecorator_HasCoverLocation::URTSAi_BTDecorator_HasCoverLocation()
{
	NodeName = TEXT("Has Cover Position");
}

bool URTSAi_BTDecorator_HasCoverLocation::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	const bool bResult =  Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	if(OwnerComp.GetBlackboardComponent())
	{
		return OwnerComp.GetBlackboardComponent()->GetValueAsObject(GetSelectedBlackboardKey()) != nullptr && bResult;
	}	

	return false;
}
