// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Behaviour/Tasks/RTSAi_BTTask_ReloadWeapon.h"
#include "Framework/Data/RTSCore_DataTypes.h"
#include "Framework/Interfaces/RTSCore_AiStateInterface.h"
#include "Framework/Interfaces/RTSCore_EntityEquipmentInterface.h"
#include "AIController.h"

URTSAi_BTTask_ReloadWeapon::URTSAi_BTTask_ReloadWeapon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = false;
	NodeName = TEXT("Reload Weapon");
}

EBTNodeResult::Type URTSAi_BTTask_ReloadWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{	
	if(IRTSCore_EntityEquipmentInterface* EquipmentInterface = Cast<IRTSCore_EntityEquipmentInterface>(OwnerComp.GetAIOwner()))
	{
		if (EquipmentInterface->HasInventoryAmmoForCurrentWeapon())
		{
			EquipmentInterface->ReloadWeapon();
			return EBTNodeResult::Succeeded;
		}
		else
		{
			if(IRTSCore_AiStateInterface* AiStateInterface = Cast<IRTSCore_AiStateInterface>(OwnerComp.GetAIOwner()))
			{
				AiStateInterface->SetState(ERTSCore_StateCategory::Ammunition, static_cast<int32>(ERTSCore_AmmoState::NoAmmo));
				return EBTNodeResult::Succeeded;
			}
		}
	}	
     
	return EBTNodeResult::Failed;
}
