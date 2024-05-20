// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Behaviour/Services/RTSAi_Service_CheckFireOnTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Framework/Interfaces/RTSCore_EntityEquipmentInterface.h"
#include "AIController.h"

URTSAi_Service_CheckFireOnTarget::URTSAi_Service_CheckFireOnTarget()
{
	NodeName = TEXT("Update Can Fire On Target");
}

void URTSAi_Service_CheckFireOnTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if(IRTSCore_EntityEquipmentInterface* EntityEquipmentInterface = Cast<IRTSCore_EntityEquipmentInterface>(OwnerComp.GetAIOwner()))
	{
		if(OwnerComp.GetBlackboardComponent())
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), EntityEquipmentInterface->CanEngageTarget());				
		}
	}
}
