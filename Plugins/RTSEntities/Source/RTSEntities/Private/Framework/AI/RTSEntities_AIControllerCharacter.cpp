// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AI/RTSEntities_AIControllerCharacter.h"
#include "Framework/Entities/Components/RTSEntities_Entity.h"
#include "Framework/Data/RTSEntities_GroupDataAsset.h"
#include "Navigation/CrowdFollowingComponent.h"


ARTSEntities_AIControllerCharacter::ARTSEntities_AIControllerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void ARTSEntities_AIControllerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ARTSEntities_AIControllerCharacter::IsNavigating() const
{
	return GetPathFollowingComponent() && GetPathFollowingComponent()->GetStatus() != EPathFollowingStatus::Idle;	
}

void ARTSEntities_AIControllerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ARTSEntities_AIControllerCharacter::BeginNavigatingCurrentWaypoint()
{
	if(Waypoints.IsValidIndex(CurrentWaypointIndex))
	{
		// Acceptance radius needs to be very small as we will handle waypoint completion ourselves
		// We essentially never want to arrive through this functions has arrived events
		MoveToLocation(Waypoints[CurrentWaypointIndex].Location, 1.f, false);		
	}

	Super::BeginNavigatingCurrentWaypoint();
}

void ARTSEntities_AIControllerCharacter::HandleWaypointNavigation()
{
	Super::HandleWaypointNavigation();

	if(HasAuthority() && Waypoints.IsValidIndex(CurrentWaypointIndex))
	{
		
	}
}

void ARTSEntities_AIControllerCharacter::ManageFormationPosition()
{	
	/*if(!NavigationIsValid())
	{
		return;
	}

	// Get entity component reference
	URTSEntities_Entity* EntityComponent = URTSEntities_Entity::FindEntityComponent(EntityNavigationData.NavActor);
	const int32 FormationIndex = EntityNavigationData.FormationPosition.FormationIndex;
	const FVector EntityLocation = EntityNavigationData.NavActor->GetActorLocation();
	
	// Check we have a valid waypoint and are not using a formation or are not almost at our destination
	if(!Waypoints.IsValidIndex(CurrentWaypointIndex)
		|| !Waypoints[CurrentWaypointIndex].UseFormation()
		|| Waypoints[CurrentWaypointIndex].WaypointType == ERTSEntities_WaypointType::Destination
		&& Waypoints[CurrentWaypointIndex].DistanceToCompletion < 500.f)
	{
		// Reset to max speed 
		if(EntityComponent)
		{
			EntityComponent->ResetMaxSpeed();
		}

		return;
	}	

	// Check if the entity is the lead position, they may have two subordinates in some formations
	int32 SubordinateIndex = -1, SubordinateIndexAlt = -1;
	if(FormationIndex == 0)
	{
		switch (EntityComponent->GetFormationType())
		{
			case EFormationType::Wedge:
			case EFormationType::Line:
				SubordinateIndexAlt = FormationIndex + 1;
				break;				
			default: ;
		}
	}

	// Check for entity subordinate
	switch (EntityComponent->GetFormationType())
	{			
		case EFormationType::Wedge:
		case EFormationType::Line:
			SubordinateIndex = FormationIndex + 2;
			break;
		case EFormationType::EchelonRight:
		case EFormationType::EchelonLeft:
		case EFormationType::Column:
			SubordinateIndex = FormationIndex + 1;
			break;			
		default: ;
	}

	const float Spacing = EntityComponent->GetSpacing();
	const float MinTolerance = Spacing - (Spacing * (1 + EntityComponent->GetFormationSpacingTolerance()));	
	const float MaxTolerance = Spacing + (Spacing * (1 + EntityComponent->GetFormationSpacingTolerance())); 

	for (int32 i = 0; i < GroupNavigationData.Num(); ++i)
	{
		// Check if the nav data is for one of our subordinates
		if(GroupNavigationData[i].FormationPosition.FormationIndex == SubordinateIndex || GroupNavigationData[i].FormationPosition.FormationIndex == SubordinateIndexAlt)
		{
			const AActor* Subordinate = GroupNavigationData[i].NavActor;
			const FVector SubordinateLocation = Subordinate ? Subordinate->GetActorLocation() : FVector::ZeroVector;
			if(SubordinateLocation == FVector::ZeroVector)
			{
				continue;
			}

			const FVector EntityToWaypoint = (Waypoints[CurrentWaypointIndex].Location - EntityLocation).GetSafeNormal();
			const FVector EntityToSubordinate = (SubordinateLocation - EntityLocation).GetSafeNormal();
			const float DotProduct = FVector::DotProduct(EntityToWaypoint, EntityToSubordinate);
			const FVector2D Angle = EntityComponent->GetFormationSubordinateRearAngle();
			const bool bSubordinateInPosition = DotProduct <= Angle.X && DotProduct >= Angle.Y;
			const bool bSubordinateForward = DotProduct > Angle.X;
			const bool bSubordinateBehind = DotProduct < Angle.Y;
			
#if WITH_EDITOR
			Debug_Formation(EntityLocation, SubordinateLocation, bSubordinateInPosition);
#endif
			
			// Calculate a distance based speed change factor
			const float Distance = (EntityLocation - SubordinateLocation).Length();
			const float DistanceFactor = FMath::Clamp(Distance / MaxTolerance, 0.0f, 1.0f);
			const float AngleFacter = FMath::Clamp((DotProduct - 1.f) / (Angle.Y - Angle.X), 0.f, 1.f);
			const float AdjustSpeedFactor = FMath::Lerp(DistanceFactor, AngleFacter, 0.5f);
			
			if(bSubordinateInPosition)
			{
				// If subordinate is behind and lagging behind	
				if(Distance > MaxTolerance)
				{
					// Slow the forward entity					
					EntityComponent->SetMaxSpeed(EntityComponent->GetDefaultMaxSpeed() * AdjustSpeedFactor);
				}
				else
				{
					// Maintain normal speed
					EntityComponent->SetMaxSpeed(EntityComponent->GetDefaultMaxSpeed() * 1.f - AdjustSpeedFactor);
				}
				
				// Maintain normal subordinate speed
				if(URTSEntities_Entity* SubordinateEntity = URTSEntities_Entity::FindEntityComponent(Subordinate))
				{
					SubordinateEntity->SetMaxSpeed(EntityComponent->GetDefaultMaxSpeed() * 1.f - AdjustSpeedFactor);
				}							
			}
			else
			{
				if(bSubordinateForward)
				{
					// Slow the subordinate until they get behind
					if(URTSEntities_Entity* SubordinateEntity = URTSEntities_Entity::FindEntityComponent(Subordinate))
					{
						SubordinateEntity->SetMaxSpeed(EntityComponent->GetDefaultMaxSpeed() * AdjustSpeedFactor);
					}
				}

				if(bSubordinateBehind)
				{
					// Slow the forward entity					
					EntityComponent->SetMaxSpeed(EntityComponent->GetDefaultMaxSpeed() * AdjustSpeedFactor);
				}				
			}						
		}					
	}*/	
}

void ARTSEntities_AIControllerCharacter::HandleWaypointApproach()
{
	if(!NavigationIsValid() || !Waypoints.IsValidIndex(CurrentWaypointIndex))
	{
		return;
	}

	// Check we are at or under approach threshold
	if(IsApproachingCurrentWaypoint())
	{
		// Check this waypoint is not our destination
		if(Waypoints[CurrentWaypointIndex].WaypointType != ERTSEntities_WaypointType::Destination)
		{
			// Proceed to next waypoint
			SetCurrentWaypointComplete();
			SetCurrentWaypoint();
		}
		else
		{
			// Check we are fully at the destination
			if(HasReachedDestination())
			{
				SetCurrentWaypointComplete();
				HandleWaypointNavigationComplete();
			}
		}
	}
}

bool ARTSEntities_AIControllerCharacter::HandleArrivalAtDestination() const
{
	if(GetPawn())
	{		
		if(EntityPosition.IsValid())
		{
			const FRotator CurrentRot = FMath::RInterpConstantTo(GetPawn()->GetActorRotation(), EntityPosition.Rotation, GetWorld()->GetDeltaSeconds(), 2.f);
			GetPawn()->SetActorRotation(CurrentRot);
		
			FRotator DeltaRotation = EntityPosition.Rotation - GetPawn()->GetActorRotation();
			DeltaRotation.Normalize();
			if(FMath::Abs(DeltaRotation.Yaw) < 0.5f)
			{
				return true;
			}			
		}		
	}

	return false;
}

void ARTSEntities_AIControllerCharacter::HandleWaypointNavigationComplete()
{
	if(GetPathFollowingComponent())
	{
		if(GetSpeed() < GetAcceptanceSpeed())
		{
			GetPathFollowingComponent()->OnPathFinished(EPathFollowingResult::Success, FPathFollowingResultFlags::None);
			Super::HandleWaypointNavigationComplete();
		}
	}	
}

float ARTSEntities_AIControllerCharacter::GetMaxWaypointLength()
{
	if(GetPawn())
	{
		if(const URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(GetPawn()))
		{
			if(const URTSEntities_GroupDataAsset* GroupData = Entity->GetEntityGroupData())
			{
				return GroupData->MaxWaypointLength;
			}
		}
	}

	return 500.f;
}

bool ARTSEntities_AIControllerCharacter::HasReachedDestination() const
{
	if(GetPawn())
	{
		if(URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(GetPawn()))
		{
			return Waypoints.IsValidIndex(CurrentWaypointIndex) && Entity->HasReachedDestination(Waypoints[CurrentWaypointIndex].DistanceToCompletion);
		}
	}

	return false;
}
