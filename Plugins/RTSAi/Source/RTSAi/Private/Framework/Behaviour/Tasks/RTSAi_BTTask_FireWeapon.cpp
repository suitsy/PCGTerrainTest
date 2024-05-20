// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Behaviour/Tasks/RTSAi_BTTask_FireWeapon.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Framework/Ai/RTSAi_ControllerBase.h"
#include "Framework/Interfaces/RTSCore_EntityEquipmentInterface.h"
#include "Framework/Interfaces/RTSCore_EntityInterface.h"

URTSAi_BTTask_FireWeapon::URTSAi_BTTask_FireWeapon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = true;
	NodeName = TEXT("Fire Weapon");
}

EBTNodeResult::Type URTSAi_BTTask_FireWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(IRTSCore_EntityEquipmentInterface* AiControllerEquipment = Cast<IRTSCore_EntityEquipmentInterface>(OwnerComp.GetAIOwner()))
	{
		if(AiControllerEquipment->GetCurrentWeaponAmmo() > 0)
		{
			if (CheckFiringCone(OwnerComp))
			{
				AiControllerEquipment->FireWeapon();
				return EBTNodeResult::Succeeded;
			}
		}
		else
		{
			AiControllerEquipment->RequestReload();
		}
	}
     
	return EBTNodeResult::Failed;
}

bool URTSAi_BTTask_FireWeapon::CheckFiringCone(const UBehaviorTreeComponent& OwnerComp) const
{
	if(OwnerComp.GetBlackboardComponent() && OwnerComp.GetAIOwner() != nullptr)
	{
		if(const IRTSCore_EntityEquipmentInterface* EntityEquipmentInterface = Cast<IRTSCore_EntityEquipmentInterface>(OwnerComp.GetAIOwner()))
		{			
			const FVector TargetLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(GetSelectedBlackboardKey());
	
			// Get the actor's eyes viewpoint
			FVector ViewPoint;
			FRotator ViewRotation;
			OwnerComp.GetAIOwner()->GetActorEyesViewPoint(ViewPoint, ViewRotation);

			// Calculate the vector from the actor's eyes to the point
			const FVector ObserverToTarget = TargetLocation - ViewPoint;

			// Normalize the direction vector
			const FVector ObserverDirection = ViewRotation.Vector().GetSafeNormal();

			// Calculate the angle between the observer direction and the vector to the target
			const float Angle = FMath::Acos(FVector::DotProduct(ObserverDirection, ObserverToTarget.GetSafeNormal()));

			// Convert the field of view to radians
			if(EntityEquipmentInterface->GetCurrentWeaponAccuracy() < 0) return false;
			const float HalfFieldOfViewRadians = FMath::DegreesToRadians(EntityEquipmentInterface->GetCurrentWeaponAccuracy());

			// Check if the angle is within the cone
			if (FMath::Abs(Angle) <= HalfFieldOfViewRadians)
			{
				return true;
			}
		}		
	}

	// Point is not within the cone
	return false;
}
