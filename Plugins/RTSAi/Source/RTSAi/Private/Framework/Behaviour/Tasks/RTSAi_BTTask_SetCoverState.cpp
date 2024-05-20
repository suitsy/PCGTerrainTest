// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Behaviour/Tasks/RTSAi_BTTask_SetCoverState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/RTSAi_StaticGameData.h"
#include "Framework/Ai/RTSAi_ControllerBase.h"

URTSAi_BTTask_SetCoverState::URTSAi_BTTask_SetCoverState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bNotifyTick = false;
	bNotifyTaskFinished = false;
	bCreateNodeInstance = false;
	NodeName = TEXT("Set Cover State");
}

EBTNodeResult::Type URTSAi_BTTask_SetCoverState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(const ARTSAi_ControllerBase* AiController = Cast<ARTSAi_ControllerBase>(OwnerComp.GetAIOwner()))
	{
		if(OwnerComp.GetBlackboardComponent() && AiController->GetPawn())
		{
			const FVector Location = OwnerComp.GetBlackboardComponent()->GetValueAsVector(GetSelectedBlackboardKey());
			const float DistanceToCover = (AiController->GetPawn()->GetActorLocation() - Location).Length();
			if(Location != FVector::ZeroVector)
			{
				if(DistanceToCover < 120.f)
				{
					OwnerComp.GetBlackboardComponent()->SetValueAsEnum(RTS_DATA_AI_BLACKBOARD_VALUE_COVERSTATE, static_cast<uint8>(ERTSAi_CoverState::Full));
					return EBTNodeResult::Succeeded;
				}
			}

			OwnerComp.GetBlackboardComponent()->SetValueAsEnum(RTS_DATA_AI_BLACKBOARD_VALUE_COVERSTATE, static_cast<uint8>(ERTSAi_CoverState::NoCover));
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;	
}
