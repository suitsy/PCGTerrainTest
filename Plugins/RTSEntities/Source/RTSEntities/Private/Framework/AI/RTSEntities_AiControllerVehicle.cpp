// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AI/RTSEntities_AiControllerVehicle.h"

#include "CommandSystem/RTSEntities_Command.h"
#include "Components/SplineComponent.h"
#include "Framework/Entities/Components/RTSEntities_Entity.h"
#include "Framework/Components/RogueRVO_VehicleMovementComponent.h"
#include "Framework/Interfaces/RTSCore_VehicleInputInterface.h"
#include "Framework/Settings/RTSCore_DeveloperSettings.h"


ARTSEntities_AiControllerVehicle::ARTSEntities_AiControllerVehicle(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	CurrentNavSplineDistance = -1.f;
	MaxAcceleration = 10.f;
	Mass = 1.f;
}

void ARTSEntities_AiControllerVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(IsNavigating() && HasAuthority() && IsActive())
	{
		// Update minimum spline position (prevent coming back onto curve earlier than we were)
		const FVector NavPoint = GetPathNavLocation();
		const float DistanceOnSpline = NavigationSpline->GetDistanceAlongSplineAtLocation(NavPoint, ESplineCoordinateSpace::World);	
		if(DistanceOnSpline > CurrentNavSplineDistance)
		{
			CurrentNavSplineDistance = DistanceOnSpline;
		}	
	}

#if WITH_EDITOR
	
#endif
}

void ARTSEntities_AiControllerVehicle::BeginPlay()
{
	Super::BeginPlay();

	
}

void ARTSEntities_AiControllerVehicle::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Apply handbrake on start
	if(HasAuthority() && GetPawn())
	{
		if(IRTSCore_VehicleInputInterface* VehicleInputInterface = Cast<IRTSCore_VehicleInputInterface>(GetPawn()))
		{
			VehicleInputInterface->SetParked(true);
			Mass = VehicleInputInterface->GetMass();
		}
	}
}

bool ARTSEntities_AiControllerVehicle::IsNavigating() const
{
	return NavigationSpline != nullptr && Waypoints.IsValidIndex(CurrentWaypointIndex);
}

void ARTSEntities_AiControllerVehicle::BeginNavigatingCurrentWaypoint()
{
	if(Waypoints.IsValidIndex(CurrentWaypointIndex) && GetPawn() && GetPawn()->HasAuthority())
	{			
		Super::BeginNavigatingCurrentWaypoint();
		
		if(IRTSCore_VehicleInputInterface* VehicleInputInterface = Cast<IRTSCore_VehicleInputInterface>(GetPawn()))
		{
			// Reset brake input controls
			VehicleInputInterface->SetParked(false);
			CurrentNavSplineDistance = 0.f;

			ThrottleCurve = VehicleInputInterface->GetThrottleCurve();
			SteeringCurve = VehicleInputInterface->GetSteeringCurve();
			BrakeCurve = VehicleInputInterface->GetBrakingCurve();
			SteeringDampeningCurve = VehicleInputInterface->GetSteeringDampeningCurve();
			CollisionSteeringCurve = VehicleInputInterface->GetCollisionSteeringCurve();
			CollisionBrakeCurve = VehicleInputInterface->GetCollisionBrakeCurve();
		}
	}
}

void ARTSEntities_AiControllerVehicle::HandleWaypointNavigation()
{
	Super::HandleWaypointNavigation();
	
	if(HasAuthority() && Waypoints.IsValidIndex(CurrentWaypointIndex) && NavigationSpline != nullptr)
	{
		// Handle waypoint approach
		HandleWaypointApproach();

		// Update Spline Navigation
		if(NavigationSpline != nullptr && NavigationSpline->GetNumberOfSplinePoints() > 0)
		{
			// Create or reset vehicle input data struct for this tick
			VehicleInput = FRTSCore_VehicleInput();
			
			// Manage throttle, braking and steering
			HandleVehicleNavigation();
			
			// Manage entities position in formation
			ManageFormationPositionVehicle();

			// Apply vehicle input to vehicle interface
			ApplyVehicleInput();
		}
	}
}

void ARTSEntities_AiControllerVehicle::ManageFormationPositionVehicle()
{
	/*if(!NavigationIsValid())
	{
		return;
	}

	// Get entity component reference
	URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(EntityNavigationData.NavActor);
	const int32 EntityIndex = EntityNavigationData.FormationPosition.FormationIndex;
	const FVector EntityLocation = EntityNavigationData.NavActor->GetActorLocation();
	
	// Check we are using a formation or are not almost at our destination
	if(!Waypoints.IsValidIndex(CurrentWaypointIndex) || !Waypoints[CurrentWaypointIndex].UseFormation()
		|| Waypoints[CurrentWaypointIndex].WaypointType == ERTSEntities_WaypointType::Destination && (GetPawn()->GetActorLocation() - Waypoints[CurrentWaypointIndex].Location).Length() < 500.f)
	{
		if(Entity)
		{
			Entity->ResetMaxSpeed();
		}

		return;
	}	

	// Check if the entity is the lead position, they may have two subordinates in some formations
	int32 SubordinateIndex = -1, SubordinateIndexAlt = -1;
	if(EntityIndex == 0)
	{
		switch (Entity->GetFormationType())
		{
			case EFormationType::Wedge:
			case EFormationType::Line:
				SubordinateIndexAlt = EntityIndex + 1;
				break;				
			default: ;
		}
	}

	// Check for entity subordinate
	switch (Entity->GetFormationType())
	{			
		case EFormationType::Wedge:
		case EFormationType::Line:
			SubordinateIndex = EntityIndex + 2;
			break;
		case EFormationType::EchelonRight:
		case EFormationType::EchelonLeft:
		case EFormationType::Column:
			SubordinateIndex = EntityIndex + 1;
			break;			
		default: ;
	}

	const float Spacing = Entity->GetSpacing();
	//const float MinTolerance = Spacing - (Spacing * (1 + Entity->GetFormationSpacingTolerance()));	
	const float MaxTolerance = Spacing + (Spacing * (1 + Entity->GetFormationSpacingTolerance())); 

	for (int32 i = 0; i < GroupNavigationData.Num(); ++i)
	{
		// Check if the nav data is for one of our subordinates
		if(GroupNavigationData[i].FormationPosition.FormationIndex == SubordinateIndex || GroupNavigationData[i].FormationPosition.FormationIndex == SubordinateIndexAlt)
		{
			const AActor* Subordinate = GroupNavigationData[i].NavActor;
			const FVector SubordinateLocation = Subordinate ? Subordinate->GetActorLocation() : FVector::ZeroVector;
			if(!Subordinate || SubordinateLocation == FVector::ZeroVector)
			{
				continue;
			}

			const FVector EntityToWaypoint = (Waypoints[CurrentWaypointIndex].Location - EntityLocation).GetSafeNormal();
			const FVector EntityToSubordinate = (SubordinateLocation - EntityLocation).GetSafeNormal();
			const float DotProduct = FVector::DotProduct(EntityToWaypoint, EntityToSubordinate);
			const FVector2D Angle = Entity->GetFormationSubordinateRearAngle();
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
			
			if(bSubordinateBehind)
			{
				// If subordinate is behind and lagging behind	
				if(Distance > MaxTolerance)
				{
					// Apply formation control if brake amount is greater than any current braking
					const float CurvedBrakeAmount = BrakeCurve != nullptr ? BrakeCurve->GetFloatValue(AdjustSpeedFactor) : 0.25f;
					if(VehicleInput.Braking < CurvedBrakeAmount)
					{
						VehicleInput.Braking = CurvedBrakeAmount;
					}					
				}							
			}
			
			if(bSubordinateForward)
			{
				// Inform subordinate to reduce speed
				if(IRTSCore_VehicleInputInterface* SubordinateVehicleInputInterface = Cast<IRTSCore_VehicleInputInterface>(GroupNavigationData[i].NavActor))
				{
					const float CurvedBrakeAmount = BrakeCurve != nullptr ? BrakeCurve->GetFloatValue(AdjustSpeedFactor) : 0.25f;						
					if(SubordinateVehicleInputInterface->GetBrake() < CurvedBrakeAmount)
					{
						SubordinateVehicleInputInterface->SetBrake(CurvedBrakeAmount);
					}
				}										
			}								
		}					
	}*/
}

void ARTSEntities_AiControllerVehicle::HandleWaypointNavigationComplete()
{
	if(GetPawn())
	{
		if(GetSpeed() < GetAcceptanceSpeed())
		{
			Super::HandleWaypointNavigationComplete();
		}			
	}
}

bool ARTSEntities_AiControllerVehicle::HandleArrivalAtDestination() const
{
	if(GetPawn() && ActiveCommand != nullptr)
	{			
		if(IRTSCore_VehicleInputInterface* VehicleInputInterface = Cast<IRTSCore_VehicleInputInterface>(GetPawn()))
		{
			// Get delta rotation required to turn to desired destination rotation
			FRotator DeltaRotation = ActiveCommand->GetTargetRotation() - GetPawn()->GetActorRotation();
			DeltaRotation.Normalize();

			VehicleInputInterface->SetThrottle(0.f);			
			if(FMath::Abs(DeltaRotation.Yaw) > 4.f)
			{
				const bool bTurnIsRight =  DeltaRotation.Yaw > 0.f;

				// Calculate the steering amount based on the delta rotation
				//const float MaxSteeringChange = GetMaxTurnRate() * GetWorld()->GetDeltaSeconds();
				const float SteeringAmount = FMath::GetMappedRangeValueClamped(FVector2D(-180.f, 180.f), FVector2D(-1.f, 1.f), DeltaRotation.Yaw);

				// Limit the steering change based on the maximum turn rate
				//SteeringAmount = FMath::Clamp(SteeringAmount, -MaxSteeringChange, MaxSteeringChange);
			
				const float SteeringOrientationScale = SteeringCurve ? SteeringCurve->GetFloatValue(FMath::Abs(SteeringAmount)) : FMath::Abs(SteeringAmount);
				const float Steering = SteeringOrientationScale * GetWorld()->GetDeltaSeconds();
				VehicleInputInterface->SetSteering(bTurnIsRight ? Steering : -Steering);				
			}
			else
			{
				VehicleInputInterface->SetParked(true);	
				return true;					
			}
		}			
	}

	return false;
}

void ARTSEntities_AiControllerVehicle::HandleVehicleNavigation()
{
	if(GetPawn() && Waypoints.IsValidIndex(CurrentWaypointIndex))
	{
		if(const IRTSCore_VehicleInputInterface* VehicleInputInterface = Cast<IRTSCore_VehicleInputInterface>(GetPawn()))
		{		
			// Calculate desired velocity based on path following
			const FVector DesiredVelocity = CalculateDesiredVelocity();

			// Calculate avoidance velocity if there's a collision threat
			FVector AvoidanceVelocity = FVector::ZeroVector;
			const float DistToDest = (GetPawn()->GetActorLocation() - Waypoints[Waypoints.Num() - 1].Location).Length();
			const bool bApproachingDestination = Waypoints[CurrentWaypointIndex].WaypointType == ERTSEntities_WaypointType::Destination;
		
			if(!bApproachingDestination && DistToDest > GetArrivalDistance())
			{
				AvoidanceVelocity = GetAvoidanceVelocity();
			}

			// Combine desired and avoidance velocities
			const FVector FinalVelocity = DesiredVelocity + AvoidanceVelocity;
		
			// Clamp the final desired velocity to the max velocity force
			Truncate(FinalVelocity);

#if WITH_EDITOR
			Debug_Avoidance(FinalVelocity);
#endif

			// Calculate the dot product of the desired velocity with the vehicle's forward vector
			const float Forward = FVector::DotProduct(FinalVelocity.GetSafeNormal(), GetPawn()->GetActorForwardVector());
			
			VehicleInput.Throttle = Forward < 0.f ? 0.f : Forward * 0.5f;
		
			// Calculate the right component by taking the dot product with the right vector
			const float Right = FVector::DotProduct(FinalVelocity.GetSafeNormal(), GetPawn()->GetActorRightVector());
			const float BrakeAmount = VehicleInputInterface->GetBrakeInput();
			const float MaxSteeringChange = GetMaxTurnRate() * GetWorld()->GetDeltaSeconds();
			const float SteeringAmount = FMath::Clamp(Right, -MaxSteeringChange, MaxSteeringChange);
			VehicleInput.Steering = SteeringAmount * (1 + BrakeAmount);

			// Approaching destination
			if(bApproachingDestination && DistToDest < GetArrivalDistance())
			{
				// Approaching destination
				if(GetSpeed() < 50.f && DistToDest >= GetAcceptanceDistance())
				{
					VehicleInput.Braking = 0.f;
				}
				else
				{		
					const float DestinationBrakeAmount = FMath::GetMappedRangeValueClamped(FVector2d(0, GetArrivalDistance() + GetMaxSpeed()), FVector2d(0.f, GetMaxDestinationBrake()), DistToDest + GetSpeed());
					VehicleInput.Braking = BrakeCurve != nullptr ? BrakeCurve->GetFloatValue(DestinationBrakeAmount) : DestinationBrakeAmount;
				}
			}
			else
			{
				// Check we are on the path
				if((GetClosestPathLocation() - GetPawn()->GetActorLocation()).Length() < 400.f)
				{
					const int32 NextPointIndex = NavigationSpline->FindInputKeyClosestToWorldLocation(GetPawn()->GetActorLocation()) + 1;
					if(Waypoints.IsValidIndex(NextPointIndex) && Waypoints[NextPointIndex].CornerSharpness > 10.f)
					{
						// Handle braking for path cornering when on path
						const float SpeedRatio = GetSpeed() / GetMaxSpeed();
						const float TurnRatio = FMath::Clamp(Waypoints[CurrentWaypointIndex].CornerSharpness / 180.f, 0.f, 1.f);
						const float BrakeRatio = FMath::GetMappedRangeValueClamped(FVector2d(0.f, 2.f), FVector2d(0.f, 1.f), SpeedRatio + TurnRatio);
						const float CornerBrakeAmount = SteeringDampeningCurve ? SteeringDampeningCurve->GetFloatValue(BrakeRatio) : 0.f;
						UE_LOG(LogTemp, Log, TEXT("%f"), CornerBrakeAmount);
						VehicleInput.Braking += CornerBrakeAmount;						
					}
				}
				else
				{
					// Limit speed to 50% if off path
					if(GetSpeed() > GetMaxSpeed() * 0.5f)
					{
						VehicleInput.Braking += 0.1f;
					}
				}
			}

			// Check for direct collision
			/*float HitRange = 0.f;
			if(SweepForCollision(HitRange))
			{
				// Get the vehicles velocity
				const float VelSize = GetPawn()->GetVelocity().Size();

				// If the vehicle has stopped
				if(VelSize <= 50.f || bHasCollision)
				{
					// Reverse out of collision
					if(!GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_Collision))
					{
						bHasCollision = true;
						GetWorld()->GetTimerManager().SetTimer(TimerHandle_Collision, this, &ThisClass::CollisionDetectionTimer, 4.f, false);
					}
				
					VehicleInput.Throttle = 0.f;
					VehicleInput.Braking = Forward;
				
				}
				else
				{				
					const float CollisionBrakeAmount = FMath::GetMappedRangeValueClamped(FVector2d(0, HitRange), FVector2d(0.f, 1.f), HitRange);
					VehicleInput.Braking = CollisionBrakeCurve != nullptr ? CollisionBrakeCurve->GetFloatValue(CollisionBrakeAmount) : CollisionBrakeAmount;
					VehicleInput.Throttle = 0.f;
				}
			}*/
		}
	}
}

FVector ARTSEntities_AiControllerVehicle::CalculateDesiredVelocity()
{
	if(!GetPawn())
	{
		return FVector::ZeroVector;
	}
	
	// Get the desired target location
	const FVector DesiredLocation = GetPathNavLocation();

	// Calculate the direction from the vehicle to the target position
	const FVector TargetDirection = (DesiredLocation - GetPawn()->GetActorLocation()).GetSafeNormal();

	// Calculate the desired velocity based on the angle (you can adjust this based on your requirements)
	return TargetDirection * GetMaxSpeed();
}

FVector ARTSEntities_AiControllerVehicle::GetAvoidanceVelocity()
{
	if(GetPawn())
	{
		if(const IRTSCore_VehicleInputInterface* VehicleInputInterface = Cast<IRTSCore_VehicleInputInterface>(GetPawn()))
		{
			return VehicleInputInterface->GetRVOAvoidanceVelocity();
		}
	}

	return FVector::ZeroVector;
}

FVector ARTSEntities_AiControllerVehicle::Truncate(const FVector& VectorToTruncate)
{
	const float MaxMagnitude = Mass * MaxAcceleration;
	if (VectorToTruncate.SizeSquared() > FMath::Square(MaxMagnitude))
	{
		return VectorToTruncate.GetSafeNormal() * MaxMagnitude;
	}
	return VectorToTruncate;
}

void ARTSEntities_AiControllerVehicle::ApplyVehicleInput() const
{
	if(GetPawn())
	{
		if(IRTSCore_VehicleInputInterface* VehicleInputInterface = Cast<IRTSCore_VehicleInputInterface>(GetPawn()))
		{
			VehicleInputInterface->ApplyVehicleInput(VehicleInput);
		}
	}
}

FVector ARTSEntities_AiControllerVehicle::GetClosestPathLocation() const
{
	if(GetPawn() && NavigationSpline)
	{		
		return NavigationSpline->FindLocationClosestToWorldLocation(GetPawn()->GetActorLocation(), ESplineCoordinateSpace::World);
	}

	return FVector::ZeroVector;
}

FVector ARTSEntities_AiControllerVehicle::GetPathNavLocation() const
{
	if(GetPawn() && NavigationSpline)
	{
		FVector ClosestTangent = NavigationSpline->FindTangentClosestToWorldLocation(GetPawn()->GetActorLocation(), ESplineCoordinateSpace::World);
		ClosestTangent.Normalize(0.0001f);
		const FVector ClosestPoint = GetClosestPathLocation();
		const float DistanceToSpline = (ClosestPoint - GetPawn()->GetActorLocation()).Length();
		const FVector NavPoint = NavigationSpline->FindLocationClosestToWorldLocation(ClosestPoint + (ClosestTangent * FMath::Clamp(DistanceToSpline, 700.f, 2000.f)), ESplineCoordinateSpace::World);
		return NavPoint;
	}

	return FVector::ZeroVector;	
}

FRotator ARTSEntities_AiControllerVehicle::GetRotationToNavPoint() const
{
	if(GetPawn())
	{
		// Get path closest point
		const FVector ClosestPoint = GetPathNavLocation();

		// Return delta rotation to closest point if valid 
		if(ClosestPoint != FVector::ZeroVector)
		{
			const FRotator LookAtRotation = FRotationMatrix::MakeFromX(ClosestPoint - GetPawn()->GetActorLocation()).Rotator();
			FRotator DeltaRotation = LookAtRotation - GetPawn()->GetActorRotation();
			DeltaRotation.Normalize();
			return DeltaRotation;
		}
	}

	return FRotator::ZeroRotator;
}

float ARTSEntities_AiControllerVehicle::GetMaxWaypointLength()
{
	if(GetPawn())
	{
		if(const URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(GetPawn()))
		{
			return Entity->GetMaxWaypointLength();			
		}
	}

	return 500.f;
}

float ARTSEntities_AiControllerVehicle::GetAcceptanceSpeed() const
{
	if(GetPawn())
	{
		if(const URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(GetPawn()))
		{
			return Entity->GetAcceptanceSpeed();
		}
	}

	return 10.f;
}

float ARTSEntities_AiControllerVehicle::GetAcceptanceDistance() const
{
	if(GetPawn())
	{
		if(const URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(GetPawn()))
		{
			return Entity->GetAcceptanceRadius();
		}
	}

	return 50.f;
}

float ARTSEntities_AiControllerVehicle::GetArrivalDistance() const
{
	if(GetPawn())
	{
		if(const URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(GetPawn()))
		{
			return Entity->GetArrivalDistance();
		}
	}

	return 50.f;
}

float ARTSEntities_AiControllerVehicle::GetMaxTurnRate() const
{
	if(GetPawn())
	{
		if(const URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(GetPawn()))
		{
			return Entity->GetMaxTurnRate();
		}
	}

	return 10.f;
}

float ARTSEntities_AiControllerVehicle::GetMaxDestinationBrake() const
{
	if(GetPawn())
	{
		if(const URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(GetPawn()))
		{
			return Entity->GetMaxDestinationBrake();
		}
	}

	return 0.25f;
}

float ARTSEntities_AiControllerVehicle::GetMaxSpeed() const
{
	if(GetPawn())
	{
		if(const URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(GetPawn()))
		{
			return Entity->GetDefaultMaxSpeed();
		}
	}

	return 50.f;
}

#if WITH_EDITOR

void ARTSEntities_AiControllerVehicle::Debug_Avoidance(const FVector& FinalVelocity) const
{
	if(const URTSCore_DeveloperSettings* DeveloperSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(DeveloperSettings->DebugRTSEntities && DeveloperSettings->DebugAiRVO)
		{			
			const FVector PawnLocation = GetPawn()->GetActorLocation();
			DrawDebugDirectionalArrow(GetWorld(), PawnLocation, PawnLocation + FinalVelocity, 400.f, FColor::Red, false, 0.1f, 0, 4.f);
		}
	}
}

#endif
