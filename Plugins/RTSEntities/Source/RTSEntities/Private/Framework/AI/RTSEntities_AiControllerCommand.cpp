// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AI/RTSEntities_AiControllerCommand.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CommandSystem/RTSEntities_Command.h"
#include "CommandSystem/RTSEntities_NavigateTo.h"
#include "Components/SplineComponent.h"
#include "Framework/Entities/Components/RTSEntities_Entity.h"
#include "Framework/Data/RTSCore_SystemStatics.h"
#include "Framework/Interfaces/RTSCore_EntityInterface.h"
#include "Framework/Settings/RTSCore_DeveloperSettings.h"
#include "Navigation/PathFollowingComponent.h"


// Sets default values
ARTSEntities_AiControllerCommand::ARTSEntities_AiControllerCommand(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;
	ActiveCommand = nullptr;
	CurrentWaypointIndex = -1;
}

void ARTSEntities_AiControllerCommand::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(IsActive())
	{
		// Handle any active navigation for ai
		HandleWaypointNavigation();
	}
}


void ARTSEntities_AiControllerCommand::BeginPlay()
{
	Super::BeginPlay();
}

void ARTSEntities_AiControllerCommand::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ARTSEntities_AiControllerCommand::InitAiForGameplay()
{
	Super::InitAiForGameplay();
}

void ARTSEntities_AiControllerCommand::BehaviourUpdate_NavigationState()
{
	if(GetBlackboardComponent())
	{
		if(Waypoints.Num() == 0)
		{
			SetState(ERTSCore_StateCategory::Navigation, static_cast<int32>(ERTSCore_NavigationState::Idle));			
		}
		else
		{
			SetState(ERTSCore_StateCategory::Navigation, static_cast<int32>(ERTSCore_NavigationState::Navigating));			
		}
	}
}

void ARTSEntities_AiControllerCommand::BehaviourUpdate_BehaviourState(const ERTSCore_BehaviourState& StateRequest)
{
	if(GetBlackboardComponent())
	{		
		SetState(ERTSCore_StateCategory::Behaviour, static_cast<int32>(StateRequest));
		
		if(HasEntityComponent())
		{
			EntityComponent->OnBehaviourStateChange(StateRequest);
		}		
	}
}

bool ARTSEntities_AiControllerCommand::IsActiveCommand(const FGuid Id) const
{
	return ActiveCommand != nullptr && ActiveCommand->GetId() == Id;
}

void ARTSEntities_AiControllerCommand::ExecuteCommand(URTSEntities_Command* Command)
{
	if(Command != nullptr)
	{
		// End ActiveCommand if one
		if(ActiveCommand != nullptr && ActiveCommand->GetId() != Command->GetId())
		{
			CompleteCurrentCommand(ERTSEntities_CommandStatus::Updating);
		}

		// Assign new command as active
		ActiveCommand = Command;
		ActiveCommand->EntityStatus = ERTSEntities_CommandStatus::Active;

		if(ActiveCommand != nullptr)
		{
			// Update behaviour
			BehaviourUpdate_BehaviourState(ActiveCommand->Data.BehaviourState);
			
			// Check command is a navigation command
			if(const URTSEntities_NavigateTo* NavCommand = Cast<URTSEntities_NavigateTo>(Command))
			{		
				// Assign current navigation data to variable on server
				NavigationData = NavCommand->GetNavigation();
			
				if(NavigationData.IsValid())
				{
					ExecuteNavigation();
				}
			}
			else
			{
				NavigationData = FRTSEntities_Navigation();
			}
		}
	}
}

void ARTSEntities_AiControllerCommand::ChangeFormation(const FPrimaryAssetId NewFormation)
{
	// Check if we have a valid current waypoint, its not the destination waypoint and
	// we have a valid waypoint after the current that is also not the destination
	/*if(Waypoints.IsValidIndex(CurrentWaypointIndex) && Waypoints[CurrentWaypointIndex].WaypointType != ERTSEntities_WaypointType::Destination)
	{
		const int32 NextWaypointIndex = GetNextValidWaypointIndex();
		if(NextWaypointIndex > 0)
		{
			for (int i = NextWaypointIndex; i < Waypoints.Num(); ++i)
			{
				if(Waypoints[i].MoveType == ERTSEntities_NavigationType::Formation)
				{
					Waypoints[i].Formation = NewFormation;
					FVector NewLocation;
					CalculateOffsetLocation(Waypoints[i].Formation, EntityNavigationData.ReferencePathPoints[i], NewLocation);
					Waypoints[i].Location = NewLocation;
				}
			}
		}
	}*/
}

void ARTSEntities_AiControllerCommand::AbortNavigation()
{
	// Check we current have a valid waypoint
	if(Waypoints.IsValidIndex(CurrentWaypointIndex))
	{
		// If valid check the next waypoint is the destination but its further than out stopping distance
		if(Waypoints[CurrentWaypointIndex].WaypointType == ERTSEntities_WaypointType::Destination && Waypoints[CurrentWaypointIndex].DistanceToCompletion > GetAcceptanceDistance() * 2.f)
		{
			// Find a new stopping point and set to our current destination
			CreateTransitionWaypoint(ERTSEntities_WaypointType::Destination);			
		}
		else
		{
			// If not the destination create a new destination at min stop distance
			CreateTransitionWaypoint(ERTSEntities_WaypointType::Destination);
		}
	}	
}

void ARTSEntities_AiControllerCommand::CompleteCurrentCommand(const ERTSEntities_CommandStatus Status)
{
	// Report to group entity command completed
	FGuid CurrentCommandId = FGuid();
	if(ActiveCommand != nullptr)
	{
		ActiveCommand->EntityStatus = Status;
		switch (Status)
		{
			case ERTSEntities_CommandStatus::Completed:
				ResetWaypoints();				
				break;
			case ERTSEntities_CommandStatus::Aborted:
				AbortNavigation();
				break;
			case ERTSEntities_CommandStatus::Updating:
				CreateTransitionWaypoint(ERTSEntities_WaypointType::Start);
				break;
			default: ;
		}
		
		CurrentCommandId = ActiveCommand->GetId();
		ActiveCommand = nullptr;
	}
	
	if(HasEntityComponent() && CurrentCommandId.IsValid())
	{
		if(ARTSEntities_Group* Group = EntityComponent->GetGroup())
		{
			Group->OnEntityCommandComplete(CurrentCommandId, Status, IsNavigating() ? GetCurrentDestination() : GetPawn()->GetActorLocation());				
		}
	}		
}

void ARTSEntities_AiControllerCommand::ExecuteNavigation()
{
	if(!NavigationData.IsValid() || !HasEntityComponent())
	{
		return;
	}
	
	for (int i = 0; i < NavigationData.Positions.Num(); ++i)
	{
		if(const ARTSEntities_Group* PositionGroup = Cast<ARTSEntities_Group>(NavigationData.Positions[i].Owner))
		{
			if(EntityComponent->GetGroup() != PositionGroup)
			{
				continue;
			}

			// Generate formation reference path for entities group
			GenerateFormationReferencePath(NavigationData.Positions[i]);
			
			// Source this entity's formation position
			for (int j = 0; j < NavigationData.Positions[i].EntityPositions.Num(); ++j)
			{
				if(NavigationData.Positions[i].EntityPositions[j].Owner == GetPawn())
				{
					EntityPosition = NavigationData.Positions[i].EntityPositions[j];
					if(EntityPosition.IsValid())
					{		
						TArray<FVector> EntityPathPoints;
						GenerateEntityFormationPath(NavigationData.Positions[i], EntityPathPoints);
						GenerateWaypoints(EntityPathPoints);		
		
						if(ActiveCommand != nullptr)
						{
							EntityComponent->HandleDestinationMarker(true, EntityPosition, ActiveCommand->GetStatus());
							return;
						}					
					}
					
					EntityComponent->HandleDestinationMarker(false);
					return;
				}
			}				
		}
	}	
}

bool ARTSEntities_AiControllerCommand::IsNavigating() const
{
	return false;
}

const UNavigationSystemV1* ARTSEntities_AiControllerCommand::GetNavSystemChecked() const
{
	const UNavigationSystemV1* NavSystem = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem());
	check(NavSystem != nullptr);
	return NavSystem;
}

bool ARTSEntities_AiControllerCommand::NavigationIsValid()
{
	if(!NavigationData.IsValid() || !EntityPosition.IsValid() || !GetPawn())
	{
		CompleteCurrentCommand(ERTSEntities_CommandStatus::Aborted);
		return false;
	}

	return true;
}

void ARTSEntities_AiControllerCommand::GenerateEntityFormationPath(FRTSEntities_FormationPosition& FormationPosition, TArray<FVector>& EntityPathPoints)
{
	if(NavigationIsValid())
	{	
		/** Check if we need to follow the complete formation offset path **/
		TArray<FVector> FormationPathPoints;
		FVector DestinationOffsetLocation = FVector::ZeroVector;		
		FVector StartLocation = GetPawn()->GetActorLocation();
			const bool bHasExistingNavigation = Waypoints.Num() > 0;

		// Get the offset destination for this entity
		if(FormationPosition.ReferencePathPoints.IsValidIndex(FormationPosition.ReferencePathPoints.Num() - 1))
		{
			// Calculate the destination offset location
			CalculateOffsetLocation(
				FormationPosition.ReferencePathPoints[FormationPosition.ReferencePathPoints.Num() - 1],
				FormationPosition.Rotation,
				DestinationOffsetLocation,
				FormationPosition.Offset
			);
		}
				
		// Generate a path from the entities location to the offset destination
		const UNavigationPath* DirectPath = GetNavSystemChecked()->FindPathToLocationSynchronously(GetWorld(), GetPawn()->GetActorLocation(), DestinationOffsetLocation, GetPawn());
		if(DirectPath->GetPathLength() < NavigationData.FormationThreshold)
		{
			FormationPathPoints.Append(DirectPath->PathPoints);
		}

		/** Generate an offset path for this entity using the guide path if direct route not used **/
		if(FormationPathPoints.Num() <= 0)
		{
			FVector OffsetLocation = FVector::ZeroVector;
			FVector NextOffsetLocation = FVector::ZeroVector;
		
			// Create the formation part of the path (excluding from the entity to the form up point)
			for (int i = 0; i < FormationPosition.ReferencePathPoints.Num() - 1; ++i)
			{
				// Check we have a valid reference point for the current point and the next point
				if(!FormationPosition.ReferencePathPoints.IsValidIndex(i) || !FormationPosition.ReferencePathPoints.IsValidIndex(i + 1))
				{
					continue;
				}

				const bool bIsLastPoint = (i == FormationPosition.ReferencePathPoints.Num() - 2);

				// Check if this is the first location		
				if(i == 0)
				{
					// Calculate the initial offset location				
					CalculateOffsetLocation(
						StartLocation,
						FRotationMatrix::MakeFromX(FormationPosition.ReferencePathPoints[i] - GetPawn()->GetActorLocation()).Rotator(),
						OffsetLocation,
						FormationPosition.Offset
					);

					StartLocation = OffsetLocation;
				}

				if(bIsLastPoint)
				{
					NextOffsetLocation = DestinationOffsetLocation;
				}
				else
				{
					// Calculate the next location
					const FRotator PointRotation = bIsLastPoint
					? FormationPosition.Rotation
					: FRotationMatrix::MakeFromX(FormationPosition.ReferencePathPoints[i + 1] - OffsetLocation).Rotator();
				
					CalculateOffsetLocation(
						FormationPosition.ReferencePathPoints[i + 1],
						PointRotation,
						NextOffsetLocation,
						FormationPosition.Offset
					);
				}			
	
				// To ensure navigation between new offset points generate the path segment between the two new offset points
				TArray<FVector> SegmentNavPoints = GetNavSystemChecked()->FindPathToLocationSynchronously(GetWorld(), OffsetLocation, NextOffsetLocation, GetPawn())->PathPoints;

				// Set the next location to the current location for next point check
				OffsetLocation = NextOffsetLocation;
			
				// Ensure offset start position is included 
				/** Combine navigation points into a single path
				 *  For each segment we need to remove the last point as it will be the same as the next segments first point
				 *  except where this is the last segment **/
				if(SegmentNavPoints.Num() > 0 && i < FormationPosition.ReferencePathPoints.Num() - 2)
				{
					SegmentNavPoints.RemoveAt(SegmentNavPoints.Num() - 1);
				}			

				// Add segment to path
				FormationPathPoints.Append(SegmentNavPoints);
			}
		}

		/** Check if the formation path does not begin at the same location, add the navigation from entity to the beginning of formation path **/
		if(FormationPosition.UseFormation && !bHasExistingNavigation && FormationPathPoints.IsValidIndex(0) && (FormationPathPoints[0] - GetPawn()->GetActorLocation()).Length() > 250.f)
		{			
			// Calculate a path from the pawns location to the start of the formation path
			EntityPathPoints = GetNavSystemChecked()->FindPathToLocationSynchronously(GetWorld(), GetPawn()->GetActorLocation(), FormationPathPoints[0], GetPawn())->PathPoints;
		
			/** So the entity can move to the form up point without using formations
			 *  we need to reference which waypoint is the begining of using a formation (the form up waypoint)
			 *  This will be the first FormationPathPoint or the last point of the entity path (which should be the same) **/
			EntityPosition.FormUpIndex = EntityPathPoints.Num() - 1;

			// Removing the last point of the start path we can then append the rest of the path
			// (form up index remains valid, ie the first FormationPathPoint)
			EntityPathPoints.RemoveAt(EntityPathPoints.Num() - 1);
			EntityPathPoints.Append(FormationPathPoints);
		}
		else
		{
			// If not using formations or the first nav point is the same as the actors location then just use the current entity path points
			EntityPathPoints = FormationPathPoints;
		}
	}
}

void ARTSEntities_AiControllerCommand::GenerateFormationReferencePath(FRTSEntities_FormationPosition& FormationPosition)
{
	if(FormationPosition.Selection.IsValid() && FormationPosition.Selection.Lead != nullptr && HasEntityComponent())
	{			
		// Set form up point to start position 
		FVector FormUpPoint = FormationPosition.SourceLocation;				

		// Generate a temp guide path from group navigation data
		const UNavigationPath* GuidePath = GetNavSystemChecked()->FindPathToLocationSynchronously(GetWorld(), FormUpPoint, FormationPosition.Destination, FormationPosition.Selection.Lead);
	
		// Get the minimum of: 50% of the path length or the groups max form up distance
		const float PathLength = GuidePath->GetPathLength();
		const float FormationThreshold = EntityComponent->GetFormationThreshold();

		// If the path length is greater than the formation threshold length then the reference path should be from the form up point as we will use a formation for the move
		FormationPosition.UseFormation = PathLength > FormationThreshold;		
		
		if(FormationPosition.UseFormation)
		{
			const float MaxFormUpDist = FormationPosition.Selection.Group->GetMaxFormUpDistance();
			const float FormUpDistance = FMath::Min(GuidePath->GetPathLength() * 0.5f, MaxFormUpDist);

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
		}	
	
		// Generate groups reference path points from group navigation data
		FormationPosition.ReferencePathPoints = GetNavSystemChecked()->FindPathToLocationSynchronously(GetWorld(), FormUpPoint, FormationPosition.Destination, FormationPosition.Selection.Lead)->PathPoints;
	}
}

void ARTSEntities_AiControllerCommand::CalculateOffsetLocation(FVector& ReferencePoint, const FRotator& Rotation, FVector& OffsetLocation, const FVector& Offset)
{
	if(NavigationIsValid())
	{		
		// Reset a local offset for this calculation
		FVector LocalOffset = Offset;
		
		// Check if the entities position is the center position, then there is no offset
		if(EntityPosition.Side == ERTSEntities_SelectionSide::CenterSide)
		{
			OffsetLocation = ReferencePoint;
			return;
		}		

		// Check if the entities position is on the left, invert the offset
		if(EntityPosition.Side == ERTSEntities_SelectionSide::LeftSide)
		{
			LocalOffset.Y = LocalOffset.Y * -1;			
		}

		// Apply spacing
		LocalOffset *= EntityPosition.Spacing;

		// Convert the location from local space to world space
		LocalOffset = Rotation.RotateVector(LocalOffset);

		// Add offset to reference position
		OffsetLocation = ReferencePoint + LocalOffset;
	}	
}

int32 ARTSEntities_AiControllerCommand::GetCurrentDestinationIndex()
{
	if(Waypoints.Num() > 0)
	{
		// Check if the current waypoint is the destination		
		if(Waypoints.IsValidIndex(CurrentWaypointIndex) && Waypoints[CurrentWaypointIndex].WaypointType == ERTSEntities_WaypointType::Destination)
		{
			return CurrentWaypointIndex;
		}
		else
		{
			// Check if distance to next waypoint is greater than our acceptance distance plus some wiggle room
			if(Waypoints[CurrentWaypointIndex].DistanceToCompletion > GetAcceptanceDistance() * 2.f)
			{
				// Assign this waypoint as our destination
				return CurrentWaypointIndex;
			}
			else
			{
				// Check if the next waypoint is not the destination
				if(Waypoints.IsValidIndex(CurrentWaypointIndex + 1) && Waypoints[CurrentWaypointIndex + 1].WaypointType != ERTSEntities_WaypointType::Destination)
				{
					// Assign the next waypoint as the destination
					return CurrentWaypointIndex + 1;
				}
			}
		}

		return Waypoints.Num() - 1;
	}

	return -1;
}

FVector ARTSEntities_AiControllerCommand::GetCurrentDestination()
{
	if(Waypoints.Num() > 0)
	{
		const int32 NextWaypointIndex = GetCurrentDestinationIndex();
		if(Waypoints.IsValidIndex(NextWaypointIndex))
		{
			return Waypoints[NextWaypointIndex].Location;
		}
		else if(Waypoints.IsValidIndex(Waypoints.Num() - 1))
		{
			return Waypoints[Waypoints.Num() - 1].Location;
		}
	} 

	return GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
}

void ARTSEntities_AiControllerCommand::SetDestination()
{
	for (int i = Waypoints.Num() - 1; i >= 0; --i)
	{
		if(Waypoints[i].WaypointType == ERTSEntities_WaypointType::Destination)
		{
			if(HasEntityComponent())
			{
				EntityComponent->SetNavigationDestination(Waypoints[i].Location);
			}
			
			return;
		}
	}
}

void ARTSEntities_AiControllerCommand::GenerateWaypoints(TArray<FVector>& EntityPathPoints)
{
	/** Convert our path points to waypoints struct so we can reference data on each point **/
	if(!NavigationIsValid() || !HasEntityComponent())
	{
		CompleteCurrentCommand(ERTSEntities_CommandStatus::Aborted);
	}

	// Generate navigation waypoints from generated navigation path points
	TArray<FRTSEntities_NavigationWaypoint> NewWaypoints;
	bool bHasFormedUp = false;
	float DestinationDirectDistance = MAX_FLT;
	if(EntityPathPoints.IsValidIndex(0))
	{
		DestinationDirectDistance = (EntityPathPoints[0] - EntityPathPoints[EntityPathPoints.Num() - 1]).Length();
	}

	const bool bShouldUseFormUp = DestinationDirectDistance > EntityComponent->GetFormationThreshold();
	
	for (int i = 0; i < EntityPathPoints.Num(); ++i)
	{
		// Check if we have reach the form up nav point (this will be always false when no form up required ie. -1)
		if(bShouldUseFormUp && i >= EntityPosition.FormUpIndex)
		{
			bHasFormedUp = true;
		}

		// Check if the first waypoint
		if(i == 0)
		{
			// Add a start waypoint
			URTSCore_SystemStatics::GetTerrainPosition(GetWorld(), EntityPathPoints[i]);
			NewWaypoints.Add(FRTSEntities_NavigationWaypoint(
				(Waypoints.Num() > 0 ? ERTSEntities_WaypointType::Nav : ERTSEntities_WaypointType::Start),
				EntityPathPoints[i],
				EntityPathPoints.Num() > 1 ? FRotationMatrix::MakeFromX(EntityPathPoints[i+1] - EntityPathPoints[i]).Rotator() : GetPawn()->GetActorRotation(),
				ERTSEntities_NavigationType::NoFormation
			));

			// Next waypoint
			continue;
		}

		// Validate the current path point and the previous point 
		if(!EntityPathPoints.IsValidIndex(i-1) || !EntityPathPoints.IsValidIndex(i))
		{
			continue;
		}

		// Get the current segment distance (previous to current) so we can check if we need to add additional waypoints
		const float PathSegmentDistance = (EntityPathPoints[i-1] - EntityPathPoints[i]).Length();
		
		/** Determine if additional waypoints are required:
		 *  If the segment distance is greater than the max waypoint length
		 *  If its not longer than 2x the max length just add a new waypoint to the mid point
		 *  otherwise determine how many points are needed and add as required **/
		const float MaxWaypointLength = GetMaxWaypointLength();
		if (PathSegmentDistance > MaxWaypointLength * 2)
		{
			// Check how many new waypoints we need
			const int32 NumNewPoints = FMath::FloorToInt(PathSegmentDistance / MaxWaypointLength) - 1;
			FVector Direction = (EntityPathPoints[i] - EntityPathPoints[i-1]).GetSafeNormal();

			for (int32 j = 1; j < NumNewPoints; j++)
			{
				FVector NewPoint = EntityPathPoints[i-1] + Direction * (PathSegmentDistance / NumNewPoints) * j;
				//ValidateLocation(NewPoint);

				// If its greater add a waypoint for the validated location at the current waypoint distance
				URTSCore_SystemStatics::GetTerrainPosition(GetWorld(), NewPoint);
				NewWaypoints.Add(FRTSEntities_NavigationWaypoint(
					ERTSEntities_WaypointType::Nav,
					NewPoint,
					FRotationMatrix::MakeFromX(EntityPathPoints[i] - EntityPathPoints[i-1]).Rotator(),
					bHasFormedUp ? ERTSEntities_NavigationType::Formation : ERTSEntities_NavigationType::NoFormation
				));
			}
		}
		else if (PathSegmentDistance > MaxWaypointLength && PathSegmentDistance < MaxWaypointLength * 2) 
		{
			// If the waypoint is greater than the max but less then two times the max add a waypoint to the segment mid point
			FVector MidPoint = (EntityPathPoints[i-1] + EntityPathPoints[i]) * 0.5f;
			//ValidateLocation(MidPoint);

			URTSCore_SystemStatics::GetTerrainPosition(GetWorld(), MidPoint);
			NewWaypoints.Add(FRTSEntities_NavigationWaypoint(
				ERTSEntities_WaypointType::Nav,
				MidPoint,
				FRotationMatrix::MakeFromX(EntityPathPoints[i] - EntityPathPoints[i-1]).Rotator(),
				bHasFormedUp ? ERTSEntities_NavigationType::Formation : ERTSEntities_NavigationType::NoFormation
			));
		}

		// At this point we have either add any required extra waypoints or not had to add any
		// Check if the current point is the last point, if so add as destination waypoint
		if(i == EntityPathPoints.Num() - 1)
		{			
			URTSCore_SystemStatics::GetTerrainPosition(GetWorld(), EntityPathPoints[i]);
			NewWaypoints.Add(FRTSEntities_NavigationWaypoint(
				ERTSEntities_WaypointType::Destination,
				EntityPathPoints[i],
				EntityPosition.Rotation,
				bHasFormedUp ? ERTSEntities_NavigationType::Formation : ERTSEntities_NavigationType::NoFormation
			));
		}
		else
		{
			/** If the next nav point is less than the max waypoint distance add a waypoint at the next point
			 *  Check if we reached the form up point, set type to form up if so **/
			URTSCore_SystemStatics::GetTerrainPosition(GetWorld(), EntityPathPoints[i]);
			NewWaypoints.Add(FRTSEntities_NavigationWaypoint(
				(i == EntityPosition.FormUpIndex && bShouldUseFormUp) ? ERTSEntities_WaypointType::FormUp : ERTSEntities_WaypointType::Nav,
				EntityPathPoints[i],
				FRotationMatrix::MakeFromX(EntityPathPoints[i] - EntityPathPoints[i-1]).Rotator(),
				bHasFormedUp ? ERTSEntities_NavigationType::Formation : ERTSEntities_NavigationType::NoFormation
			));
		}
	}

	// Add corner sharpness values to waypoints
	AssignWaypointCornerSharpness(NewWaypoints);

	// Assign waypoints
	if(Waypoints.Num() > 0)
	{
		Waypoints.Append(NewWaypoints);
	}
	else
	{
		Waypoints = NewWaypoints;
	}
	
	HandleWaypointsUpdated();	
}

int32 ARTSEntities_AiControllerCommand::GetNextValidWaypointIndex()
{	
	// Check the current waypoint is not valid or the current is our destination
	if(!Waypoints.IsValidIndex(CurrentWaypointIndex) || Waypoints[CurrentWaypointIndex].WaypointType == ERTSEntities_WaypointType::Destination)
	{
		// Return invalid waypoint index
		return -1;
	}

	// Check if our current waypoint is greater than 50% of our max waypoint length, use current waypoint if it is
	if(HasEntityComponent())
	{			
		if(Waypoints[CurrentWaypointIndex].DistanceToCompletion > EntityComponent->GetMaxWaypointLength() * 0.5f)
		{
			return CurrentWaypointIndex;
		}
	}
	
	// Return the next waypoint if valid and not the destination
	if(Waypoints.IsValidIndex(CurrentWaypointIndex + 1) && Waypoints[CurrentWaypointIndex + 1].WaypointType != ERTSEntities_WaypointType::Destination)
	{
		return CurrentWaypointIndex + 1;
	}

	// If all of the above fail return current waypoint
	return CurrentWaypointIndex;
}

void ARTSEntities_AiControllerCommand::CreateSplinePath()
{
	if(GetPawn())
	{
		if(const IRTSCore_EntityInterface* EntityInterface = Cast<IRTSCore_EntityInterface>(GetPawn()))
		{
			NavigationSpline = EntityInterface->GetNavigationSpline();
			
			// Clear existing spline points
			if(NavigationSpline)
			{
				NavigationSpline->ClearSplinePoints();
				NavigationSpline->SetWorldLocation(GetPawn()->GetActorLocation());
				
				for (int i = 0; i < Waypoints.Num(); ++i)
				{
					NavigationSpline->AddSplineWorldPoint(Waypoints[i].Location);
				}
			}
		}		
	}
}

void ARTSEntities_AiControllerCommand::UpdateSplinePath(const int32 UpdateFromIndex)
{
	if(NavigationSpline)
	{
		// Remove points equal to or greater than the UpdateFromIndex
		for (int i = NavigationSpline->GetNumberOfSplinePoints() - 1; i >= 0; --i)
		{
			if(i >= UpdateFromIndex)
			{
				NavigationSpline->RemoveSplinePoint(i);
			}
		}

		// Recreate new spline points from the UpdateFromIndex
		for (int i = UpdateFromIndex; i < Waypoints.Num(); ++i)
		{
			NavigationSpline->AddSplineWorldPoint(Waypoints[i].Location);
		}
	}
}

void ARTSEntities_AiControllerCommand::AssignWaypointCornerSharpness(TArray<FRTSEntities_NavigationWaypoint>& NewWaypoints)
{
	if(GetPawn() && NavigationSpline)
	{
		for (int i = 0; i < NewWaypoints.Num() - 1; ++i)
		{
			if(NewWaypoints.IsValidIndex(i) && NewWaypoints.IsValidIndex(i+1))
			{
				FVector CurrentTangent = NavigationSpline->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::World);
				FVector NextTangent = NavigationSpline->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::World);

				// Normalize the tangents
				CurrentTangent.Normalize();
				NextTangent.Normalize();

				// Calculate the dot product to get the cosine of the angle
				const float DotProduct = FVector::DotProduct(CurrentTangent, NextTangent);

				// Calculate the angle in radians
				const float AngleRadians = FMath::Acos(DotProduct);

				// Convert the angle to degrees
				const float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

				NewWaypoints[i+1].CornerSharpness = AngleDegrees;
			}
		}
	}
}

void ARTSEntities_AiControllerCommand::SetCurrentWaypoint()
{	
	// Check we have a valid waypoint and it is not complete yet
	if(Waypoints.IsValidIndex(CurrentWaypointIndex) && !Waypoints[CurrentWaypointIndex].bIsCompleted)
	{
		return;
	}
	
	// Find the next uncompleted waypoint
	for (int i = 0; i < Waypoints.Num(); ++i)
	{
		// Check if waypoint is complete
		if(!Waypoints[i].bIsCompleted)
		{
			UpdateCurrentWaypointIndex(i);
			return;
		}			
	}

	// If no uncompleted waypoint found set to invalid waypoint and execute command complete
	UpdateCurrentWaypointIndex(-1);	
}

void ARTSEntities_AiControllerCommand::BeginNavigatingCurrentWaypoint()
{
	// Update entity navigation state
	BehaviourUpdate_NavigationState();
}

void ARTSEntities_AiControllerCommand::HandleWaypointNavigation()
{
	if(Waypoints.IsValidIndex(CurrentWaypointIndex))
	{
		// Update waypoint distance to completion
		Waypoints[CurrentWaypointIndex].UpdateDistanceToCompletion(GetPathFollowingComponent()->GetCurrentNavLocation());

		// Handle waypoint approach
		HandleWaypointApproach();
		
#if WITH_EDITOR
		Debug_Waypoints();		
#endif
	}	
}

void ARTSEntities_AiControllerCommand::ManageFormationPosition()
{
}

void ARTSEntities_AiControllerCommand::HandleWaypointApproach()
{
	if(Waypoints.IsValidIndex(CurrentWaypointIndex))
	{
		// Check this waypoint is not our destination
		if(Waypoints[CurrentWaypointIndex].WaypointType != ERTSEntities_WaypointType::Destination)
		{
			// Check we are at or under approach threshold
			if(IsApproachingCurrentWaypoint())
			{
				// Proceed to next waypoint
				SetCurrentWaypointComplete();
				SetCurrentWaypoint();
			}
		}
		else
		{
			if(HasReachedDestination())
			{					
				if(HandleArrivalAtDestination())
				{					
					HandleWaypointNavigationComplete();
				}
			}
		}
	}
}

bool ARTSEntities_AiControllerCommand::IsApproachingCurrentWaypoint() const
{	
	if(EntityComponent != nullptr)
	{
		return Waypoints.IsValidIndex(CurrentWaypointIndex) && EntityComponent->IsApproachingWaypoint(Waypoints[CurrentWaypointIndex].DistanceToCompletion);
	}	

	return false;
}

void ARTSEntities_AiControllerCommand::SetCurrentWaypointComplete()
{	
	if(Waypoints.IsValidIndex(CurrentWaypointIndex))
	{
		Waypoints[CurrentWaypointIndex].Completed();
	}	
}

bool ARTSEntities_AiControllerCommand::HasReachedDestination() const
{
	return GetPawn() && Waypoints.IsValidIndex(CurrentWaypointIndex) && (Waypoints[CurrentWaypointIndex].Location - GetPawn()->GetActorLocation()).Length() < GetAcceptanceDistance() && GetSpeed() < GetAcceptanceSpeed();
}

void ARTSEntities_AiControllerCommand::HandleWaypointNavigationComplete()
{
	if(HasEntityComponent())
	{
		SetCurrentWaypointComplete();		

		// Reset Entity Position
		EntityComponent->SetNavigationDestination(GetPawn()->GetActorLocation());
		EntityComponent->SetStanceState(ERTSCore_StanceState::Crouch);
		
		CompleteCurrentCommand(ERTSEntities_CommandStatus::Completed);
	}
}

void ARTSEntities_AiControllerCommand::ResetWaypoints()
{	
	Waypoints.Empty();
	HandleWaypointsUpdated();	
}

void ARTSEntities_AiControllerCommand::HandleWaypointsUpdated()
{
	// On client and server (can remove from client if no visualisation required)
	if(Waypoints.Num() > 0)
	{
		CreateSplinePath();
		SetDestination();
	}
	else
	{
		if(NavigationSpline)
		{
			NavigationSpline->ClearSplinePoints();
		}
	}

	// Update navigation state and current waypoint
	BehaviourUpdate_NavigationState();
	SetCurrentWaypoint();	
}

void ARTSEntities_AiControllerCommand::UpdateCurrentWaypointIndex(const int32 NewWaypointIndex)
{
	if(NewWaypointIndex == CurrentWaypointIndex)
	{
		return;
	}
	
	CurrentWaypointIndex = NewWaypointIndex;
	
	// Check if the current waypoint is invalid and the state is still set to navigating
	/*if(CurrentWaypointIndex == -1 && ActiveCommand != nullptr)
	{		
		// End waypoint nav
		if(ActiveCommand->EntityStatus != ERTSEntities_CommandStatus::Completed)
		{
			HandleWaypointNavigationComplete();
		}
	}*/
	
	if(CurrentWaypointIndex != -1)
	{
		// Begin waypoint nav
		BeginNavigatingCurrentWaypoint();
	}	
}

void ARTSEntities_AiControllerCommand::CreateTransitionWaypoint(const ERTSEntities_WaypointType& Type)
{
	if(NavigationSpline != nullptr)
	{
		const FVector NearestSplinePoint = NavigationSpline->FindLocationClosestToWorldLocation(GetPawn()->GetActorLocation(), ESplineCoordinateSpace::World);
		const float InputKey = NavigationSpline->FindInputKeyClosestToWorldLocation(NearestSplinePoint);
		const float SplineDistance = NavigationSpline->GetDistanceAlongSplineAtSplineInputKey(InputKey);
		const FVector NewPosition = NavigationSpline->GetLocationAtDistanceAlongSpline(SplineDistance + (GetAcceptanceDistance() * 2.f), ESplineCoordinateSpace::World);
		Waypoints.Reset();
		Waypoints.Add(
		FRTSEntities_NavigationWaypoint(
			Type,
			NewPosition,
			GetPawn()->GetActorRotation(),
			ERTSEntities_NavigationType::NoFormation
		));
		
		if(Waypoints.IsValidIndex(0))
		{
			UpdateCurrentWaypointIndex(0);
		}
	}
}

FVector ARTSEntities_AiControllerCommand::GetAgentExtent() const
{
	if(GetPawn())
	{
		return GetPawn()->GetComponentsBoundingBox().GetExtent();
	}
	
	return FVector::ZeroVector;
}

float ARTSEntities_AiControllerCommand::GetMaxWaypointLength()
{
	return 500.f;
}

float ARTSEntities_AiControllerCommand::GetSpeed() const
{
	return GetPawn() && FMath::Abs(GetPawn()->GetVelocity().Length()) > 10.f ? FMath::Abs(GetPawn()->GetVelocity().Length()) : 0.f;
}

float ARTSEntities_AiControllerCommand::GetMaxSpeed() const
{
	return 400.f;
}

float ARTSEntities_AiControllerCommand::GetAcceptanceSpeed() const
{
	return 5.f;
}

float ARTSEntities_AiControllerCommand::GetAcceptanceDistance() const
{
	return 20.f;
}

void ARTSEntities_AiControllerCommand::Debug_Waypoints()
{
	if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(CoreSettings->DebugRTSEntities && CoreSettings->DebugEntityNavigation && GetWorld())
		{
			for (int i = 0; i < Waypoints.Num(); ++i)
			{
				if(Waypoints[i].WaypointType == ERTSEntities_WaypointType::Start)
				{
					DrawDebugSphere(GetWorld(), Waypoints[i].Location, 20.f, 8, FColor::Blue, false, -1.f, 0, 5.f);
				}
				else if(Waypoints[i].WaypointType == ERTSEntities_WaypointType::FormUp)
				{
					DrawDebugSphere(GetWorld(), Waypoints[i].Location, 20.f, 8, FColor::Orange, false, -1.f, 0, 5.f);
				}	
				else if(Waypoints[i].WaypointType == ERTSEntities_WaypointType::Nav)
				{
					DrawDebugSphere(GetWorld(), Waypoints[i].Location, 20.f, 8, FColor::Green, false, -1.f, 0, 5.f);
				}
				if(Waypoints[i].WaypointType == ERTSEntities_WaypointType::Destination) 
				{
					DrawDebugSphere(GetWorld(), Waypoints[i].Location, 20.f, 8, FColor::Cyan, false, -1.f, 0, 2.f);
				}
			}
		}
	}
}

void ARTSEntities_AiControllerCommand::Debug_Formation(FVector EntityLocation, FVector SubordinateLocation,	const bool bSubordinateInPosition) const
{
	if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(CoreSettings->DebugRTSEntities && CoreSettings->DebugEntityFormations && GetWorld() && EntityPosition.IsValid())
		{
			/*FColor Color;
			switch (NavData[NavIndex].FormationPosition.Index)
			{
				case 0: Color = FColor::Blue; break;
				case 1: Color = FColor::Green; break;
				case 2: Color = FColor::Cyan; break;
				case 3: Color = FColor::Red; break;
				default: Color = FColor::Black;
			}
	
			const FVector MemberPos = NavData[NavIndex].Entity->GetActorLocation();
			DrawDebugSphere(GetWorld(), MemberPos, 50.f, 8, Color, false, -1.f, 0, 2.f);*/

			EntityLocation.Z += 100.f;
			SubordinateLocation.Z += 100.f;
		
			if(bSubordinateInPosition)
			{
				// Subordinate in Position			
				DrawDebugDirectionalArrow(GetWorld(), EntityLocation, SubordinateLocation, 1000.f, FColor::Green, false, -1.f, 0, 4.f);
			}
			else
			{
				// Subordinate is out of Position
				DrawDebugDirectionalArrow(GetWorld(), EntityLocation, SubordinateLocation, 1000.f, FColor::Red, false, -1.f, 0, 4.f);
			}
		}
	}
}
