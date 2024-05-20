// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Behaviour/Tasks/RTSAi_BTTask_FocusTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Framework/Ai/RTSAi_ControllerBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

URTSAi_BTTask_FocusTarget::URTSAi_BTTask_FocusTarget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bNotifyTick = false;
	bNotifyTaskFinished = false;
	bCreateNodeInstance = false;
	NodeName = TEXT("Focus Target");
}

EBTNodeResult::Type URTSAi_BTTask_FocusTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(ARTSAi_ControllerBase* AiController = Cast<ARTSAi_ControllerBase>(OwnerComp.GetAIOwner()))
	{		
		if(OwnerComp.GetBlackboardComponent())
		{
			// Get blackboard key
			const FBlackboard::FKey Key = OwnerComp.GetBlackboardComponent()->GetKeyID(GetSelectedBlackboardKey());

			// check key is a of vector type
			if (OwnerComp.GetBlackboardComponent()->IsKeyOfType<UBlackboardKeyType_Vector>(Key))
			{
				const FVector Location = OwnerComp.GetBlackboardComponent()->GetValueAsVector(GetSelectedBlackboardKey());
				if(Location != FVector::ZeroVector)
				{
					if(const ACharacter* Character = Cast<ACharacter>(AiController->GetCharacter()))
					{
						if(UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>(Character->GetCharacterMovement()))
						{
							MovementComponent->bOrientRotationToMovement = false;
							MovementComponent->bUseControllerDesiredRotation = true;
							AiController->SetFocalPoint(Location);
							return EBTNodeResult::Succeeded;
						}
					}
				}
			}
			
			if (OwnerComp.GetBlackboardComponent()->IsKeyOfType<UBlackboardKeyType_Object>(Key))
			{
				if(AActor* FocusActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(GetSelectedBlackboardKey())))
				{
					if(const ACharacter* Character = Cast<ACharacter>(AiController->GetCharacter()))
					{
						if(UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>(Character->GetCharacterMovement()))
						{
							MovementComponent->bOrientRotationToMovement = false;
							MovementComponent->bUseControllerDesiredRotation = true;
							AiController->SetFocus(FocusActor);
							return EBTNodeResult::Succeeded;
						}
					}
				}
			}			
		}	
	}

	return EBTNodeResult::Failed;	
}