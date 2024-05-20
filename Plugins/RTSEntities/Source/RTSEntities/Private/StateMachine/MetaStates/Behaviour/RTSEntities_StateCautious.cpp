// Fill out your copyright notice in the Description page of Project Settings.


#include "StateMachine/MetaStates/Behaviour/RTSEntities_StateCautious.h"
#include "Framework/Entities/Group/RTSEntities_Group.h"
#include "Framework/Data/RTSEntities_GroupDataAsset.h"
#include "Framework/Debug/RTSEntities_Debug.h"
#include "StateMachine/RTSEntities_GroupStateManager.h"

URTSEntities_StateCautious::URTSEntities_StateCautious()
{
}

void URTSEntities_StateCautious::Init(ARTSEntities_Group* InGroup)
{
	OwningGroup = InGroup;

	if(OwningGroup)
	{
		if(const URTSEntities_GroupDataAsset* GroupData = OwningGroup->GetData())
		{
			StateTimeout = GroupData->CautiousTimeout;
		}
	}
	
	UE_LOG(LogRTSEntities, Warning, TEXT("[%s] Cautious State Initialised"), *GetClass()->GetName());
}

void URTSEntities_StateCautious::OnEnter()
{
	if(OwningGroup)
	{
		OwningGroup->SetState(ERTSCore_StateCategory::Behaviour, static_cast<int32>(ERTSCore_BehaviourState::Cautious));
		
		// Notify group safe
		if(OwningGroup->GetGroupStateManager())
		{
			OwningGroup->GetGroupStateManager()->PushGroupBehaviourStateChange(ERTSCore_BehaviourState::Cautious);
		}
	}
}

void URTSEntities_StateCautious::OnUpdate()
{
	Super::OnUpdate();

	// Check if any of the group is reporting combat state
	if(HasKnownTargets())
	{
		// Change to combat state if any group member is in combat
		SendTrigger(ERTSEntities_BehaviourTriggers::ToCombat);
	}
	else
	{
		// Check if the state has timed out
		if(const URTSEntities_GroupDataAsset* GroupData = OwningGroup->GetData())
		{
			// Current time greater than the last seen time + combat timeout then change state
			if(GetWorld()->GetTimeSeconds() > GetLastSeenEnemyTime() + GroupData->TargetRecallTime * 2.f)
			{
				SendTrigger(ERTSEntities_BehaviourTriggers::ToSafe);
			}
		}
	}
}

void URTSEntities_StateCautious::OnExit()
{
	Super::OnExit();
	
}
