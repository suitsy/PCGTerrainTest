// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Data/RTSEntities_FormationDataAsset.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Framework/Entities/Components/RTSEntities_Entity.h"
#include "Framework/Entities/Group/RTSEntities_Group.h"


void URTSEntities_FormationDataAsset::CreateFormation(FRTSEntities_Navigation& Navigation, const TArray<FRTSEntities_PlayerSelection>& Selections) const
{
	if(Navigation.IsValid() && Selections.Num() > 0)
	{			
		//const bool bIsOddGroupCount = Selections.Num() % 2 != 0;		
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
		{*/
			// Multiple Selections

			// Create multi selection estimated group formation
			const float EstimatedSpacing = GetSelectedAverageSpacing(Navigation, Selections);
			CreateFormationPositions(Navigation, Selections, EstimatedSpacing);
		
			// Assign groups to positions
			AssignSelectionPositions(Navigation, Selections);

			// Update formation with actual group position data
			UpdateSelectionAssignedPositions(Navigation);

			
			for (int i = 0; i < Navigation.Positions.Num(); ++i)
			{				
				// Create entity estimated formation positions
				CreateEntityPositions(Navigation, i);

				// Assign entities to positions
				AssignEntityPositions(Navigation.Positions[i]);
				
				// Assign entities to formation
				UpdateEntityPositions(Navigation, i);
			}
		
		//}
	}	
}

float URTSEntities_FormationDataAsset::GetSelectedAverageSpacing(const FRTSEntities_Navigation& Navigation, const TArray<FRTSEntities_PlayerSelection>& Selections)
{
	float SelectedAvgSpacing = 0.f;
	for (int i = 0; i < Selections.Num(); ++i)
	{
		SelectedAvgSpacing += GetSelectionSpacing(Selections[i], Navigation.EntitySpacing);
		SelectedAvgSpacing += Navigation.GroupSpacing;
	}

	// Prevent divide by zero
	if(SelectedAvgSpacing == 0.f) { return 500.f; }
	
	return SelectedAvgSpacing /= Selections.Num();
}

float URTSEntities_FormationDataAsset::GetSelectionSpacing(const FRTSEntities_PlayerSelection& Selection, const float CommandEntitySpacing)
{
	float SelectionSpacing = 0.f;
	for (int i = 0; i < Selection.Entities.Num(); ++i)
	{
		const float EntityReqSpacing = GetEntitySpacing(Selection.Entities[i]) * 2.f;
		//const float EntitySpacing = i != 0 ? CommandEntitySpacing : 0.f;
		SelectionSpacing += EntityReqSpacing + CommandEntitySpacing;
	}

	return SelectionSpacing;
}

float URTSEntities_FormationDataAsset::GetEntitiesAverageSpacing(const TArray<AActor*>& Entities, const float Spacing)
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

float URTSEntities_FormationDataAsset::GetEntitySpacing(const AActor* Entity)
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

void URTSEntities_FormationDataAsset::CreateFormationPositions(FRTSEntities_Navigation& Navigation, const TArray<FRTSEntities_PlayerSelection>& Selections,
	const float Spacing) const
{
	// Reset spacing variables
	float SpacingLeft = 0.f, SpacingRight = 0.f, SpacingCenter = 0.f;
	
	// Check if there is an odd count to selections (require center position for odd)
	const bool bIsOddCount = Selections.Num() % 2 != 0;

	// Force a side by side formation for groups 2 or less
	FVector FormationOffset = Offset;
	if(Selections.Num() <= 2)
	{
		FormationOffset = FVector(0.f, 1.f, 0.f);
	}
	
	for (int i = 0; i < Selections.Num(); ++i)
	{
		// Reset a local offset for this calculation
		FVector LocalOffset = FormationOffset;
		
		// Check if formation has alternating positions
		if(Alt)
		{
			// If there is a odd number of selections add a center position
			if(i == 0 && bIsOddCount)
			{
				// Assign a position with a center side, an zero index and a temp location
				Navigation.Positions.Add(FRTSEntities_FormationPosition(Navigation.Location, Navigation.Rotation, Navigation.Offset, ERTSEntities_SelectionSide::CenterSide));

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
				LocalOffset = Navigation.Rotation.RotateVector(LocalOffset);
				Navigation.Positions.Add(FRTSEntities_FormationPosition(Navigation.Location + LocalOffset, Navigation.Rotation, Navigation.Offset, ERTSEntities_SelectionSide::LeftSide));
		
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
				LocalOffset = Navigation.Rotation.RotateVector(LocalOffset);
				Navigation.Positions.Add(FRTSEntities_FormationPosition(Navigation.Location + LocalOffset, Navigation.Rotation, Navigation.Offset, ERTSEntities_SelectionSide::RightSide));
								
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
			Navigation.Positions.Add(FRTSEntities_FormationPosition((i == 0 ? Navigation.Location : Navigation.Location + LocalOffset), Navigation.Rotation, Navigation.Offset, ERTSEntities_SelectionSide::NoSide));
		}		
	}
}

bool URTSEntities_FormationDataAsset::NextPositionPermutation(int* Array, int Size)
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

void URTSEntities_FormationDataAsset::AssignSelectionPositions(FRTSEntities_Navigation& Navigation, const TArray<FRTSEntities_PlayerSelection>& Selections)
{
	float MinTotalDistance = FLT_MAX;

	// Build array of index's to use to assign a location
	TArray<int32> IndexArray;
	for (int32 i = 0; i < Selections.Num(); ++i)
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
		for (int32 j = 0; j < Selections.Num(); ++j)
		{
			if(Selections[j].Lead != nullptr)
			{
				FVector CurrentLocation = Navigation.Positions[IndexArray[j]].Destination;
				TotalDistance += (Selections[j].Lead->GetActorLocation() - CurrentLocation).Length();
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

	} while (NextPositionPermutation(IndexArray.GetData(), Selections.Num()));

	// Assign the positions to the group selections
	for (int32 k = 0; k < Selections.Num(); ++k)
	{
		// Assign the selection the position for this permutation
		if(Selections[k].IsValid() && Navigation.Positions.IsValidIndex(IndexArray[k]))
		{
			Navigation.Positions[BestIndexArray[k]].Owner = Selections[k].Group;
			Navigation.Positions[BestIndexArray[k]].SourceLocation = Selections[k].GetSelectionCenterPosition();
			Navigation.Positions[BestIndexArray[k]].Selection = Selections[k];
		}
	}
}

void URTSEntities_FormationDataAsset::UpdateSelectionAssignedPositions(FRTSEntities_Navigation& Navigation) const
{
	// Set spacing 
	float SpacingLeft = 0, SpacingRight = 0, SpacingCenter = 0;		

	for (int i = 0; i < Navigation.Positions.Num(); ++i)
	{
		if(!Navigation.Positions[i].IsValid())
		{
			continue;
		}

		// Reset a local offset for this calculation
		// Force a side by side formation for groups 2 or less
		FVector LocalOffset = Offset;
		if(Navigation.Positions.Num() <= 2)
		{
			LocalOffset = FVector(0.f, 1.f, 0.f);
		}

		// Get Spacing for this position
		Navigation.Positions[i].SelectionSpacing = GetSelectionSpacing(Navigation.Positions[i].Selection, Navigation.EntitySpacing) + (Navigation.GroupSpacing * 2.f);
		//UE_LOG(LogRTSEntities, Warning, TEXT("[%s] SelectionSpacing: %f, Entities: %i"), *GetClass()->GetName(), Positions[i].SelectionSpacing, Positions[i].Selection.Entities.Num());
		
		if(Alt)
		{
			// If there is a center position there must be an odd number of selections
			if(Navigation.Positions[i].Side == ERTSEntities_SelectionSide::CenterSide)
			{
				// Assign command location to this positions location
				Navigation.Positions[i].Destination = Navigation.Location;

				// Add the selections spacing plus the group spacing left and right for next positions
				SpacingLeft += Navigation.Positions[i].SelectionSpacing * 0.5f + Navigation.GroupSpacing * 0.5f;
				SpacingRight += Navigation.Positions[i].SelectionSpacing * 0.5f + Navigation.GroupSpacing * 0.5f;
			}

			if(Navigation.Positions[i].Side == ERTSEntities_SelectionSide::LeftSide)
			{
				// Invert offset for left side
				LocalOffset.Y = LocalOffset.Y * -1;
				
				// Add 50% of the group spacing and 50% of the selections spacing to get the selection center location
				SpacingLeft += Navigation.Positions[i].SelectionSpacing * 0.5f + Navigation.GroupSpacing * 0.5f;				

				// Add spacing and local rotation to offset 
				LocalOffset *= SpacingLeft;			
				LocalOffset = Navigation.Rotation.RotateVector(LocalOffset);

				// Calculate the offset	location
				Navigation.Positions[i].Destination = Navigation.Location + LocalOffset;

				// Add rest of this selections spacing
				SpacingLeft += Navigation.Positions[i].SelectionSpacing * 0.5f + Navigation.GroupSpacing * 0.5f;	
			}

			if(Navigation.Positions[i].Side == ERTSEntities_SelectionSide::RightSide)
			{
				// Add 50% of the group spacing and 50% of the selections spacing to get the selection center location
				SpacingRight += Navigation.Positions[i].SelectionSpacing * 0.5f + Navigation.GroupSpacing * 0.5f;				

				// Add spacing and local rotation to offset 
				LocalOffset *= SpacingRight;			
				LocalOffset = Navigation.Rotation.RotateVector(LocalOffset);

				// Calculate the offset location
				Navigation.Positions[i].Destination = Navigation.Location + LocalOffset;	

				// Add rest of this selections spacing
				SpacingRight += Navigation.Positions[i].SelectionSpacing * 0.5f + Navigation.GroupSpacing * 0.5f;	
			}
		}
		else
		{
			// Offset all but the first position
			if(i != 0)
			{
				SpacingCenter += Navigation.Positions[i].SelectionSpacing * 0.5f + Navigation.GroupSpacing * 0.5f;
				LocalOffset *= SpacingCenter;			
				LocalOffset = Navigation.Rotation.RotateVector(Offset);
			}

			// Set first position to command location, use offset for all following
			Navigation.Positions[i].Destination = i == 0 ? Navigation.Location : Navigation.Location + LocalOffset;
		}
	}
}

void URTSEntities_FormationDataAsset::CreateEntityPositions(FRTSEntities_Navigation& Navigation, const int32 Index) const
{
	if(!Navigation.Positions.IsValidIndex(Index))
	{
		return;
	}
	
	// Reset spacing variables
	float SpacingLeft = 0.f, SpacingRight = 0.f, SpacingCenter = 0.f;
	
	// Check if there is an odd count to selections (require center position for odd)
	const bool bIsOddCount = Navigation.Positions[Index].Selection.Entities.Num() % 2 != 0;

	const float Spacing = GetEntitiesAverageSpacing(Navigation.Positions[Index].Selection.Entities, Navigation.EntitySpacing);
	
	for (int FormationIndex = 0; FormationIndex < Navigation.Positions[Index].Selection.Entities.Num(); ++FormationIndex)
	{
		// Reset a local offset for this calculation
		FVector LocalOffset = Offset;
		
		// Check if formation has alternating positions
		if(Alt)
		{
			// If there is a odd number of selections add a center position
			if(FormationIndex == 0 && bIsOddCount)
			{
				// Assign a position with a center side, an zero index and a temp location
				Navigation.Positions[Index].EntityPositions.Add(FRTSEntities_EntityPosition(Navigation.Positions[Index].Destination,
					Navigation.Positions[Index].Destination, Navigation.Rotation, ERTSEntities_SelectionSide::CenterSide));

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
				LocalOffset = Navigation.Rotation.RotateVector(LocalOffset);
				Navigation.Positions[Index].EntityPositions.Add(FRTSEntities_EntityPosition(Navigation.Positions[Index].Destination + LocalOffset,
					Navigation.Positions[Index].Destination, Navigation.Rotation, ERTSEntities_SelectionSide::LeftSide));

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
				LocalOffset = Navigation.Rotation.RotateVector(LocalOffset);
				Navigation.Positions[Index].EntityPositions.Add(FRTSEntities_EntityPosition(Navigation.Positions[Index].Destination + LocalOffset,
					Navigation.Positions[Index].Destination, Navigation.Rotation, ERTSEntities_SelectionSide::RightSide));

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
			Navigation.Positions[Index].EntityPositions.Add(FRTSEntities_EntityPosition((FormationIndex == 0 ? Navigation.Positions[Index].Destination :
				Navigation.Positions[Index].Destination + LocalOffset), Navigation.Positions[Index].Destination, Navigation.Rotation, ERTSEntities_SelectionSide::NoSide));
		}
	}
}

void URTSEntities_FormationDataAsset::AssignEntityPositions(FRTSEntities_FormationPosition& Position)
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

void URTSEntities_FormationDataAsset::UpdateEntityPositions(FRTSEntities_Navigation& Navigation, const int32 Index) const
{
	// Set spacing 
	float SpacingLeft = 0, SpacingRight = 0, SpacingCenter = 0;		

	for (int i = 0; i < Navigation.Positions[Index].EntityPositions.Num(); ++i)
	{
		if(!Navigation.Positions[Index].EntityPositions[i].IsValid())
		{
			continue;
		}
		
		// Reset a local offset for this calculation
		FVector LocalOffset = Offset;

		// Get entity's required spacing
		const float RequiredSpacing = GetEntitySpacing(Navigation.Positions[Index].EntityPositions[i].Owner);
		
		if(Alt)
		{
			// If there is a center position there must be an odd number of selections
			if(Navigation.Positions[Index].EntityPositions[i].Side == ERTSEntities_SelectionSide::CenterSide)
			{
				// Assign command location to this positions location
				Navigation.Positions[Index].EntityPositions[i].Destination = Navigation.Positions[Index].Destination;

				// Add the selections spacing plus the group spacing left and right for next positions
				SpacingLeft += RequiredSpacing * 0.5f + Navigation.EntitySpacing * 0.5f;
				SpacingRight += RequiredSpacing * 0.5f + Navigation.EntitySpacing * 0.5f;
			}

			if(Navigation.Positions[Index].EntityPositions[i].Side == ERTSEntities_SelectionSide::LeftSide)
			{
				// Invert offset for left side
				LocalOffset.Y = LocalOffset.Y * -1;
			
				// Add 50% of the group spacing and 50% of the selections spacing to get the selection center location
				SpacingLeft += RequiredSpacing * 0.5f + Navigation.EntitySpacing * 0.5f;
				Navigation.Positions[Index].EntityPositions[i].Spacing = SpacingLeft;

				// Add spacing and local rotation to offset 
				LocalOffset *= SpacingLeft;			
				LocalOffset = Navigation.Rotation.RotateVector(LocalOffset);

				// Calculate the offset	location
				Navigation.Positions[Index].EntityPositions[i].Destination = Navigation.Positions[Index].Destination + LocalOffset;

				// Add rest of this selections spacing
				SpacingLeft += RequiredSpacing * 0.5f + Navigation.EntitySpacing * 0.5f;	
			}

			if(Navigation.Positions[Index].EntityPositions[i].Side == ERTSEntities_SelectionSide::RightSide)
			{
				// Add 50% of the group spacing and 50% of the selections spacing to get the selection center location
				SpacingRight += RequiredSpacing * 0.5f + Navigation.EntitySpacing * 0.5f;				
				Navigation.Positions[Index].EntityPositions[i].Spacing = SpacingRight;
				
				// Add spacing and local rotation to offset 
				LocalOffset *= SpacingRight;			
				LocalOffset = Navigation.Rotation.RotateVector(LocalOffset);

				// Calculate the offset location
				Navigation.Positions[Index].EntityPositions[i].Destination = Navigation.Positions[Index].Destination + LocalOffset;	

				// Add rest of this selections spacing
				SpacingRight += RequiredSpacing * 0.5f + Navigation.EntitySpacing * 0.5f;	
			}
		}
		else
		{
			// Offset all but the first position
			if(i != 0)
			{
				SpacingCenter += RequiredSpacing * 0.5f + Navigation.EntitySpacing * 0.5f;
				Navigation.Positions[Index].EntityPositions[i].Spacing = SpacingCenter;
				LocalOffset *= SpacingCenter;			
				LocalOffset = Navigation.Rotation.RotateVector(Offset);
			}

			// Set first position to command location, use offset for all following
			Navigation.Positions[Index].EntityPositions[i].Destination = i == 0 ? Navigation.Positions[Index].Destination : Navigation.Positions[Index].Destination + LocalOffset;
		}		
	}
}

void URTSEntities_FormationDataAsset::GenerateFormationReferencePaths(FRTSEntities_Navigation& Navigation) const
{
	if(!Navigation.IsValid())
	{
		return;
	}
		
	for (int i = 0; i < Navigation.Positions.Num(); ++i)
	{
		if(Navigation.Positions[i].IsValid() && Navigation.Positions[i].Selection.IsValid() && Navigation.Positions[i].Selection.Lead != nullptr)
		{
			UWorld* World = Navigation.Positions[i].Selection.Lead->GetWorld();
			if(!World)
			{
				return;
			}
			
			const UNavigationSystemV1* NavSystem = Cast<UNavigationSystemV1>(World->GetNavigationSystem());
			if(!NavSystem)
			{
				return;
			}
			
			// Set form up point to start position 
			FVector FormUpPoint = Navigation.Positions[i].SourceLocation;				

			// Generate a temp guide path from group navigation data
			const UNavigationPath* GuidePath = NavSystem->FindPathToLocationSynchronously(World, FormUpPoint, Navigation.Positions[i].Destination, Navigation.Positions[i].Selection.Lead);
		
			// Get the minimum of: 50% of the path length or the groups max form up distance
			const float HalfPathLength = GuidePath->GetPathLength() * 0.5f;
			const float MaxFormUpDist = Navigation.Positions[i].Selection.Group->GetMaxFormUpDistance();
			const float FormUpDistance = FMath::Min(HalfPathLength, MaxFormUpDist);

			// Interpolate along the path until we find two points that overlap the form up point
			float DistanceAlongPath = 0;
			for (int32 j = 0; j < GuidePath->PathPoints.Num() - 1; j++)
			{
				const float SegmentLength = (GuidePath->PathPoints[j + 1] - GuidePath->PathPoints[j]).Length();
				if (DistanceAlongPath + SegmentLength >= FormUpDistance)
				{
					// Segment is further than the form up point, lerp from the last point the remaining distance
					FormUpPoint = FMath::Lerp(GuidePath->PathPoints[j], GuidePath->PathPoints[j + 1], (FormUpDistance - DistanceAlongPath) / SegmentLength);
					break;
				}
				else
				{
					DistanceAlongPath += SegmentLength;
				}			
			}
		
		
			// Generate groups reference path points from group navigation data
			Navigation.Positions[i].ReferencePathPoints = NavSystem->FindPathToLocationSynchronously(World, FormUpPoint, Navigation.Positions[i].Destination, Navigation.Positions[i].Selection.Lead)->PathPoints;				
		}
	}	
}
