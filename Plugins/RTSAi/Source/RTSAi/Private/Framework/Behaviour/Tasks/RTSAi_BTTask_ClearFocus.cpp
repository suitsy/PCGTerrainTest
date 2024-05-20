// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Behaviour/Tasks/RTSAi_BTTask_ClearFocus.h"
#include "Framework/Ai/RTSAi_ControllerBase.h"
#include "Framework/Interfaces/RTSCore_AiControllerInterface.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

URTSAi_BTTask_ClearFocus::URTSAi_BTTask_ClearFocus(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bNotifyTick = false;
	bNotifyTaskFinished = false;
	bCreateNodeInstance = false;
	NodeName = TEXT("Clear Focus");
}

EBTNodeResult::Type URTSAi_BTTask_ClearFocus::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(IRTSCore_AiControllerInterface* ControllerInterface = Cast<IRTSCore_AiControllerInterface>(OwnerComp.GetAIOwner()))
	{
		switch (ControllerInterface->GetEntityType())
		{
			case ERTSCore_EntityType::Character:
				if(const ACharacter* Character = Cast<ACharacter>(OwnerComp.GetAIOwner()->GetCharacter()))
				{
					if(UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>(Character->GetCharacterMovement()))
					{
						MovementComponent->bOrientRotationToMovement = true;
						MovementComponent->bUseControllerDesiredRotation = false;
						OwnerComp.GetAIOwner()->ClearFocus(EAIFocusPriority::Gameplay);
						return EBTNodeResult::Succeeded;
					}
				}		
				break;
			case ERTSCore_EntityType::Vehicle:
				break;
			default: ;
		}
		
	}
	
	return EBTNodeResult::Failed;
	
}