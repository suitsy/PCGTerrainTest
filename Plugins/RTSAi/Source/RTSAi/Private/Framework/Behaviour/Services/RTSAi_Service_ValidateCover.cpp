// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Behaviour/Services/RTSAi_Service_ValidateCover.h"
#include "BehaviorTree/BlackboardComponent.h"

URTSAi_Service_ValidateCover::URTSAi_Service_ValidateCover()
{
	NodeName = TEXT("Validate Cover");
}

void URTSAi_Service_ValidateCover::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	/*if(ARTSAi_ControllerBase* AiController = Cast<ARTSAi_ControllerBase>(OwnerComp.GetAIOwner()))
	{
		if(OwnerComp.GetBlackboardComponent())
		{
			/*if(AiController->ValidateCover())
			{
				OwnerComp.GetBlackboardComponent()->SetValueAsEnum(GetSelectedBlackboardKey(), static_cast<uint8>(ERTSAi_CoverState::Full));
			}
			else
			{
				OwnerComp.GetBlackboardComponent()->SetValueAsEnum(GetSelectedBlackboardKey(), static_cast<uint8>(ERTSAi_CoverState::NoCover));
			}#1#
		}
	}	*/
}
