// Fill out your copyright notice in the Description page of Project Settings.


#include "StateMachine/RTSEntities_StateMachine.h"
#include "Framework/Entities/Group/RTSEntities_Group.h"
#include "Framework/AI/RTSEntities_AiControllerBase.h"
#include "Framework/AI/RTSEntities_AiControllerEquipment.h"
#include "Framework/CoverSystem/RogueCover_Point.h"
#include "Framework/Data/RTSEntities_GroupDataAsset.h"
#include "Framework/Debug/RTSEntities_Debug.h"
#include "Framework/Interfaces/RTSCore_EntityEquipmentInterface.h"
#include "StateMachine/MetaStates/Behaviour/RTSEntities_StateCautious.h"
#include "StateMachine/MetaStates/Behaviour/Combat/RTSEntities_StateCombat.h"
#include "StateMachine/MetaStates/Behaviour/RTSEntities_StateSafe.h"

URTSEntities_StateMachine::URTSEntities_StateMachine()
{
	DefaultMetaState = nullptr;
	CurrentMetaState = nullptr;
	Parent = nullptr;
	OwningGroup = nullptr;
}

void URTSEntities_StateMachine::EnterStateMachine()
{
	// Enter parent state first
	OnEnter();

	// Assign current meta state to default meta state if no current meta state
	if (CurrentMetaState == nullptr && DefaultMetaState != nullptr)
	{
		CurrentMetaState = DefaultMetaState;
	}

	// Check current meta state is valid
	if (CurrentMetaState != nullptr)
	{
		// Enter meta state
		CurrentMetaState->EnterStateMachine();
	}
}

URTSEntities_StateMachine* URTSEntities_StateMachine::GetMetaState(const TSubclassOf<URTSEntities_StateMachine> Class)
{
	if (URTSEntities_StateMachine** StatePtr = MetaStates.Find(Class))
	{
		return *StatePtr;
	}

	return nullptr;
}

void URTSEntities_StateMachine::UpdateStateMachine()
{
	// Update parent state first
	OnUpdate();

	// Check current meta state is valid
	if (CurrentMetaState != nullptr)
	{
		// Perform meta state update
		CurrentMetaState->UpdateStateMachine();
	}	
}

void URTSEntities_StateMachine::ExitStateMachine()
{
	if (CurrentMetaState != nullptr)
	{
		CurrentMetaState->ExitStateMachine();
	}

	// Perform parent exit after meta state exit
	OnExit();
}

void URTSEntities_StateMachine::SendTrigger(const int32 Trigger)
{
	URTSEntities_StateMachine* Root = this;

	// Traverse up the hierarchy to find the root state
	while (Root && Root->Parent)
	{
		Root = Root->Parent;
	}

	while (Root)
	{
		// Check if the root state has a transition for the trigger
		if (URTSEntities_StateMachine** ToStatePtr = Root->Transitions.Find(Trigger))
		{
			if(const URTSEntities_StateMachine* ToState = *ToStatePtr)
			{
				// Change MetaState if the transition is found
				if (Root->Parent != nullptr)
				{
					Root->Parent->ChangeMetaState(ToState);
				}
			}
			return;
		}

		// Move to the current MetaState in the hierarchy
		Root = Root->CurrentMetaState;
	}
}

float URTSEntities_StateMachine::GetLastSeenEnemyTime() const
{
	float LastSeenTime = 0.f;
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
			if(const ARTSEntities_AiControllerBase* AiController = Cast<ARTSEntities_AiControllerBase>(AiPawn->GetController()))
			{
				if(AiController->GetLastSeenTargetTime() > LastSeenTime)
				{
					LastSeenTime = AiController->GetLastSeenTargetTime();
				}
			}
		}
	}

	return LastSeenTime;
}

bool URTSEntities_StateMachine::HasKnownTargets() const
{
	return OwningGroup && OwningGroup->KnownTargets.Num() > 0;
}

void URTSEntities_StateMachine::UpdateKnownTargets()
{
	if(!OwningGroup)
	{
		return;
	}

	// Flag known targets updating
	OwningGroup->SetKnownTargetsUpdating();
	
	// Update known targets list
	for (int k = OwningGroup->KnownTargets.Num() - 1; k >= 0; --k)
	{                    
		if(const APawn* TargetPawn = Cast<APawn>(OwningGroup->KnownTargets[k].TargetActor))
		{
			if(const IRTSCore_AiStateInterface* TargetStateInterface = Cast<IRTSCore_AiStateInterface>(TargetPawn->GetController()))
			{
				if(TargetStateInterface->GetState(ERTSCore_StateCategory::Condition) == static_cast<int32>(ERTSCore_ConditionState::Dead))
				{
					OwningGroup->KnownTargets.RemoveAt(k);
					continue;
				}
				else
				{
					// Check if Ai has not seen target for longer than timeout
					if(const URTSEntities_GroupDataAsset* GroupData = OwningGroup->GetData())
					{
						if(GetWorld())
						{						
							if(GetWorld()->GetTimeSeconds() > OwningGroup->KnownTargets[k].LastSeenEnemyTime + GroupData->TargetRecallTime)
							{
								OwningGroup->KnownTargets.RemoveAt(k);
								continue;
							}
						}
					}
				}
			}
		}
	}

	// Set the enemy location to the average known targets position
	SetEnemyLocation();

	// Flag known targets updating
	OwningGroup->SetKnownTargetsUpdating(false);
}

void URTSEntities_StateMachine::SetEnemyLocation()
{
	FVector CenterPosition = FVector::ZeroVector;
	if(OwningGroup->KnownTargets.Num() > 0)
	{
		for (int i = 0; i < OwningGroup->KnownTargets.Num(); ++i)
		{		
			CenterPosition += OwningGroup->KnownTargets[i].LastSeenEnemyLocation;
		}
		CenterPosition /= OwningGroup->KnownTargets.Num();
	}

	EnemyLocation = CenterPosition;
}

void URTSEntities_StateMachine::AssignCombatTargets()
{
	TArray<AActor*> Members;
	OwningGroup->GetEntities(Members);

	// Sort known targets by threat level
	if(OwningGroup->KnownTargets.Num() > 1)
	{
		OwningGroup->KnownTargets.Sort([](const FRTSEntities_TargetData& A, const FRTSEntities_TargetData& B)
		{
			return static_cast<int32>(A.ThreatLevel) > static_cast<int32>(B.ThreatLevel);
		});
	}

	// Get list of group members requiring targets
	TArray<AActor*> MembersRequiringTargets;
	for (int i = 0; i < Members.Num(); ++i)
	{
		if(Members[i] == nullptr)
		{
			continue;
		}

		if(const APawn* AiPawn = Cast<APawn>(Members[i]))
		{
			if(IRTSCore_EntityEquipmentInterface* EntityEquipmentInterface = Cast<IRTSCore_EntityEquipmentInterface>(AiPawn->GetController()))
			{
				if(EntityEquipmentInterface->CanUpdateTarget())
				{
					MembersRequiringTargets.AddUnique(Members[i]);
				}
			}
		}
	}

	// Check if any of the members can see a target, assign target if they can
	TargetSeenEnemy(MembersRequiringTargets);

	// If still unassigned target last know positions
	TargetLastKnownLocation(MembersRequiringTargets);	
}

void URTSEntities_StateMachine::AssignCoverPositions(TArray<URogueCover_Point*> Positions)
{
	if(OwningGroup)
	{
		TArray<AActor*> Members;
		OwningGroup->GetEntities(Members);
		for (int i = 0; i < Members.Num(); ++i)
		{
			for (int j = 0; j < Positions.Num(); ++j)
			{
				if(Positions[i]->IsOccupied())
				{
					continue;
				}				
			}
		}
	}
}

void URTSEntities_StateMachine::TakeCover()
{
	
}

void URTSEntities_StateMachine::Init(ARTSEntities_Group* InGroup)
{
	OwningGroup = InGroup;
	if(OwningGroup)
	{
		URTSEntities_StateMachine* SafeState = NewObject<URTSEntities_StateSafe>(this);
		URTSEntities_StateMachine* CautiousState = NewObject<URTSEntities_StateCautious>(this);
		URTSEntities_StateMachine* CombatState = NewObject<URTSEntities_StateCombat>(this);

		LoadMetaState(SafeState);
		LoadMetaState(CautiousState);
		LoadMetaState(CombatState);

		AddTransition(SafeState, CautiousState, ERTSEntities_BehaviourTriggers::ToCautious);
		AddTransition(SafeState, CombatState, ERTSEntities_BehaviourTriggers::ToCombat);

		AddTransition(CautiousState, CombatState, ERTSEntities_BehaviourTriggers::ToCombat);
		AddTransition(CautiousState, SafeState, ERTSEntities_BehaviourTriggers::ToSafe);

		AddTransition(CombatState, CautiousState, ERTSEntities_BehaviourTriggers::ToCautious);
	}
	else
	{
		UE_LOG(LogRTSEntities, Error, TEXT("[%s] StateMachine Failed to find Manager on Intialisation!"), *GetClass()->GetName());
	}	
}

void URTSEntities_StateMachine::OnEnter()
{
	UE_LOG(LogRTSEntities, Log, TEXT("[%s] Start StateMachine"), *GetClass()->GetName());
}

void URTSEntities_StateMachine::OnUpdate()
{
	UpdateNavigationState();
	UpdateKnownTargets();
}

void URTSEntities_StateMachine::OnExit()
{
	UE_LOG(LogRTSEntities, Log, TEXT("[%s] Exit StateMachine"), *GetClass()->GetName());
	ConditionalBeginDestroy();
}

void URTSEntities_StateMachine::LoadMetaState(URTSEntities_StateMachine* MetaState)
{
	if (MetaStates.Num() == 0)
	{
		DefaultMetaState = MetaState;
	}

	MetaState->Parent = this;

	// Try to add the MetaState to the MetaStates map
	if (!MetaStates.Contains(MetaState->GetClass()))
	{
		MetaStates.Add(MetaState->GetClass(), MetaState);
		MetaState->Init(OwningGroup);
	}
	else
	{
		UE_LOG(LogRTSEntities, Error, TEXT("State %s already contains a MetaState of type %s"), *GetClass()->GetName(), *MetaState->GetClass()->GetName());
	}
}

void URTSEntities_StateMachine::AddTransition(URTSEntities_StateMachine* From, URTSEntities_StateMachine* To, const int32 Trigger) const
{
	// Check if 'From' MetaState has a MetaState of type 'From'
	if (MetaStates.Contains(From->GetClass()))
	{
		// Check if 'To' MetaState has a MetaState of type 'To'
		if (MetaStates.Contains(To->GetClass()))
		{
			From->Transitions.Add(Trigger, To);
		}
		else 
		{
			UE_LOG(LogRTSEntities, Error, TEXT("State %s does not have a MetaState of type %s to transition to."), *GetClass()->GetName(), *To->GetClass()->GetName());
		}
	}
	else 
	{
		UE_LOG(LogRTSEntities, Error, TEXT("State %s does not have a MetaState of type %s to transition from."), *GetClass()->GetName(), *From->GetClass()->GetName());
	}
}

void URTSEntities_StateMachine::ChangeMetaState(const URTSEntities_StateMachine* MetaState)
{
	// Exit the current meta state if one exists
	if(CurrentMetaState != nullptr)
	{
		CurrentMetaState->ExitStateMachine();
	}

	// Create a new meta state for type
	if(URTSEntities_StateMachine* NewMetaState = *MetaStates.Find(MetaState->GetClass()))
	{
		CurrentMetaState = NewMetaState;
		NewMetaState->EnterStateMachine();
	}
	else
	{
		UE_LOG(LogRTSEntities, Error, TEXT("Failed to change from State %s to State %s"), *CurrentMetaState->GetClass()->GetName(), *MetaState->GetClass()->GetName());
	}
}

void URTSEntities_StateMachine::TargetSeenEnemy(TArray<AActor*>& MembersRequiringTargets) const
{
	for (int i = 0; i < OwningGroup->KnownTargets.Num(); ++i)
	{
		// Check if member can see the highest threat target
		for (int j = MembersRequiringTargets.Num() - 1; j >= 0; --j)
		{
			if(const APawn* AiPawn = Cast<APawn>(MembersRequiringTargets[j]))
			{
				if(ARTSEntities_AiControllerEquipment* AiControllerEquipment = Cast<ARTSEntities_AiControllerEquipment>(AiPawn->GetController()))
				{
					FVector ViewPoint;
					FRotator ViewRotation;
					OwningGroup->KnownTargets[i].TargetActor->GetActorEyesViewPoint(ViewPoint, ViewRotation);
					
					if(AiControllerEquipment->HasLineOfSight(ViewPoint))
					{
						AiControllerEquipment->AssignTargetData(OwningGroup->KnownTargets[i]);
						MembersRequiringTargets.RemoveAt(j);
					}
				}
			}
		}
	}
}

void URTSEntities_StateMachine::TargetLastKnownLocation(TArray<AActor*>& MembersRequiringTargets) const
{
	for (int i = 0; i < OwningGroup->KnownTargets.Num(); ++i)
	{
		// Check if member can see the highest threat target
		for (int j = MembersRequiringTargets.Num() - 1; j >= 0; --j)
		{
			if(const APawn* AiPawn = Cast<APawn>(MembersRequiringTargets[j]))
			{
				if(ARTSEntities_AiControllerEquipment* AiControllerEquipment = Cast<ARTSEntities_AiControllerEquipment>(AiPawn->GetController()))
				{
					if(AiControllerEquipment->HasLineOfSight(OwningGroup->KnownTargets[i].LastSeenEnemyLocation))
					{
						AiControllerEquipment->AssignTargetData(OwningGroup->KnownTargets[i]);
						MembersRequiringTargets.RemoveAt(j);
					}
				}
			}
		}
	}
}

void URTSEntities_StateMachine::UpdateNavigationState() const
{
	if(!OwningGroup)
	{
		return;
	}
	
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
			if(const IRTSCore_AiStateInterface* AiStateInterface = Cast<IRTSCore_AiStateInterface>(AiPawn->GetController()))
			{
				// Check if entity is navigating
				if(AiStateInterface->GetState(ERTSCore_StateCategory::Navigation) == static_cast<int32>(ERTSCore_NavigationState::Navigating))
				{
					// Check if current state is not navigating
					if(OwningGroup->GetState(ERTSCore_StateCategory::Navigation) != static_cast<int32>(ERTSCore_NavigationState::Navigating))
					{
						// If one of the members is navigating set group state to navigating
						OwningGroup->SetState(ERTSCore_StateCategory::Navigation, static_cast<int32>(ERTSCore_NavigationState::Navigating));
					}

					// End checks as we have an entity navigating and our state is navigating
					return;
				}
			}
		}		
	}

	// If we got to this point no members are navigating
	if(OwningGroup->GetState(ERTSCore_StateCategory::Navigation) != static_cast<int32>(ERTSCore_NavigationState::Idle))
	{
		// Set state to idle
		OwningGroup->SetState(ERTSCore_StateCategory::Navigation, static_cast<int32>(ERTSCore_NavigationState::Idle));
	}
}

bool URTSEntities_StateMachine::IsNavigating() const
{
	return OwningGroup && OwningGroup->GetState(ERTSCore_StateCategory::Navigation) == static_cast<int32>(ERTSCore_NavigationState::Navigating);
}
