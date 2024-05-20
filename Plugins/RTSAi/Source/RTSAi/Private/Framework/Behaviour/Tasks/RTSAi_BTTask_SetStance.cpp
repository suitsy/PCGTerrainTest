// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Behaviour/Tasks/RTSAi_BTTask_SetStance.h"
#include "Framework/Ai/RTSAi_ControllerBase.h"
#include "Framework/Interfaces/RTSCore_AiInterface.h"
#include "GameFramework/Character.h"

URTSAi_BTTask_SetStance::URTSAi_BTTask_SetStance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bNotifyTick = false;
	bNotifyTaskFinished = false;
	bCreateNodeInstance = false;
	NodeName = TEXT("Clear Focus");
	Stance = ERTSAi_StanceState::Standing;
}

EBTNodeResult::Type URTSAi_BTTask_SetStance::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(const ARTSAi_ControllerBase* AiController = Cast<ARTSAi_ControllerBase>(OwnerComp.GetAIOwner()))
	{
		if(ACharacter* Character = Cast<ACharacter>(AiController->GetCharacter()))
		{
			if(IRTSCore_AiInterface* AiInterface = Cast<IRTSCore_AiInterface>(Character))
			{
				switch (Stance)
				{
					case ERTSAi_StanceState::Standing:
						AiInterface->Stand();
						return EBTNodeResult::Succeeded;
					case ERTSAi_StanceState::Crouch:
						AiInterface->Crouch();
						return EBTNodeResult::Succeeded;
					case ERTSAi_StanceState::Prone:				
						return EBTNodeResult::Succeeded;
				}
			}
		}
	}

	return EBTNodeResult::Failed;
}
