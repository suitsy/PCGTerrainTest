// Fill out your copyright notice in the Description page of Project Settings.


#include "CommandSystem/AsyncTasks/RTSEntities_CreateAsyncFormation.h"
#include "Framework/Entities/Group/RTSEntities_Group.h"
#include "Framework/Entities/Components/RTSEntities_Entity.h"

FRTSEntities_CreateAsyncFormation::FRTSEntities_CreateAsyncFormation(
	const FGuid NewCommandId,
	const FRTSEntities_CommandData& NewCommandData,
	const FRTSEntities_PlayerSelections& NewSelected,
	FRTSEntities_Navigation& NewNavigation)
:
	CommandId(NewCommandId),
	CommandData(NewCommandData),
	Selected(NewSelected),
	Navigation(NewNavigation)
{
}

void FRTSEntities_CreateAsyncFormation::DoWork()
{
	//GetFormationData();
	//GetCommandData(FormationData->Offset);
	//CreateFormation();	
}

/*
URTSEntities_FormationDataAsset* FRTSEntities_CreateAsyncFormation::GetFormationData()
{
	return nullptr;
}

FRTSEntities_CommandData FRTSEntities_CreateAsyncFormation::GetCommandData(const FVector& Offset)
{
	return FRTSEntities_CommandData();
}

void FRTSEntities_CreateAsyncFormation::CreateFormation()
{
	FRTSEntities_Navigation NewFormation = FRTSEntities_Navigation();
	
	if(CommandData.IsValid() && Selected.IsValid())
	{			
		//const bool bIsOddGroupCount = Selected.Selections.Num() % 2 != 0;

		
		/*if(Selected.IsValid() && Selected.Selections.Num() == 1)
		{
			// Single Selection
			
			// Create single selection formation
			TArray<FRTSEntities_FormationPosition> Positions;
			Positions.Add(FRTSEntities_FormationPosition(CommandData.Location, ERTSEntities_SelectionSide::CenterSide, 0));
			FRTSEntities_Formation NewFormation = FRTSEntities_Formation(0.f, Positions);

			// Create single selection entities formation


			// Assign entities to formation
			if(Selected.Selections.IsValidIndex(0))
			{
				AssignEntityPositions(Selected.Selections[0].Entities, NewFormation.Positions);
			}
		
		}
		else
		{#1#
			// Multiple Selections

			// Create multi selection estimated group formation
			const float EstimatedSpacing = GetSelectedAverageSpacing();
			NewFormation = FRTSEntities_Navigation(CommandId, EstimatedSpacing, CreateFormationPositions(EstimatedSpacing));
		
			// Assign groups to positions
			AssignSelectionPositions(NewFormation.Positions);

			// Update formation with actual group position data
			UpdateSelectionAssignedPositions(NewFormation.Positions);

			
			for (int i = 0; i < NewFormation.Positions.Num(); ++i)
			{				
				// Create entity estimated formation positions
				CreateEntityPositions(NewFormation.Positions[i]);

				// Assign entities to positions
				AssignEntityPositions(NewFormation.Positions[i]);
				
				// Assign entities to formation
				UpdateEntityPositions(NewFormation.Positions[i]);
			}
			

			// Update formation with actual entity position data
		
		//}
	}

	Navigation = NewFormation;
}

float FRTSEntities_CreateAsyncFormation::GetSelectedAverageSpacing() const
{
	float SelectedAvgSpacing = 0.f;
	for (int i = 0; i < Selected.Selections.Num(); ++i)
	{
		SelectedAvgSpacing += GetSelectionSpacing(Selected.Selections[i]);
		SelectedAvgSpacing += CommandData.GroupSpacing;
	}

	// Prevent divide by zero
	if(SelectedAvgSpacing == 0.f) { return 500.f; }
	
	return SelectedAvgSpacing /= Selected.Selections.Num();
}

float FRTSEntities_CreateAsyncFormation::GetSelectionSpacing(const FRTSEntities_PlayerSelection& SingleSelection) const
{
	float SelectionSpacing = 0.f;
	for (int i = 0; i < SingleSelection.Entities.Num(); ++i)
	{
		const float EntityReqSpacing = GetEntitySpacing(SingleSelection.Entities[i]);
		const float EntitySpacing = i != 0 ? CommandData.EntitySpacing : 0.f;
		SelectionSpacing += EntityReqSpacing + EntitySpacing;
	}

	return SelectionSpacing;
}

float FRTSEntities_CreateAsyncFormation::GetEntitiesAverageSpacing(const TArray<AActor*>& Entities, const float Spacing) const
{
	float AverageSpacing = 0.f;
	for (int i = 0; i < Entities.Num(); ++i)
	{
		const float EntityReqSpacing = GetEntitySpacing(Entities[i]);
		const float EntitySpacing = i != 0 ? Spacing : 0.f;
		AverageSpacing += EntityReqSpacing + EntitySpacing;
	}

	// Prevent divide by zero
	if(AverageSpacing == 0.f) { return 50.f; }
	
	return AverageSpacing /= Entities.Num();
}

float FRTSEntities_CreateAsyncFormation::GetEntitySpacing(const AActor* Entity) const
{
	if(const APawn* Pawn = Cast<APawn>(Entity))
	{
		// Is selection entity
		if(const URTSEntities_Entity* EntityComponent = URTSEntities_Entity::FindEntityComponent(Pawn))
		{
			return EntityComponent->GetSpacing();
		}
	}

	return 25.f;
}

TArray<FRTSEntities_FormationPosition> FRTSEntities_CreateAsyncFormation::CreateFormationPositions(const float Spacing) const
{
	TArray<FRTSEntities_FormationPosition> Positions = TArray<FRTSEntities_FormationPosition>();

	// Reset spacing variables
	float SpacingLeft = 0.f, SpacingRight = 0.f, SpacingCenter = 0.f;
	
	// Check if there is an odd count to selections (require center position for odd)
	const bool bIsOddCount = Selected.Selections.Num() % 2 != 0;
	
	for (int i = 0; i < Selected.Selections.Num(); ++i)
	{
		// Reset a local offset for this calculation
		FVector LocalOffset = FormationData->Offset;
		
		// Check if formation has alternating positions
		if(FormationData->Alt)
		{
			// If there is a odd number of selections add a center position
			if(i == 0 && bIsOddCount)
			{
				// Assign a position with a center side, an zero index and a temp location
				Positions.Add(FRTSEntities_FormationPosition(CommandData.Location, CommandData, ERTSEntities_SelectionSide::CenterSide));

				// Update spacing for left and right positions
				SpacingLeft = Spacing;
				SpacingRight = Spacing;

				continue;
			}		

			// Check if the position is right or left side
			if (i % 2 != 0)
			{
				if(SpacingLeft == 0.f && !bIsOddCount)
				{
					SpacingLeft += Spacing * 0.5f;
				}
				
				// Apply spacing and inversion for left side offset position
				LocalOffset *= SpacingLeft;
				LocalOffset.Y = LocalOffset.Y * -1;
				LocalOffset = CommandData.Rotation.RotateVector(LocalOffset);
				Positions.Add(FRTSEntities_FormationPosition(CommandData.Location + LocalOffset, CommandData, ERTSEntities_SelectionSide::LeftSide));

				// Add spacing for next left position
				SpacingLeft += Spacing;
			}
			else
			{
				if(SpacingRight == 0.f && !bIsOddCount)
				{
					SpacingRight += Spacing * 0.5f;
				}
				
				// Assign a position with a right side, an R-index and a temp location
				LocalOffset *= SpacingRight;
				LocalOffset = CommandData.Rotation.RotateVector(LocalOffset);
				Positions.Add(FRTSEntities_FormationPosition(CommandData.Location + LocalOffset, CommandData, ERTSEntities_SelectionSide::RightSide));

				// Add spacing for next left position
				SpacingRight += Spacing;
			}
		}
		else
		{
			if(i != 0)
			{
				SpacingCenter += Spacing;
				LocalOffset *= SpacingCenter;
			}
			
			// Assign a position with a no side, offset determines position only
			Positions.Add(FRTSEntities_FormationPosition((i == 0 ? CommandData.Location : CommandData.Location + LocalOffset), CommandData, ERTSEntities_SelectionSide::NoSide));
		}
	}

	return Positions;
}

bool FRTSEntities_CreateAsyncFormation::NextPositionPermutation(int* Array, int Size) const
{
	// Find the largest index i such that array[i] < array[i+1]
	int i = Size - 2;
	while (i >= 0 && Array[i] >= Array[i + 1])
	{
		--i;
	}

	// If no such index is found, the sequence is already the largest permutation
	if (i < 0)
	{
		return false;
	}

	// Find the largest index j greater than i such that array[j] > array[i]
	int j = Size - 1;
	while (Array[j] <= Array[i])
	{
		--j;
	}

	// Swap array[i] and array[j]
	std::swap(Array[i], Array[j]);

	// Reverse the sequence from i+1 to the end
	std::reverse(Array + i + 1, Array + Size);

	return true;
}

void FRTSEntities_CreateAsyncFormation::AssignSelectionPositions(TArray<FRTSEntities_FormationPosition>& Positions) const
{
	float MinTotalDistance = FLT_MAX;

	// Build array of index's to use to assign a location
	TArray<int32> IndexArray;
	for (int32 i = 0; i < Selected.Selections.Num(); ++i)
	{
		IndexArray.Add(i);
	}

	// Set a best index array to the current default array
	// After permutations this array will be the formation positions sorted by entity index
	TArray<int32> BestIndexArray = IndexArray;

	do
	{
		// Get this permutation's total distance of each group to a separate location
		float TotalDistance = 0.0f;
		for (int32 j = 0; j < Selected.Selections.Num(); ++j)
		{
			if(Selected.Selections[j].Lead != nullptr)
			{
				FVector CurrentLocation = Positions[IndexArray[j]].Destination;
				TotalDistance += (Selected.Selections[j].Lead->GetActorLocation() - CurrentLocation).Length();
			}
		}

		// Check if the total distance is less than the previous minimum total distance
		if (TotalDistance < MinTotalDistance)
		{
			// Update the min dist
			MinTotalDistance = TotalDistance;

			// Update the best index array to this index array
			BestIndexArray = IndexArray;
		}

	} while (NextPositionPermutation(IndexArray.GetData(), Selected.Selections.Num()));

	// Assign the positions to the group selections
	for (int32 k = 0; k < Selected.Selections.Num(); ++k)
	{
		// Assign the selection the position for this permutation
		if(Selected.Selections[k].IsValid() && Positions.IsValidIndex(IndexArray[k]))
		{
			Positions[BestIndexArray[k]].Owner = Selected.Selections[k].Group;
			Positions[BestIndexArray[k]].SourceLocation = Selected.Selections[k].GetSelectionCenterPosition();
			Positions[BestIndexArray[k]].Selection = Selected.Selections[k];
		}
	}
}

void FRTSEntities_CreateAsyncFormation::UpdateSelectionAssignedPositions(TArray<FRTSEntities_FormationPosition>& Positions) const
{
	// Set spacing 
	float SpacingLeft = 0, SpacingRight = 0, SpacingCenter = 0;		

	for (int i = 0; i < Positions.Num(); ++i)
	{
		if(!Positions[i].IsValid())
		{
			continue;
		}
		
		// Reset a local offset for this calculation
		FVector LocalOffset = FormationData->Offset;

		// Get Spacing for this position
		Positions[i].SelectionSpacing = GetSelectionSpacing(Positions[i].Selection);
		
		if(FormationData->Alt)
		{
			// If there is a center position there must be an odd number of selections
			if(Positions[i].Side == ERTSEntities_SelectionSide::CenterSide)
			{
				// Assign command location to this positions location
				Positions[i].Destination = Positions[i].CommandData.Location;

				// Add the selections spacing plus the group spacing left and right for next positions
				SpacingLeft += Positions[i].SelectionSpacing * 0.5f + Positions[i].CommandData.GroupSpacing * 0.5f;
				SpacingRight += Positions[i].SelectionSpacing * 0.5f + Positions[i].CommandData.GroupSpacing * 0.5f;
			}

			if(Positions[i].Side == ERTSEntities_SelectionSide::LeftSide)
			{
				// Invert offset for left side
				LocalOffset.Y = LocalOffset.Y * -1;
				
				// Add 50% of the group spacing and 50% of the selections spacing to get the selection center location
				SpacingLeft += Positions[i].SelectionSpacing * 0.5f + Positions[i].CommandData.GroupSpacing * 0.5f;				

				// Add spacing and local rotation to offset 
				LocalOffset *= SpacingLeft;			
				LocalOffset = Positions[i].CommandData.Rotation.RotateVector(LocalOffset);

				// Calculate the offset	location
				Positions[i].Destination = Positions[i].CommandData.Location + LocalOffset;

				// Add rest of this selections spacing
				SpacingLeft += Positions[i].SelectionSpacing * 0.5f + Positions[i].CommandData.GroupSpacing * 0.5f;	
			}

			if(Positions[i].Side == ERTSEntities_SelectionSide::RightSide)
			{
				// Add 50% of the group spacing and 50% of the selections spacing to get the selection center location
				SpacingRight += Positions[i].SelectionSpacing * 0.5f + Positions[i].CommandData.GroupSpacing * 0.5f;				

				// Add spacing and local rotation to offset 
				LocalOffset *= SpacingRight;			
				LocalOffset = Positions[i].CommandData.Rotation.RotateVector(LocalOffset);

				// Calculate the offset location
				Positions[i].Destination = Positions[i].CommandData.Location + LocalOffset;	

				// Add rest of this selections spacing
				SpacingRight += Positions[i].SelectionSpacing * 0.5f + Positions[i].CommandData.GroupSpacing * 0.5f;	
			}
		}
		else
		{
			// Offset all but the first position
			if(i != 0)
			{
				SpacingCenter += Positions[i].SelectionSpacing * 0.5f + Positions[i].CommandData.GroupSpacing * 0.5f;
				LocalOffset *= SpacingCenter;			
				LocalOffset = Positions[i].CommandData.Rotation.RotateVector(FormationData->Offset);
			}

			// Set first position to command location, use offset for all following
			Positions[i].Destination = i == 0 ? Positions[i].CommandData.Location : Positions[i].CommandData.Location + LocalOffset;
		}
	}
}

void FRTSEntities_CreateAsyncFormation::CreateEntityPositions(FRTSEntities_FormationPosition& Position) const
{
	// Reset spacing variables
	float SpacingLeft = 0.f, SpacingRight = 0.f, SpacingCenter = 0.f;
	
	// Check if there is an odd count to selections (require center position for odd)
	const bool bIsOddCount = Position.Selection.Entities.Num() % 2 != 0;

	const float Spacing = GetEntitiesAverageSpacing(Position.Selection.Entities, CommandData.EntitySpacing);
	
	for (int FormationIndex = 0; FormationIndex < Position.Selection.Entities.Num(); ++FormationIndex)
	{
		// Reset a local offset for this calculation
		FVector LocalOffset = FormationData->Offset;
		
		// Check if formation has alternating positions
		if(FormationData->Alt)
		{
			// If there is a odd number of selections add a center position
			if(FormationIndex == 0 && bIsOddCount)
			{
				// Assign a position with a center side, an zero index and a temp location
				Position.EntityPositions.Add(FRTSEntities_EntityPosition(Position.Destination, Position.Destination, ERTSEntities_SelectionSide::CenterSide));

				// Update spacing for left and right positions
				SpacingLeft = Spacing * 0.5f;
				SpacingRight = Spacing * 0.5f;

				continue;
			}		

			// Check if the position is right or left side
			if (FormationIndex % 2 != 0)
			{
				if(SpacingLeft == 0.f && !bIsOddCount)
				{
					SpacingLeft += Spacing * 0.5f;
				}
				
				// Apply spacing and inversion for left side offset position
				LocalOffset *= SpacingLeft;
				LocalOffset.Y = LocalOffset.Y * -1;
				LocalOffset = CommandData.Rotation.RotateVector(LocalOffset);
				Position.EntityPositions.Add(FRTSEntities_EntityPosition(Position.Destination + LocalOffset, Position.Destination, ERTSEntities_SelectionSide::LeftSide));

				// Add spacing for next left position
				SpacingLeft += Spacing * 0.5f;;
			}
			else
			{
				if(SpacingRight == 0.f && !bIsOddCount)
				{
					SpacingRight += Spacing * 0.5f;
				}
				
				// Assign a position with a right side, an R-index and a temp location
				LocalOffset *= SpacingRight;
				LocalOffset = CommandData.Rotation.RotateVector(LocalOffset);
				Position.EntityPositions.Add(FRTSEntities_EntityPosition(Position.Destination + LocalOffset, Position.Destination, ERTSEntities_SelectionSide::RightSide));

				// Add spacing for next left position
				SpacingRight += Spacing * 0.5f;;
			}
		}
		else
		{
			if(FormationIndex != 0)
			{
				SpacingCenter += Spacing;
				LocalOffset *= SpacingCenter;
			}
			
			// Assign a position with a no side, offset determines position only
			Position.EntityPositions.Add(FRTSEntities_EntityPosition((FormationIndex == 0 ? Position.Destination : Position.Destination + LocalOffset), Position.Destination, ERTSEntities_SelectionSide::NoSide));
		}
	}
}

void FRTSEntities_CreateAsyncFormation::AssignEntityPositions(FRTSEntities_FormationPosition& Position) const
{
	float MinTotalDistance = FLT_MAX;

	// Build array of index's to use to assign a location
	TArray<int32> IndexArray;
	for (int32 i = 0; i < Position.Selection.Entities.Num(); ++i)
	{
		IndexArray.Add(i);
	}

	// Set a best index array to the current default array
	// After permutations this array will be the formation positions sorted by entity index
	TArray<int32> BestIndexArray = IndexArray;

	do
	{
		// Get this permutation's total distance of each group to a separate location
		float TotalDistance = 0.0f;
		for (int32 j = 0; j < Position.Selection.Entities.Num(); ++j)
		{
			if(Position.Selection.Entities[j] != nullptr)
			{
				FVector CurrentLocation = Position.EntityPositions[IndexArray[j]].Destination;
				TotalDistance += (Position.Selection.Entities[j]->GetActorLocation() - CurrentLocation).Length();
			}
		}

		// Check if the total distance is less than the previous minimum total distance
		if (TotalDistance < MinTotalDistance)
		{
			// Update the min dist
			MinTotalDistance = TotalDistance;

			// Update the best index array to this index array
			BestIndexArray = IndexArray;
		}

	} while (NextPositionPermutation(IndexArray.GetData(), Position.Selection.Entities.Num()));

	// Assign the positions to the group selections
	for (int32 k = 0; k < Position.Selection.Entities.Num(); ++k)
	{
		// Assign the selection the position for this permutation
		if(Position.Selection.Entities[k] != nullptr && Position.EntityPositions.IsValidIndex(IndexArray[k]))
		{
			Position.EntityPositions[BestIndexArray[k]].Owner = Position.Selection.Entities[k];
		}
	}
}

void FRTSEntities_CreateAsyncFormation::UpdateEntityPositions(FRTSEntities_FormationPosition& Position) const
{
	// Set spacing 
	float SpacingLeft = 0, SpacingRight = 0, SpacingCenter = 0;		

	for (int i = 0; i < Position.EntityPositions.Num(); ++i)
	{
		if(!Position.EntityPositions[i].IsValid())
		{
			continue;
		}
		
		// Reset a local offset for this calculation
		FVector LocalOffset = FormationData->Offset;

		// Get entity's required spacing
		const float RequiredSpacing = GetEntitySpacing(Position.EntityPositions[i].Owner);
		
		if(FormationData->Alt)
		{
			// If there is a center position there must be an odd number of selections
			if(Position.EntityPositions[i].Side == ERTSEntities_SelectionSide::CenterSide)
			{
				// Assign command location to this positions location
				Position.EntityPositions[i].Destination = Position.Destination;

				// Add the selections spacing plus the group spacing left and right for next positions
				SpacingLeft += RequiredSpacing * 0.5f + CommandData.EntitySpacing * 0.5f;
				SpacingRight += RequiredSpacing * 0.5f + CommandData.EntitySpacing * 0.5f;
			}

			if(Position.EntityPositions[i].Side == ERTSEntities_SelectionSide::LeftSide)
			{
				// Invert offset for left side
				LocalOffset.Y = LocalOffset.Y * -1;
			
				// Add 50% of the group spacing and 50% of the selections spacing to get the selection center location
				SpacingLeft += RequiredSpacing * 0.5f + CommandData.EntitySpacing * 0.5f;
				Position.EntityPositions[i].Spacing = SpacingLeft;

				// Add spacing and local rotation to offset 
				LocalOffset *= SpacingLeft;			
				LocalOffset = CommandData.Rotation.RotateVector(LocalOffset);

				// Calculate the offset	location
				Position.EntityPositions[i].Destination = Position.Destination + LocalOffset;

				// Add rest of this selections spacing
				SpacingLeft += RequiredSpacing * 0.5f + CommandData.EntitySpacing * 0.5f;	
			}

			if(Position.EntityPositions[i].Side == ERTSEntities_SelectionSide::RightSide)
			{
				// Add 50% of the group spacing and 50% of the selections spacing to get the selection center location
				SpacingRight += RequiredSpacing * 0.5f + CommandData.EntitySpacing * 0.5f;				
				Position.EntityPositions[i].Spacing = SpacingRight;
				
				// Add spacing and local rotation to offset 
				LocalOffset *= SpacingRight;			
				LocalOffset = CommandData.Rotation.RotateVector(LocalOffset);

				// Calculate the offset location
				Position.EntityPositions[i].Destination = Position.Destination + LocalOffset;	

				// Add rest of this selections spacing
				SpacingRight += RequiredSpacing * 0.5f + CommandData.EntitySpacing * 0.5f;	
			}
		}
		else
		{
			// Offset all but the first position
			if(i != 0)
			{
				SpacingCenter += RequiredSpacing * 0.5f + CommandData.EntitySpacing * 0.5f;
				Position.EntityPositions[i].Spacing = SpacingCenter;
				LocalOffset *= SpacingCenter;			
				LocalOffset = CommandData.Rotation.RotateVector(FormationData->Offset);
			}

			// Set first position to command location, use offset for all following
			Position.EntityPositions[i].Destination = i == 0 ? Position.Destination : Position.Destination + LocalOffset;
		}		
	}
}*/