// Fill out your copyright notice in the Description page of Project Settings.


#include "StateMachine/MetaStates/Behaviour/RTSEntities_StateSafe.h"
#include "Framework/Entities/Group/RTSEntities_Group.h"
#include "Framework/Debug/RTSEntities_Debug.h"
#include "StateMachine/RTSEntities_GroupStateManager.h"


URTSEntities_StateSafe::URTSEntities_StateSafe()
{
}

void URTSEntities_StateSafe::Init(ARTSEntities_Group* InGroup)
{
	OwningGroup = InGroup;
	
	UE_LOG(LogRTSEntities, Warning, TEXT("[%s] Safe State Initialised"), *GetClass()->GetName());
}

void URTSEntities_StateSafe::OnEnter()
{
	if(OwningGroup)
	{
		OwningGroup->SetState(ERTSCore_StateCategory::Behaviour, static_cast<int32>(ERTSCore_BehaviourState::Safe));
		
		// Notify group safe
		if(OwningGroup->GetGroupStateManager())
		{
			OwningGroup->GetGroupStateManager()->PushGroupBehaviourStateChange(ERTSCore_BehaviourState::Safe);
		}
	}
}

void URTSEntities_StateSafe::OnUpdate()
{
	if(HasKnownTargets())
	{
		SendTrigger(ERTSEntities_BehaviourTriggers::ToCombat);
	}	
}

void URTSEntities_StateSafe::OnExit()
{
	UE_LOG(LogRTSEntities, Warning, TEXT("[%s] OnExit State Safe"), *GetClass()->GetName());
}
