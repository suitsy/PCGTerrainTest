// Fill out your copyright notice in the Description page of Project Settings.


#include "StateMachine/RTSEntities_GroupStateManager.h"
#include "Framework/Entities/Group/RTSEntities_Group.h"
#include "Framework/Ai/RTSAi_ControllerBase.h"
#include "Framework/AI/RTSEntities_AiControllerBase.h"
#include "Framework/Debug/RTSEntities_Debug.h"
#include "StateMachine/RTSEntities_StateMachine.h"

URTSEntities_GroupStateManager::URTSEntities_GroupStateManager()
{
	StateMachine = nullptr;
	OwningGroup = nullptr;
}

void URTSEntities_GroupStateManager::Init()
{
	// Set owning group
	OwningGroup = Cast<ARTSEntities_Group>(GetOuter());
	
	// Ensure we are not already managing a state machine
	ExitStateMachine();

	if(OwningGroup)
	{
		// Create state machine
		StateMachine = NewObject<URTSEntities_StateMachine>(this);
		if(StateMachine)
		{
			StateMachine->Init(OwningGroup);

			// Start state machine updates
			StartStateMachine();
		}
	}
	else
	{
		UE_LOG(LogRTSEntities, Error, TEXT("[%s] Failed to assing owning group to Group State Manager"), *GetClass()->GetName());
	}
}

void URTSEntities_GroupStateManager::StartStateMachine()
{	
	// Start a timer to update behaviour at interval
	if(GetWorld() && StateMachine != nullptr && !GetWorld()->GetTimerManager().TimerExists(Handle_StateMachineTimer))
	{
		StateMachine->EnterStateMachine();
		GetWorld()->GetTimerManager().SetTimer(Handle_StateMachineTimer, this, &ThisClass::UpdateStateMachine, 1.f, true, 1.f);		
	}
}

void URTSEntities_GroupStateManager::UpdateStateMachine() const
{
	if(StateMachine)
	{
		StateMachine->UpdateStateMachine();
	}
}

void URTSEntities_GroupStateManager::ExitStateMachine() const
{
	if(StateMachine)
	{
		StateMachine->ExitStateMachine();
	}
}

void URTSEntities_GroupStateManager::PushGroupBehaviourStateChange(const ERTSCore_BehaviourState NewState) const
{
	TArray<AActor*> Members;
	OwningGroup->GetEntities(Members);
	for (int i = 0; i < Members.Num(); ++i)
	{
		if(Members[i] == nullptr)
		{
			continue;
		}

		if(const APawn* AiPawn = Cast<APawn>(Members[i]))
		{
			if(ARTSEntities_AiControllerBase* AiController = Cast<ARTSEntities_AiControllerBase>(AiPawn->GetController()))
			{
				// If a member is not in the new state transition to the new state
				if(AiController->GetState(ERTSCore_StateCategory::Behaviour) != static_cast<int32>(NewState))
				{
					AiController->SetState(ERTSCore_StateCategory::Behaviour, static_cast<int32>(NewState));
				}
			}
		}
	}
}
