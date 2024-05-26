// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Behaviour/Tasks/RTSAi_BTTask_SetSpeed.h"
#include "Framework/Ai/RTSAi_ControllerBase.h"

URTSAi_BTTask_SetSpeed::URTSAi_BTTask_SetSpeed(const FObjectInitializer& ObjectInitializer): Speed(ERTSAi_SpeedState::Run)
{
	bNotifyTick = false;
	bNotifyTaskFinished = false;
	bCreateNodeInstance = false;
	NodeName = TEXT("Set Speed");
}

EBTNodeResult::Type URTSAi_BTTask_SetSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(const ARTSAi_ControllerBase* AiController = Cast<ARTSAi_ControllerBase>(OwnerComp.GetAIOwner()))
	{
		
	}

	return EBTNodeResult::Failed;
}
