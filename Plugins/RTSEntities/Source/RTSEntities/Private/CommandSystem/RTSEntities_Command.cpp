// Fill out your copyright notice in the Description page of Project Settings.


#include "CommandSystem/RTSEntities_Command.h"
#include "Framework/Entities/Group/RTSEntities_Group.h"

URTSEntities_Command::URTSEntities_Command()
{
}

void URTSEntities_Command::Execute()
{
	Activate();		
}

void URTSEntities_Command::Fail()
{
	OnCommandFailed.Broadcast(Data.Id);
}

void URTSEntities_Command::Complete(const FGuid CompleteId, const ARTSEntities_Group* Group, const ERTSEntities_CommandStatus StatusUpdate, const FVector& Location)
{
	if(ERTSEntities_CommandStatus* StatusPtr = GroupStatus.Find(Group))
	{
		*StatusPtr = StatusUpdate;
	}

	if(CheckStatus() == ERTSEntities_CommandStatus::Completed)
	{
		SetComplete();

		if(!Location.IsNearlyZero())
		{
			SetLocation(Location);
		}

        OnCommandComplete.Broadcast(Data.Id, true);
	}	
}

bool URTSEntities_Command::IsLocalController() const
{
	if(const APlayerController* PlayerController = Cast<APlayerController>(GetOuter()))
	{
		return PlayerController->IsLocalController();
	}

	return false;
}

bool URTSEntities_Command::HasAuthority() const
{
	if(const APlayerController* PlayerController = Cast<APlayerController>(GetOuter()))
	{
		return PlayerController->HasAuthority();
	}

	return false;
}

FVector URTSEntities_Command::GetSelectionCenter(const FRTSEntities_PlayerSelection& Selection)
{
	FVector CenterPosition = FVector::ZeroVector;
	for (int i = 0; i < Selection.Entities.Num(); ++i)
	{
		CenterPosition += Selection.Entities[i]->GetActorLocation();
	}

	if(Selection.Entities.Num() > 0)
	{
		CenterPosition /= Selection.Entities.Num();
	}

	return CenterPosition;
}

TArray<AActor*> URTSEntities_Command::GetGroupsSelectedEntities(const ARTSEntities_Group* Group)
{
	for (int i = 0; i < Data.Selected.Selections.Num(); ++i)
	{
		if(Data.Selected.Selections[i].IsValid() && Data.Selected.Selections[i].Group != nullptr && Data.Selected.Selections[i].Group == Group)
		{
			return Data.Selected.Selections[i].Entities;
		}
	}

	return TArray<AActor*>();
}

bool URTSEntities_Command::Valid() const
{
	return Data.Selected.Selections.Num() > 0;
}

void URTSEntities_Command::Activate()
{	
	// Set status to active
	Data.Status = ERTSEntities_CommandStatus::Active;

	// Build command complete map
	for (int i = 0; i < Data.Selected.Selections.Num(); ++i)
	{
		if(Data.Selected.Selections[i].IsValid())
		{
			if(!Data.Selected.Selections[i].Group->AssignedCommand(Data.Id) && !Data.Selected.Selections[i].Group->HasCompletedCommand(Data.Id))
			{				
				GroupStatus.Add(Data.Selected.Selections[i].Group, Data.Status);
			}
		}
	}
}

ERTSEntities_CommandStatus URTSEntities_Command::CheckStatus() const
{
	// Generate array of status values from the group status map
	TArray<ERTSEntities_CommandStatus> MemberStates;
	GroupStatus.GenerateValueArray(MemberStates);

	// Check for any active states
	for (const ERTSEntities_CommandStatus& MemberStatus : MemberStates)
	{		
		if(MemberStatus == ERTSEntities_CommandStatus::Active)
		{
			// If active state found command is still active, break out
			return ERTSEntities_CommandStatus::Active;
		}
	}

	return ERTSEntities_CommandStatus::Completed;
}

/*void URTSEntities_Command::Set(const FRTSEntities_CommandData CommandData, const FPlayerSelection Selection)
{
	Data = CommandData;
	OwningSelection = Selection;
}*/

void URTSEntities_Command::SetSelected(const FRTSEntities_PlayerSelections& InSelected)
{
	Data.Selected = InSelected;

	// Assign data generated from the current selection
	AssignSelectionData();
}

void URTSEntities_Command::AssignSelectionData()
{
	FVector CenterLocation = FVector::ZeroVector;
	GetCommandCenterLocation(CenterLocation);
	
	// Get the group lead closest to the center of the selection, if only one group skip check
	Data.LeadSelectionIndex = 0;
	if(Data.Selected.Selections.Num() > 1)
	{
		float Distance = MAX_FLT;
		for (int i = 0; i < Data.Selected.Selections.Num(); ++i)
		{
			const float SelectionDistance = FVector::DistSquared(Data.Selected.Selections[i].Lead->GetActorLocation(), CenterLocation);
			if(SelectionDistance < Distance)
			{
				Distance = SelectionDistance;
				Data.LeadSelectionIndex = i;
			}
		}
	}

	// If lead group selection is valid, assign source data
	if(Data.Selected.Selections.IsValidIndex(Data.LeadSelectionIndex))
	{
		// Assign command source data, source location and rotation
		if(ARTSEntities_Group* LeadGroup = Data.Selected.Selections[Data.LeadSelectionIndex].Group)
		{	
			// Set the source position to the group leads position or the center of the groups position if no group lead
			const AActor* LeadMember = LeadGroup->GetLead(Data.Selected.Selections[Data.LeadSelectionIndex].Entities);
			if(LeadMember != nullptr)
			{
				Data.SourceTransform.SetLocation(LeadMember->GetActorLocation());
			}
			else
			{
				Data.SourceTransform.SetLocation(LeadGroup->GetCenterPosition());
			}
			
			// Check if orientation has been assigned
			if(LeadGroup->GetOrientation().Equals(FRotator::ZeroRotator, 0.0001f))
			{
				// Set the source rotation to the groups orientation
				Data.SourceTransform.SetRotation(LeadGroup->GetOrientation().Quaternion());
			}
			else
			{
				// If groups orientation is not assigned, set rotation to the direction of the current command
				if(Data.TargetTransform.GetLocation() != FVector::ZeroVector)
				{
					Data.SourceTransform.SetRotation(FRotationMatrix::MakeFromX(Data.TargetTransform.GetLocation() - Data.SourceTransform.GetLocation()).ToQuat());
				}
				else
				{
					// If the command location is not set, set rotation to no rotation
					Data.SourceTransform.SetRotation(FQuat::Identity);
				}				
			}
		}
	}
	else
	{
		Data.SourceTransform = FTransform::Identity;
	}
}

void URTSEntities_Command::GetCommandCenterLocation(FVector& CenterLocation) const
{
	for (int i = 0; i < Data.Selected.Selections.Num(); ++i)
	{
		if(Data.Selected.Selections[i].IsValid())
		{
			if(const AActor* GroupLead = Data.Selected.Selections[i].Group->GetLead(Data.Selected.Selections[i].Entities))
			{
				CenterLocation += GroupLead->GetActorLocation();
			}
		}		
	}
	
	if(Data.Selected.Selections.Num() > 0)
	{
		CenterLocation /= Data.Selected.Selections.Num();
	}
}

/*void URTSEntities_Command::OnComplete(const FGuid Id)
{
	if(Id == Data.Id)
	{
		Complete();
	}
}

void URTSEntities_Command::OnAborted(const FGuid Id)
{
	if(Id == Data.Id)
	{
		Aborted();
	}
}*/
