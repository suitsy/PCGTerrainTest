// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Data/RTSEntities_DataTypes.h"
#include "Framework/Entities/Components/RTSEntities_Entity.h"
#include "Framework/Entities/Group/RTSEntities_Group.h"
#include "Framework/Data/RTSEntities_GroupDataAsset.h"


FVector FRTSEntities_PlayerSelection::GetSelectionCenterPosition() const
{
	FVector CenterPosition = FVector::ZeroVector;
	for (int i = 0; i < Entities.Num(); ++i)
	{
		CenterPosition += Entities[i]->GetActorLocation();
	}

	if(Entities.Num() > 0)
	{
		CenterPosition /= Entities.Num();
	}

	return CenterPosition;
}

AActor* FRTSEntities_PlayerSelection::GetClosestMemberToPosition(const TArray<AActor*>& GroupMembers, const FVector& Destination)
{
	AActor* ClosestMember = nullptr;
	float ClosestDistanceSquared = MAX_FLT;

	for (int i = 0; i < GroupMembers.Num(); ++i)
	{
		if (GroupMembers[i] != nullptr)
		{
			const float DistanceSquared = FVector::DistSquared(GroupMembers[i]->GetActorLocation(), Destination);
			if (DistanceSquared < ClosestDistanceSquared)
			{
				ClosestDistanceSquared = DistanceSquared;
				ClosestMember = GroupMembers[i];
			}
		}
	}

	return ClosestMember;
}

float FRTSEntities_PlayerSelection::GetMaxFormUpDistance() const
{
	if(Group != nullptr)
	{
		if(const URTSEntities_GroupDataAsset* GroupData = Group->GetData())
		{
			return GroupData->MaxFormUpDistance;
		}
	}

	return 99999;
}

void FRTSEntities_PlayerSelection::UpdateGroupLead()
{
	AActor* NewLeader = nullptr;
	
	if(Group)
	{
		int32 LeaderIndex = 99;	
		
		for (int i = 0; i < Entities.Num(); ++i)
		{
			if(const URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(Entities[i]))
			{
				if(Entity->GetIndex() < LeaderIndex)
				{
					NewLeader = Entities[i];
					LeaderIndex = Entity->GetIndex();
				}
			}		
		}
	}

	Lead = NewLeader;
}

void FRTSEntities_PlayerSelection::SetSelectionFormation(const FPrimaryAssetId& Formation)
{
	for (int i = 0; i < Entities.Num(); ++i)
	{
		if(URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(Entities[i]))
		{
			Entity->SetFormation(Formation);
		}
	}
}

bool FRTSEntities_PlayerSelections::IsValid() const
{
	return Selections.Num() > 0
		&& LeadSelectionIndex != -1
		&& Selections.IsValidIndex(LeadSelectionIndex)
		&& Selections[LeadSelectionIndex].Group != nullptr;
}

TArray<FPrimaryAssetId> FRTSEntities_PlayerSelections::GetFormationData() const
{
	if(Selections.IsValidIndex(LeadSelectionIndex))
	{
		if(Selections[LeadSelectionIndex].Group)
		{
			return Selections[LeadSelectionIndex].Group->GetFormationData();
		}
	}

	return TArray<FPrimaryAssetId>();
}

FPrimaryAssetId FRTSEntities_PlayerSelections::GetFormation() const
{
	if(Selections.IsValidIndex(LeadSelectionIndex))
	{
		if(const URTSEntities_Entity* EntityComponent = URTSEntities_Entity::FindEntityComponent(Selections[LeadSelectionIndex].Lead))
		{
			return EntityComponent->GetFormation();
		}		
	}

	return FPrimaryAssetId();
}

FPrimaryAssetId FRTSEntities_PlayerSelections::GetNextFormation(const float Input) const
{
	const TArray<FPrimaryAssetId> AvailableFormationData = GetFormationData();
	const FPrimaryAssetId CurrentFormation = GetFormation();

	// Determine the current formation index
	int32 Index = 0.f;
	for (int i = 0; i < AvailableFormationData.Num(); ++i)
	{
		if(AvailableFormationData[i].IsValid() && AvailableFormationData[i] == CurrentFormation)
		{
			Index = i;
			break;
		}
	}

	// Convert float input to integer
	const int32 InputInt = FMath::RoundToInt(Input);

	// Calculate next index wrapping selection over array
	const int32 NextIndex = (Index + InputInt) % AvailableFormationData.Num();

	return AvailableFormationData.IsValidIndex(NextIndex) ? AvailableFormationData[NextIndex] : FPrimaryAssetId();
}

ARTSEntities_Group* FRTSEntities_PlayerSelections::GetLeadGroup() const
{
	if(IsValid() && Selections.IsValidIndex(LeadSelectionIndex))
	{
		return Selections[LeadSelectionIndex].Group;
	}

	return nullptr;
}

void FRTSEntities_CommandData::ApplyClientData(const FRTSEntities_ClientCommandData& ClientData)
{
	TargetTransform = ClientData.TargetTransform;
	CommandType = ClientData.CommandType;
	HasNavigation = ClientData.HasNavigation;
	BehaviourState = ClientData.BehaviourState;
}

void FRTSEntities_CommandData::SetSelected(const FRTSEntities_PlayerSelections& InSelected)
{
	Selected = InSelected;

	// Assign data generated from the current selection
	AssignSelectionData();
}

void FRTSEntities_CommandData::AssignSelectionData()
{
	FVector CenterLocation = FVector::ZeroVector;
	GetCommandCenterLocation(CenterLocation);
	
	// Get the group lead closest to the center of the selection, if only one group skip check
	LeadSelectionIndex = 0;
	if(Selected.Selections.Num() > 1)
	{
		float Distance = MAX_FLT;
		for (int i = 0; i < Selected.Selections.Num(); ++i)
		{
			const float SelectionDistance = FVector::DistSquared(Selected.Selections[i].Lead->GetActorLocation(), CenterLocation);
			if(SelectionDistance < Distance)
			{
				Distance = SelectionDistance;
				LeadSelectionIndex = i;
			}
		}
	}

	// If lead group selection is valid, assign source data
	if(Selected.Selections.IsValidIndex(LeadSelectionIndex))
	{
		// Assign command source data, source location and rotation
		if(ARTSEntities_Group* LeadGroup = Selected.Selections[LeadSelectionIndex].Group)
		{	
			// Set the source position to the group leads position or the center of the groups position if no group lead
			const AActor* LeadMember = LeadGroup->GetLead(Selected.Selections[LeadSelectionIndex].Entities);
			if(LeadMember != nullptr)
			{
				SourceTransform.SetLocation(LeadMember->GetActorLocation());
			}
			else
			{
				SourceTransform.SetLocation(LeadGroup->GetCenterPosition());
			}
			
			// Check if orientation has been assigned
			if(LeadGroup->GetOrientation().Equals(FRotator::ZeroRotator, 0.0001f))
			{
				// Set the source rotation to the groups orientation
				SourceTransform.SetRotation(LeadGroup->GetOrientation().Quaternion());
			}
			else
			{
				// If groups orientation is not assigned, set rotation to the direction of the current command
				if(TargetTransform.GetLocation() != FVector::ZeroVector)
				{
					SourceTransform.SetRotation(FRotationMatrix::MakeFromX(TargetTransform.GetLocation() - SourceTransform.GetLocation()).ToQuat());
				}
				else
				{
					// If the command location is not set, set rotation to no rotation
					SourceTransform.SetRotation(FQuat::Identity);
				}				
			}
		}
	}
	else
	{
		SourceTransform = FTransform::Identity;
	}
}

void FRTSEntities_CommandData::GetCommandCenterLocation(FVector& CenterLocation) const
{
	for (int i = 0; i < Selected.Selections.Num(); ++i)
	{
		if(Selected.Selections[i].IsValid())
		{
			if(const AActor* GroupLead = Selected.Selections[i].Group->GetLead(Selected.Selections[i].Entities))
			{
				CenterLocation += GroupLead->GetActorLocation();
			}
		}		
	}
	
	if(Selected.Selections.Num() > 0)
	{
		CenterLocation /= Selected.Selections.Num();
	}
}

void FRTSEntities_GroupState::SetState(const ERTSCore_StateCategory Category, const int32 State)
{
	switch (Category)
	{
		case ERTSCore_StateCategory::Behaviour:
			Behaviour = static_cast<ERTSCore_BehaviourState>(State);
			break;
		case ERTSCore_StateCategory::Speed:
			Speed = static_cast<ERTSCore_SpeedState>(State);
			break;
		case ERTSCore_StateCategory::Condition:
			Condition = static_cast<ERTSCore_ConditionState>(State);
			break;
		case ERTSCore_StateCategory::Posture:
			Posture = static_cast<ERTSCore_PostureState>(State);
			break;
		case ERTSCore_StateCategory::Navigation:
			Navigation = static_cast<ERTSCore_NavigationState>(State);
			break;
		default: ;
	}
}

int32 FRTSEntities_GroupState::GetState(const ERTSCore_StateCategory Category)
{
	switch (Category)
	{
		case ERTSCore_StateCategory::Behaviour:
			return static_cast<int32>(Behaviour);
		case ERTSCore_StateCategory::Speed:
			return static_cast<int32>(Speed);
		case ERTSCore_StateCategory::Condition:
			return static_cast<int32>(Condition);
		case ERTSCore_StateCategory::Posture:
			return static_cast<int32>(Posture);
		case ERTSCore_StateCategory::Navigation:
			return static_cast<int32>(Navigation);
		default: return 0;
	}
}

