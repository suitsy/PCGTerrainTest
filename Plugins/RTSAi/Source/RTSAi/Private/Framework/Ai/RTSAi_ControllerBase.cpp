// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Ai/RTSAi_ControllerBase.h"


ARTSAi_ControllerBase::ARTSAi_ControllerBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	//bReplicates = true;
	/*bAiInitialised = false;
	BehaviourTreeAsset = nullptr;
	SightConfig = nullptr;
	Group = nullptr;
	NavSystem = nullptr;
	NavIndex = -1;
	bReverse = false;
	
	SetupPerceptionSystem();*/
}

/*void ARTSAi_ControllerBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, State);
	DOREPLIFETIME(ThisClass, LastSeenTargetTime);
	DOREPLIFETIME(ThisClass, NavData);
	DOREPLIFETIME(ThisClass, NavIndex);	
	DOREPLIFETIME(ThisClass, Waypoints);
	DOREPLIFETIME(ThisClass, CurrentWaypointIndex);
}


void ARTSAi_ControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if(GetBlackboardComponent() && bAiInitialised)
	{
		// Check if behaviour state has changed
		if(State.GetState(ERTSAi_StateCategory::Behaviour) != GetBlackboardComponent()->GetValueAsInt(RTS_DATA_AI_BLACKBOARD_VALUE_BEHAVESTATE))
		{
			// Update blackboard if value changed
			GetBlackboardComponent()->SetValueAsInt(RTS_DATA_AI_BLACKBOARD_VALUE_BEHAVESTATE, State.GetState(ERTSAi_StateCategory::Behaviour));
		}
		
		// Check if speed state has changed
		if(State.GetState(ERTSAi_StateCategory::Speed) != GetBlackboardComponent()->GetValueAsInt(RTS_DATA_AI_BLACKBOARD_VALUE_SPEEDSTATE))
		{
			// Update blackboard if value changed
			GetBlackboardComponent()->SetValueAsInt(RTS_DATA_AI_BLACKBOARD_VALUE_SPEEDSTATE, State.GetState(ERTSAi_StateCategory::Speed));
		}
		
		// Check if condition state has changed
		if(State.GetState(ERTSAi_StateCategory::Condition) != GetBlackboardComponent()->GetValueAsInt(RTS_DATA_AI_BLACKBOARD_VALUE_CONDSTATE))
		{
			// Update blackboard if value changed
			GetBlackboardComponent()->SetValueAsInt(RTS_DATA_AI_BLACKBOARD_VALUE_CONDSTATE, State.GetState(ERTSAi_StateCategory::Condition));
		}
		
		// Check if posture state has changed
		if(State.GetState(ERTSAi_StateCategory::Posture) != GetBlackboardComponent()->GetValueAsInt(RTS_DATA_AI_BLACKBOARD_VALUE_POSTURESTATE))
		{
			// Update blackboard if value changed
			GetBlackboardComponent()->SetValueAsInt(RTS_DATA_AI_BLACKBOARD_VALUE_POSTURESTATE, State.GetState(ERTSAi_StateCategory::Posture));
		}
		
		// Check if navigation state has changed
		if(State.GetState(ERTSAi_StateCategory::Navigation) != GetBlackboardComponent()->GetValueAsInt(RTS_DATA_AI_BLACKBOARD_VALUE_NAVSTATE))
		{
			// Update blackboard if value changed
			GetBlackboardComponent()->SetValueAsInt(RTS_DATA_AI_BLACKBOARD_VALUE_NAVSTATE, State.GetState(ERTSAi_StateCategory::Navigation));
		}

		if(GetTargetLastKnownLocation() != FVector::ZeroVector)
		{
			DrawDebugSphere(GetWorld(), GetTargetLastKnownLocation(), 150.f, 8, FColor::Red);
		}
	}

	if(HasAuthority())
	{
		// Handle any active navigation for ai
		HandleWaypointNavigation();
	}

#if WITH_EDITOR
	DebugTarget();
#endif	
}

void ARTSAi_ControllerBase::BeginPlay()
{
	Super::BeginPlay();

	State = FRTSAi_State();
}

void ARTSAi_ControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if(HasAuthority())
	{
		if(AiDataAsset.IsValid() && !bAiInitialised)
		{		
			LoadAiData();
		}
	}

	// Ensure we have a reference to the navigation system
	ensureMsgf((NavSystem = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem())) != nullptr, TEXT("[%s] - NavSystem is Null!"), *GetClass()->GetSuperClass()->GetName());
}

void ARTSAi_ControllerBase::SetAiData(const FPrimaryAssetId& BehaviourAssetId)
{
	if(BehaviourAssetId.IsValid())
	{
		AiDataAsset = BehaviourAssetId;
		
		if(!bAiInitialised)
		{
			LoadAiData();
		}
	}
}

URTSAi_DataAsset* ARTSAi_ControllerBase::GetAiData()
{
	if (const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		if(AiDataAsset.IsValid())
		{
			return Cast<URTSAi_DataAsset>(AssetManager->GetPrimaryAssetObject(AiDataAsset));
		}
	}

	return nullptr;
}

int32 ARTSAi_ControllerBase::GetState(const ERTSAi_StateCategory Category)
{
	return State.GetState(Category);
}

int32 ARTSAi_ControllerBase::GetBehaviourState()
{
	return State.GetState(ERTSAi_StateCategory::Behaviour);
}

int32 ARTSAi_ControllerBase::GetSpeedState()
{
	return State.GetState(ERTSAi_StateCategory::Speed);
}

int32 ARTSAi_ControllerBase::GetConditionState()
{
	return State.GetState(ERTSAi_StateCategory::Condition);
}

int32 ARTSAi_ControllerBase::GetPostureState()
{
	return State.GetState(ERTSAi_StateCategory::Posture);
}

int32 ARTSAi_ControllerBase::GetNavigationState()
{
	return State.GetState(ERTSAi_StateCategory::Navigation);
}

void ARTSAi_ControllerBase::RequestReload()
{
	if(GetEquipment() != nullptr)
	{
		if(IRTSCore_InventoryInterface* EquipmentInterface = Cast<IRTSCore_InventoryInterface>(GetEquipment()))
		{
			if(EquipmentInterface->HasInventoryAmmoForCurrentWeapon())
			{
				SetAmmunitionState(ERTSAi_AmmoState::Reloading);
			}
			else
			{
				// Implement switch weapon
			}
		}
	}
}

void ARTSAi_ControllerBase::LoadAiData()
{
	if(UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		if(AiDataAsset.IsValid())
		{
			const TArray<FName> Bundles;
			const FStreamableDelegate LoadedDelegate = FStreamableDelegate::CreateUObject(this, &ARTSAi_ControllerBase::OnAiDataLoaded);
			AssetManager->LoadPrimaryAsset(AiDataAsset, Bundles, LoadedDelegate);
		}
		else
		{
			UE_LOG(LogRTSAi, Error, TEXT("[%s] - Failed to get player GroupData Assets"), *GetClass()->GetSuperClass()->GetName());
		}
	}
}

void ARTSAi_ControllerBase::OnAiDataLoaded()
{
	if(!bAiInitialised && GetPawn())
	{
		if(const URTSAi_DataAsset* BehaviourData = GetAiData())
		{		
			InitialiseAiBehaviour(BehaviourData);
			InitialisePerceptionSight(BehaviourData);
			
			/*if(IRTSCore_AiInterface* AiInterface = Cast<IRTSCore_AiInterface>(GetPawn()))
			{
				AiInterface->CreatePerceptionStimuliSourceComponent();
			}#1#

			InitAiForGameplay();
		}
	}
}

void ARTSAi_ControllerBase::InitAiForGameplay()
{
	if(GetEquipment())
	{
		OnEquipmentCreated();
	}
	
	bAiInitialised = true;
}

void ARTSAi_ControllerBase::SetState(const ERTSAi_StateCategory Category, const int32 NewState)
{
	if(HasAuthority())
	{
		State.SetState(Category, NewState);
	}
}

void ARTSAi_ControllerBase::InitialiseAiBehaviour(const URTSAi_DataAsset* BehaviourData)
{
	// Set the behaviour tree from the behaviour data
	BehaviourTreeAsset = BehaviourData->BehaviourTreeAsset.LoadSynchronous();

	// Assign the behaviour tree blackboard as our blackboard
	if(BehaviourTreeAsset != nullptr && BehaviourTreeAsset->BlackboardAsset != nullptr)
	{
		UBlackboardComponent* NewBlackboard;
		UseBlackboard(BehaviourTreeAsset->BlackboardAsset, NewBlackboard);
		Blackboard = NewBlackboard;

		// Start the behaviour tree
		RunBehaviorTree(BehaviourTreeAsset);
	}

	SetState(ERTSAi_StateCategory::Behaviour, static_cast<int32>(ERTSAi_BehaviourState::Safe));
}

void ARTSAi_ControllerBase::AbortNavigation(const FGuid CommandId)
{
	if(HasAuthority() && GetPawn())
	{
		if(NavData.IsValidIndex(NavIndex))
		{
			if(GetPathFollowingComponent() && GetPathFollowingComponent()->HasValidPath())
			{
				GetPathFollowingComponent()->AbortMove(*this, EPathFollowingResult::Aborted);
				ResetWaypoints();

				if(NavData.IsValidIndex(NavIndex) && NavData[NavIndex].CommandId == CommandId)
				{
					OnEntityAbortedCommand.Broadcast(NavData[NavIndex].CommandId, GetPawn(), false);
					return;
				}
			}	
		}

		// This delegate broadcast is a fallback if this entity gets an abort for a command its completed or doesnt belong to it
		// the entity will still report back that is aborted / not doing the command
		OnEntityAbortedCommand.Broadcast(CommandId, GetPawn(), false);
	}
}

bool ARTSAi_ControllerBase::IsNavigating() const
{
	return GetPathFollowingComponent() && GetPathFollowingComponent()->GetStatus() != EPathFollowingStatus::Idle;
}

void ARTSAi_ControllerBase::UpdateNavigation(const TArray<FRTSAi_MemberNavigationData>& InGroupNavData,	const int32 InNavIndex)
{
	if(!HasAuthority() || InGroupNavData.Num() <= 0 || !InGroupNavData.IsValidIndex(InNavIndex))
	{
		return;
	}

	// Nav index is the entities reference to its nav data
	NavIndex = InNavIndex;

	// NavData is replicated using OnRep which executes navigation creation and execution 
	NavData = InGroupNavData;
	OnRep_NavData();
}

bool ARTSAi_ControllerBase::HasValidNavigation()
{
	if(!HasAuthority() || !NavData.IsValidIndex(NavIndex) || NavData[NavIndex].Entity == nullptr || !GetWorld() || !NavSystem || !GetPawn())
	{
		ensureMsgf((NavData.IsValidIndex(NavIndex) || NavData[NavIndex].Entity == nullptr), TEXT("[%s] - NavData Invalid!"), *GetClass()->GetSuperClass()->GetName());
		ensureMsgf(NavSystem != nullptr, TEXT("[%s] - NavSystem Invalid!"), *GetClass()->GetSuperClass()->GetName());
		ensureMsgf(GetWorld() != nullptr, TEXT("[%s] - World Reference Invalid!"), *GetClass()->GetSuperClass()->GetName());
		ensureMsgf(GetPawn() != nullptr, TEXT("[%s] - Pawn Reference Invalid!"), *GetClass()->GetSuperClass()->GetName());
		return false;
	}

	return true;
}

void ARTSAi_ControllerBase::GenerateNavigationPath()
{
	if(!HasValidNavigation())
	{
		return;
	}
	
	// Generate an offset path for this entity using the guide path
	TArray<FVector> EntityPathPoints;
	FVector OffsetLocation = FVector::ZeroVector;
	FVector NextOffsetLocation = FVector::ZeroVector;

	// For the guide points we use -1 we only check to the last point, not from the last point to nothing
	for (int i = 0; i < NavData[NavIndex].GuidePathPoints.Num() - 1; ++i)
	{
		// Check we have a valid reference point for the current point and the next point
		if(!NavData[NavIndex].GuidePathPoints.IsValidIndex(i) && !NavData[NavIndex].GuidePathPoints.IsValidIndex(i + 1))
		{
			continue;
		}
			
		// Check if this is the first location		
		if(i == 0)
		{
			// Set the start point to the current destination (this will be next waypoint on aborts)
			FVector StartLocation = IsNavigating() ? BT_GetDestination() : NavData[NavIndex].GuidePathPoints[i];
			
			// Calculate the initial offset location
			CalculateOffsetLocation(StartLocation, OffsetLocation);
		}
		else
		{
			// If not the first location set the current location to the previously calculated next location
			OffsetLocation = NextOffsetLocation;
		}

		// Calculate the next location
		CalculateOffsetLocation(NavData[NavIndex].GuidePathPoints[i + 1], NextOffsetLocation);
			
		// To ensure navigation between new offset points generate the path segment between the two new offset points
		TArray<FVector> SegmentNavPoints = NavSystem->FindPathToLocationSynchronously(GetWorld(), OffsetLocation, NextOffsetLocation, GetPawn())->PathPoints;

		// Ensure offset start position is included 
		/** Combine navigation points into a single path
		 *  For each segment we need to remove the last point as it will be the same as the next segments first point
		 *  except where this is the last segment *#1#
		if(i < NavData[NavIndex].GuidePathPoints.Num() - 2)
		{
			SegmentNavPoints.RemoveAt(SegmentNavPoints.Num() - 1);
		}			

		// Add segment to path
		EntityPathPoints.Append(SegmentNavPoints);	
	}

	/** Next we want to convert our path points to a FRTSEntities_Waypoint struct so we can reference data on each point
	 *  The begining of formation moves will be handled in the generation of waypoint so we can disable formations for the form up*#1#
	if(EntityPathPoints.Num() >= 2)
	{
#if WITH_EDITOR
		Debug_EntityPath(EntityPathPoints);
#endif
		
		Waypoints = GenerateWaypoints(EntityPathPoints);	
		OnRep_Waypoints();		
	}
	else
	{
		UE_LOG(LogRTSAi, Error, TEXT("[%s] Failed to generate valid path!"), *GetClass()->GetSuperClass()->GetName());
	}
}

void ARTSAi_ControllerBase::CalculateOffsetLocation(FVector& ReferencePoint, FVector& OffsetLocation)
{
	if(GetPawn() && NavData.IsValidIndex(NavIndex))
	{
		// Get an offset value from the formation
		FVector Offset = GetFormationOffset();	

		// Check if the entities position is the center position, then there is no offset
		if(NavData[NavIndex].FormationPosition.Side == ERTSAi_SelectionSide::CenterSide)
		{
			OffsetLocation = ReferencePoint;
			return;
		}		

		// Check if the entities position is on the left, invert the offset
		if(NavData[NavIndex].FormationPosition.Side == ERTSAi_SelectionSide::LeftSide)
		{
			Offset.Y = Offset.Y * -1;			
		}

		// Add the formation spacing to the calculation (this is the total spacing to the entities position)
		Offset *= NavData[NavIndex].FormationPosition.Spacing;

		// Convert the location from local space to world space
		Offset = NavData[NavIndex].DestinationRotation.RotateVector(Offset);

		// Add offset to reference position
		OffsetLocation = ReferencePoint + Offset;

		// Validate the new location on the nav system
		ValidateLocation(OffsetLocation);
	}
}

TArray<FRTSAi_NavigationWaypoint> ARTSAi_ControllerBase::GenerateWaypoints(const TArray<FVector>& EntityPathPoints)
{
	if(!HasValidNavigation())
	{
		return TArray<FRTSAi_NavigationWaypoint>();
	}

	/** Ensure we have a complete path from entity to destination and register the form up path point index *#1#
	TArray<FVector> CompletePathPoints;
	int32 FormUpIndex = -1;
	GenerateCompletePath(EntityPathPoints, CompletePathPoints, FormUpIndex);

	// Generate navigation waypoints from generated navigation path points
	TArray<FRTSAi_NavigationWaypoint> NewWaypoints;
	bool bHasFormedUp = false;
	for (int i = 0; i < CompletePathPoints.Num(); ++i)
	{
		// Check if we have reach the form up nav point (this will be always false when no form up required ie. -1)
		if(i >= FormUpIndex)
		{
			bHasFormedUp = true;
		}

		// Check if the first waypoint
		if(i == 0)
		{
			// Add a start waypoint
			URTSCore_SystemStatics::GetTerrainPosition(GetWorld(), CompletePathPoints[i]);
			NewWaypoints.Add(FRTSAi_NavigationWaypoint(
				ERTSAi_WaypointType::Start,
				CompletePathPoints[i],
				FRotationMatrix::MakeFromX(CompletePathPoints[i+1] - CompletePathPoints[i]).Rotator(),
				ERTSAi_MoveType::NoFormation
			));

			// Next waypoint
			continue;
		}

		// Validate the current path point and the previous point 
		if(!CompletePathPoints.IsValidIndex(i-1) || !CompletePathPoints.IsValidIndex(i))
		{
			continue;
		}

		// Get the current segment distance (previous to current) so we can check if we need to add additional waypoints
		const float PathSegmentDistance = (CompletePathPoints[i-1] - CompletePathPoints[i]).Length();
		
		/** Determine if additional waypoints are required:
		 *  If the segment distance is greater than the max waypoint length
		 *  If its not longer than 2x the max length just add a new waypoint to the mid point
		 *  otherwise determine how many points are needed and add as required *#1#
		const float MaxWaypointLength = GetMaxWaypointLength();
		if (PathSegmentDistance > MaxWaypointLength * 2)
		{
			// Check how many new waypoints we need
			const int32 NumNewPoints = FMath::FloorToInt(PathSegmentDistance / MaxWaypointLength) - 1;
			FVector Direction = (CompletePathPoints[i] - CompletePathPoints[i-1]).GetSafeNormal();

			for (int32 j = 1; j < NumNewPoints; j++)
			{
				FVector NewPoint = CompletePathPoints[i-1] + Direction * (PathSegmentDistance / NumNewPoints) * j;
				ValidateLocation(NewPoint);

				// If its greater add a waypoint for the validated location at the current waypoint distance
				URTSCore_SystemStatics::GetTerrainPosition(GetWorld(), NewPoint);
				NewWaypoints.Add(FRTSAi_NavigationWaypoint(
					ERTSAi_WaypointType::Nav,
					NewPoint,
					FRotationMatrix::MakeFromX(CompletePathPoints[i] - CompletePathPoints[i-1]).Rotator(),
					bHasFormedUp ? ERTSAi_MoveType::Formation : ERTSAi_MoveType::NoFormation
				));
			}
		}
		else if (PathSegmentDistance > MaxWaypointLength && PathSegmentDistance < MaxWaypointLength * 2) 
		{
			// If the waypoint is greater than the max but less then two times the max add a waypoint to the segment mid point
			FVector MidPoint = (CompletePathPoints[i-1] + CompletePathPoints[i]) * 0.5f;
			ValidateLocation(MidPoint);

			URTSCore_SystemStatics::GetTerrainPosition(GetWorld(), MidPoint);
			NewWaypoints.Add(FRTSAi_NavigationWaypoint(
				ERTSAi_WaypointType::Nav,
				MidPoint,
				FRotationMatrix::MakeFromX(CompletePathPoints[i] - CompletePathPoints[i-1]).Rotator(),
				bHasFormedUp ? ERTSAi_MoveType::Formation : ERTSAi_MoveType::NoFormation
			));
		}

		// At this point we have either add any required extra waypoints or not had to add any
		// Check if the current point is the last point, if so add as destination waypoint
		if(i == CompletePathPoints.Num() - 1)
		{
			URTSCore_SystemStatics::GetTerrainPosition(GetWorld(), CompletePathPoints[i]);
			NewWaypoints.Add(FRTSAi_NavigationWaypoint(
				ERTSAi_WaypointType::Destination,
				CompletePathPoints[i],
				NavData[NavIndex].DestinationRotation,
				bHasFormedUp ? ERTSAi_MoveType::Formation : ERTSAi_MoveType::NoFormation
			));
		}
		else
		{
			/** If the next nav point is less than the max waypoint distance add a waypoint at the next point
			 *  Check if we reached the form up point, set type to form up if so *#1#
			URTSCore_SystemStatics::GetTerrainPosition(GetWorld(), CompletePathPoints[i]);
			NewWaypoints.Add(FRTSAi_NavigationWaypoint(
				(i == FormUpIndex && ShouldUseFormation()) ? ERTSAi_WaypointType::FormUp : ERTSAi_WaypointType::Nav,
				CompletePathPoints[i],
				FRotationMatrix::MakeFromX(CompletePathPoints[i] - CompletePathPoints[i-1]).Rotator(),
				bHasFormedUp ? ERTSAi_MoveType::Formation : ERTSAi_MoveType::NoFormation
			));
		}
	}

	return NewWaypoints;
}

void ARTSAi_ControllerBase::ValidateLocation(FVector& OffsetLocation)
{
	if(GetPawn())
	{	
		const FNavAgentProperties& AgentProps = GetNavAgentPropertiesRef();
		FNavLocation ProjectedLocation;
		NavSystem->ProjectPointToNavigation(OffsetLocation, ProjectedLocation, GetAgentExtent(), &AgentProps);
		OffsetLocation = ProjectedLocation.Location;		
	}
}

void ARTSAi_ControllerBase::GenerateCompletePath(const TArray<FVector>& EntityPathPoints, TArray<FVector>& CompletePathPoints, int32& FormUpIndex)
{
	if(!HasValidNavigation())
	{
		return;
	}

	/** Add the navigation from entity to the beginning of formation path			
	 *  If using a formation, this will be a path from the entity to the form up point
	 *  Check that the actors location and the first nav point are different locations *#1#	
	if(ShouldUseFormation() && EntityPathPoints.IsValidIndex(0) && (EntityPathPoints[0] - GetPawn()->GetActorLocation()).Length() > 100.f)
	{
		// Get a valid start point from the entity, do a stand off check for larger actors 
		FVector StartLocation = FVector::ZeroVector;
		GenerateStandOffLocation(StartLocation, EntityPathPoints[0]);

		// Calculate a path from the new StartLocation to the start of the entity formation path
		CompletePathPoints = NavSystem->FindPathToLocationSynchronously(GetWorld(), StartLocation, EntityPathPoints[0], GetPawn())->PathPoints;

		/** So the entity can move to the form up point without using formations
		 *  we need to reference which waypoint is the begining of using formations (the form up waypoint)
		 *  This will be the first EntityPathPoints or the last point of the new start path (which should be the same) *#1#
		FormUpIndex = CompletePathPoints.Num() - 1;

		// Removing the last point of the start path we can then append the rest of the path
		// (form up index remains valid, ie the first EntityPathPoint)
		CompletePathPoints.RemoveAt(CompletePathPoints.Num() - 1);
		CompletePathPoints.Append(EntityPathPoints);
	}
	else
	{
		// If not using formations or the first nav point is the same as the actors location then just use the current entity path points
		CompletePathPoints = EntityPathPoints;
	}

#if WITH_EDITOR
	Debug_RawPathPoints(CompletePathPoints);
#endif	
}

void ARTSAi_ControllerBase::GenerateStandOffLocation(FVector& StandoffLocation, const FVector& FirstNavPoint)
{
	if(HasAuthority() && GetPawn())
	{		
		// Get a start location at the stand off radius in the direction of the first nav point
		StandoffLocation = GetPawn()->GetActorLocation() + ((FirstNavPoint - GetPawn()->GetActorLocation()).GetSafeNormal()) * GetStandOffRadius();

		// Ensure new standoff location is on the navmesh and valid for the entity
		ValidateLocation(StandoffLocation);		
	}
}

void ARTSAi_ControllerBase::CreateSplinePath()
{
	if(GetPawn())
	{
		if(const IRTSCore_EntityInterface* AiInterface = Cast<IRTSCore_EntityInterface>(GetPawn()))
		{
			NavigationSpline = AiInterface->GetNavigationSpline();
			
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

void ARTSAi_ControllerBase::AssignNavigationCornerSharpness()
{
	if(GetPawn() && NavigationSpline)
	{
		for (int i = 0; i < Waypoints.Num() - 1; ++i)
		{
			if(Waypoints.IsValidIndex(i) && Waypoints.IsValidIndex(i+1))
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

				Waypoints[i+1].CornerSharpness = AngleDegrees;
			}
		}
	}
}

void ARTSAi_ControllerBase::ResetWaypoints()
{
	if(HasAuthority())
	{
		Waypoints.Empty();
		OnRep_Waypoints();
	}
}

void ARTSAi_ControllerBase::SetCurrentWaypoint()
{
	if(!HasAuthority())
	{
		return;
	}

	// Find an uncompleted waypoint
	for (int i = 0; i < Waypoints.Num(); ++i)
	{
		// Check if waypoint is complete
		if(Waypoints[i].bIsCompleted)
		{
			continue;
		}
		else
		{
			CurrentWaypointIndex = i;			
			OnRep_CurrentWaypointIndex();
			return;
		}			
	}

	// If no uncompleted waypoint found set to invalid waypoint and execute command complete
	CurrentWaypointIndex = -1;
	OnRep_CurrentWaypointIndex();
	
	if(State.GetState(ERTSAi_StateCategory::Navigation) == static_cast<int32>(ERTSAi_NavigationState::Navigating))
	{
		HandleWaypointNavigationComplete();
	}
}

void ARTSAi_ControllerBase::BeginNavigatingCurrentWaypoint()
{
	BT_UpdateNavigationState();
}

void ARTSAi_ControllerBase::HandleWaypointNavigation()
{
	if(HasAuthority() && Waypoints.IsValidIndex(CurrentWaypointIndex))
	{
		// Update waypoint distance to completion
		Waypoints[CurrentWaypointIndex].UpdateDistanceToCompletion(GetPathFollowingComponent()->GetCurrentNavLocation());

		// Manage entities position in formation
		ManageFormationPosition();

		// Handle waypoint approach
		HandleWaypointApproach();
		
#if WITH_EDITOR
		Debug_Waypoints();		
#endif
	}	
}

bool ARTSAi_ControllerBase::HandleArrivalAtDestination()
{
	return false;
}

void ARTSAi_ControllerBase::HandleWaypointNavigationComplete()
{
	if(GetPawn() && GetPathFollowingComponent())
	{
		GetPathFollowingComponent()->OnPathFinished(EPathFollowingResult::Success, FPathFollowingResultFlags::None);
		ResetWaypoints();

		if(NavData.IsValidIndex(NavIndex))
		{
			OnEntityCompletedCommand.Broadcast(NavData[NavIndex].CommandId, GetPawn(), true);
		}
	}
}

void ARTSAi_ControllerBase::ManageFormationPosition()
{
}

void ARTSAi_ControllerBase::HandleWaypointApproach()
{
	if(!HasValidNavigation() || !Waypoints.IsValidIndex(CurrentWaypointIndex))
	{
		return;
	}

	// Check this waypoint is not our destination
	if(Waypoints[CurrentWaypointIndex].WaypointType != ERTSAi_WaypointType::Destination)
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
			SetCurrentWaypointComplete();
			HandleWaypointNavigationComplete();
			HandleArrivalAtDestination();
		}
	}
}

void ARTSAi_ControllerBase::SetCurrentWaypointComplete()
{
	if(HasAuthority())
	{
		if(Waypoints.IsValidIndex(CurrentWaypointIndex))
		{
			Waypoints[CurrentWaypointIndex].Completed();
		}
	}
}

bool ARTSAi_ControllerBase::GetMovementDirection() const
{
	return bReverse;
}

bool ARTSAi_ControllerBase::IsApproachingCurrentWaypoint() const
{
	return false;
}

bool ARTSAi_ControllerBase::HasReachedDestination() const
{
	return GetPawn() && Waypoints.IsValidIndex(CurrentWaypointIndex) && (Waypoints[CurrentWaypointIndex].Location - GetPawn()->GetActorLocation()).Length() < GetAcceptanceDistance() && GetSpeed() < GetAcceptanceSpeed();
}

float ARTSAi_ControllerBase::GetSpeed() const
{
	return GetPawn() && FMath::Abs(GetPawn()->GetVelocity().Length()) > 10.f ? FMath::Abs(GetPawn()->GetVelocity().Length()) : 0.f; 
}

float ARTSAi_ControllerBase::GetMaxSpeed() const
{
	return 400.f;
}

float ARTSAi_ControllerBase::GetAcceptanceSpeed() const
{
	return 5.f;
}

float ARTSAi_ControllerBase::GetAcceptanceDistance() const
{
	return 20.f;
}

FVector ARTSAi_ControllerBase::GetAgentExtent() const
{
	if(GetPawn())
	{
		return GetPawn()->GetComponentsBoundingBox().GetExtent();
	}
	
	return FVector::ZeroVector;
}

float ARTSAi_ControllerBase::GetMaxWaypointLength()
{
	return 500.f;
}

FVector ARTSAi_ControllerBase::GetFormationOffset() const
{
	return FVector(0.f, 1.f, 0.f);
}

float ARTSAi_ControllerBase::GetStandOffRadius() const
{
	return 150.f;
}

bool ARTSAi_ControllerBase::ShouldUseFormation() const
{
	return false;
}

void ARTSAi_ControllerBase::BT_UpdateCurrentWaypoint()
{
	if(GetPawn() && GetBlackboardComponent())
	{
		if(Waypoints.Num() == 0 || CurrentWaypointIndex == -1)
		{
			GetBlackboardComponent()->SetValueAsVector(RTS_DATA_AI_BLACKBOARD_VALUE_WPLOC,GetPawn()->GetActorLocation());				
			return;
		}
		
		GetBlackboardComponent()->SetValueAsVector(RTS_DATA_AI_BLACKBOARD_VALUE_WPLOC, Waypoints[CurrentWaypointIndex].Location);
	}
}

void ARTSAi_ControllerBase::BT_UpdateDestination()
{
	if(GetBlackboardComponent())
	{
		if(GetPawn() && Waypoints.Num() == 0)
		{
			GetBlackboardComponent()->SetValueAsVector(RTS_DATA_AI_BLACKBOARD_VALUE_DESTLOC, GetPawn()->GetActorLocation());
			return;
		}
		
		for (int i = 0; i < Waypoints.Num(); ++i)
		{
			if(Waypoints[i].WaypointType == ERTSAi_WaypointType::Destination)
			{
				GetBlackboardComponent()->SetValueAsVector(RTS_DATA_AI_BLACKBOARD_VALUE_DESTLOC, Waypoints[i].Location);
			}
		}
	}
}

void ARTSAi_ControllerBase::BT_SetDestination(const FVector& NewDestination)
{
	if(GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsVector(RTS_DATA_AI_BLACKBOARD_VALUE_DESTLOC, NewDestination);
	}
}

FVector ARTSAi_ControllerBase::BT_GetDestination() const
{
	if(GetBlackboardComponent())
	{
		return GetBlackboardComponent()->GetValueAsVector(RTS_DATA_AI_BLACKBOARD_VALUE_DESTLOC);
	}

	return FVector::ZeroVector;
}

void ARTSAi_ControllerBase::BT_UpdateNavigationState()
{
	if(GetBlackboardComponent())
	{
		if(Waypoints.Num() == 0)
		{
			if(State.GetState(ERTSAi_StateCategory::Navigation) != static_cast<int32>(ERTSAi_NavigationState::Idle))
			{
				// Update blackboard if value changed
				SetState(ERTSAi_StateCategory::Navigation, static_cast<int32>(ERTSAi_NavigationState::Idle));
			}
		}
		else
		{
			if(State.GetState(ERTSAi_StateCategory::Navigation) != static_cast<int32>(ERTSAi_NavigationState::Navigating))
			{
				// Update blackboard if value changed
				SetState(ERTSAi_StateCategory::Navigation, static_cast<int32>(ERTSAi_NavigationState::Navigating));
			}
		}
	}
}

void ARTSAi_ControllerBase::BT_SetNavigationStateAvoidance()
{
	if(GetBlackboardComponent())
	{
		if(State.GetState(ERTSAi_StateCategory::Navigation) != static_cast<int32>(ERTSAi_NavigationState::Avoiding))
		{
			// Update blackboard if value changed
			SetState(ERTSAi_StateCategory::Navigation, static_cast<int32>(ERTSAi_NavigationState::Avoiding));
		}
	}
}

void ARTSAi_ControllerBase::BT_SetNavigationStateReNav()
{
	if(GetBlackboardComponent())
	{
		if(State.GetState(ERTSAi_StateCategory::Navigation) != static_cast<int32>(ERTSAi_NavigationState::ReNavigating))
		{
			// Update blackboard if value changed
			SetState(ERTSAi_StateCategory::Navigation, static_cast<int32>(ERTSAi_NavigationState::ReNavigating));
		}
	}
}

#if WITH_EDITOR
void ARTSAi_ControllerBase::Debug_MemberNav() const
{
	if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(CoreSettings->DebugRTSAi && CoreSettings->DebugEntityNavigation && GetWorld() && NavData.IsValidIndex(NavIndex))
		{
			FColor Color;
			switch (NavData[NavIndex].FormationPosition.Index)
			{
				case 0: Color = FColor::Blue; break;
				case 1: Color = FColor::Green; break;
				case 2: Color = FColor::Orange; break;
				case 3: Color = FColor::Red; break;
				default: Color = FColor::Black;
			}

			const FVector MemberPos = NavData[NavIndex].Entity->GetActorLocation();
			DrawDebugSphere(GetWorld(), MemberPos, 50.f, 8, Color, false, 7.f, 0, 2.f);
			DrawDebugSphere(GetWorld(), NavData[NavIndex].DestinationLocation, 50.f, 8, Color, false, 7.f, 0, 2.f);
			DrawDebugLine(GetWorld(), MemberPos, NavData[NavIndex].DestinationLocation, Color, false, 7.f, 0, 5.f);		
		}
	}
}

void ARTSAi_ControllerBase::Debug_RawPathPoints(const TArray<FVector>& PathPoints) const
{
	if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(CoreSettings->DebugRTSAi && CoreSettings->DebugEntityNavigation && GetWorld())
		{
			for (int i = 0; i < PathPoints.Num(); ++i)
			{
				FColor SphereColor = i % 2 == 0 ? FColor::Blue : FColor::Emerald;
				FColor LineColor = i % 2 == 0 ? FColor::Red : FColor::Cyan;
				
				// Draw a debug sphere at point
				DrawDebugSphere(GetWorld(), PathPoints[i], 20.f, 12, SphereColor, false, 10.f, 0, 2.f);

				// Draw a debug line to the next point (if there is a next point)
				if (i < PathPoints.Num() - 1)
				{
					DrawDebugLine(GetWorld(), NavData[NavIndex].GuidePathPoints[i], NavData[NavIndex].GuidePathPoints[i + 1], LineColor, false, 10.f, 0, 2.f);
				}
			}
		}
	}
}

void ARTSAi_ControllerBase::Debug_GuidePath() const
{
	if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(CoreSettings->DebugRTSAi && CoreSettings->DebugEntityNavigation &&  GetWorld())
		{
			for (int i = 0; i < NavData[NavIndex].GuidePathPoints.Num(); ++i)
			{
				// Draw a debug sphere at point
				DrawDebugSphere(GetWorld(), NavData[NavIndex].GuidePathPoints[i], 20.f, 12, FColor::Red, false, 10.f, 0, 2.f);

				// Draw a debug line to the next point (if there is a next point)
				if (i < NavData[NavIndex].GuidePathPoints.Num() - 1)
				{
					DrawDebugLine(GetWorld(), NavData[NavIndex].GuidePathPoints[i], NavData[NavIndex].GuidePathPoints[i + 1], FColor::Red, false, 10.f, 0, 2.f);
				}
			}
		}
	}
}

void ARTSAi_ControllerBase::Debug_EntityPath(const TArray<FVector>& EntityPoints) const
{
	if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(CoreSettings->DebugRTSAi && CoreSettings->DebugEntityNavigation && GetWorld() && NavData.IsValidIndex(NavIndex))
		{
			FColor Color;
			switch (NavData[NavIndex].FormationPosition.Index)
			{
				case 0: Color = FColor::Blue; break;
				case 1: Color = FColor::Green; break;
				case 2: Color = FColor::Orange; break;
				case 3: Color = FColor::Red; break;
				default: Color = FColor::Black;
			}

			for (int i = 0; i < EntityPoints.Num(); ++i)
			{
				DrawDebugSphere(GetWorld(), EntityPoints[i], 30.f, 8, Color, false, 7.f, 0, 2.f);
			}		
		}
	}
}

void ARTSAi_ControllerBase::Debug_Waypoints()
{
	if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(CoreSettings->DebugRTSAi && CoreSettings->DebugEntityNavigation && GetWorld())
		{
			for (int i = 0; i < Waypoints.Num(); ++i)
			{
				if(Waypoints[i].WaypointType == ERTSAi_WaypointType::Start)
				{
					DrawDebugSphere(GetWorld(), Waypoints[i].Location, 20.f, 8, FColor::Blue, false, -1.f, 0, 5.f);
				}
				else if(Waypoints[i].WaypointType == ERTSAi_WaypointType::FormUp)
				{
					DrawDebugSphere(GetWorld(), Waypoints[i].Location, 20.f, 8, FColor::Orange, false, -1.f, 0, 5.f);
				}	
				else if(Waypoints[i].WaypointType == ERTSAi_WaypointType::Nav)
				{
					DrawDebugSphere(GetWorld(), Waypoints[i].Location, 20.f, 8, FColor::Green, false, -1.f, 0, 5.f);
				}
				if(Waypoints[i].WaypointType == ERTSAi_WaypointType::Destination) 
				{
					DrawDebugSphere(GetWorld(), Waypoints[i].Location, 20.f, 8, FColor::Cyan, false, -1.f, 0, 2.f);
				}
			}
		}
	}
}

void ARTSAi_ControllerBase::Debug_Formation(FVector EntityLocation, FVector SubordinateLocation, const bool bSubordinateInPosition) const
{
	if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(CoreSettings->DebugRTSAi && CoreSettings->DebugEntityFormations && GetWorld() && NavData.IsValidIndex(NavIndex))
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
			DrawDebugSphere(GetWorld(), MemberPos, 50.f, 8, Color, false, -1.f, 0, 2.f);#1#

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

void ARTSAi_ControllerBase::Debug_EntityGroupIndex() const
{
	if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(CoreSettings->DebugRTSAi && CoreSettings->DebugEntityFormations && GetWorld() && NavData.IsValidIndex(NavIndex))
		{
			FColor Color;
			switch (NavData[NavIndex].FormationPosition.Index)
			{
			case 0: Color = FColor::Blue; break;
			case 1: Color = FColor::Green; break;
			case 2: Color = FColor::Cyan; break;
			case 3: Color = FColor::Red; break;
			default: Color = FColor::Black;
			}

			const FVector MemberPos = NavData[NavIndex].Entity->GetActorLocation();
			DrawDebugSphere(GetWorld(), MemberPos, 50.f, 8, Color, false, -1.f, 0, 2.f);
		}
	}
}
#endif




void ARTSAi_ControllerBase::OnRep_NavData()
{
	if(HasAuthority())
	{
#if WITH_EDITOR
		Debug_MemberNav();
		Debug_GuidePath();		
#endif		
		
		GenerateNavigationPath();
	}
}

void ARTSAi_ControllerBase::OnRep_Waypoints()
{
	if(NavigationSpline)
	{
		NavigationSpline->ClearSplinePoints();
	}
	
	if(Waypoints.Num() > 0)
	{
		CreateSplinePath();
		AssignNavigationCornerSharpness();
	}
	
	if(HasAuthority())
	{		
		BT_UpdateDestination();
		BT_UpdateNavigationState();
		SetCurrentWaypoint();
	}
}

void ARTSAi_ControllerBase::OnRep_CurrentWaypointIndex()
{
	if(HasAuthority())
	{
		// Update behaviour tree current waypoint
		BT_UpdateCurrentWaypoint();

		// Begin nav
		BeginNavigatingCurrentWaypoint();
	}
}


void ARTSAi_ControllerBase::SetupPerceptionSystem()
{
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	if(SightConfig)
	{
		SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent")));
	}
}

void ARTSAi_ControllerBase::InitialisePerceptionSight(const URTSAi_DataAsset* BehaviourData)
{
	if(SightConfig && GetPerceptionComponent() && BehaviourData != nullptr)
	{
		SightConfig->SightRadius = BehaviourData->SightRadius;
		SightConfig->LoseSightRadius = SightConfig->SightRadius + BehaviourData->LoseSightRadius;
		SightConfig->PeripheralVisionAngleDegrees = BehaviourData->PeripheralVisionAngleDegrees;
		SightConfig->SetMaxAge(BehaviourData->SightAge);
		SightConfig->AutoSuccessRangeFromLastSeenLocation = BehaviourData->AutoSuccessRangeFromLastSeenLocation;
		SightConfig->DetectionByAffiliation.bDetectEnemies = BehaviourData->DetectEnemies;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = BehaviourData->DetectFriendlies;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = BehaviourData->DetectNeutrals;

		GetPerceptionComponent()->ConfigureSense(*SightConfig);
		GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
		GetPerceptionComponent()->OnPerceptionUpdated.AddDynamic(this, &ThisClass::UpdatePerceivedActors);
		GetPerceptionComponent()->RequestStimuliListenerUpdate();
		GetPerceptionComponent()->SetSenseEnabled(UAISense_Sight::StaticClass(), true);
	}
}

ETeamAttitude::Type ARTSAi_ControllerBase::GetTeamAttitudeTowards(const AActor& Other) const
{
	if(const APawn* OtherPawn = Cast<APawn>(&Other))
	{
		if(const IGenericTeamAgentInterface* OtherTeamInterface = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController()))
		{
			FGenericTeamId OtherId = OtherTeamInterface->GetGenericTeamId();
			FGenericTeamId ThisId = GetGenericTeamId();
			UE_LOG(LogTemp, Warning, TEXT("ThisId: %d, OtherId: %d"), ThisId.GetId(), OtherId.GetId());
			
			// Check for neutral
			if(OtherTeamInterface->GetGenericTeamId().GetId() != 0)
			{
				// Check friendly or hostile
				//return Super::GetTeamAttitudeTowards(Other);
				return OtherTeamInterface->GetGenericTeamId().GetId() == GetGenericTeamId().GetId() ? ETeamAttitude::Friendly : ETeamAttitude::Hostile;
			}
		}
	}

	return ETeamAttitude::Neutral;
}

void ARTSAi_ControllerBase::StartUpdatingPerception()
{
	if(GetWorld() && !GetWorld()->GetTimerManager().TimerExists(Handle_UpdatingPerception))
	{
		GetWorld()->GetTimerManager().SetTimer(Handle_UpdatingPerception, this, &ThisClass::UpdatePerception, 1.f, true);
	}
}

void ARTSAi_ControllerBase::StopUpdatingPerception()
{
	if(GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(Handle_UpdatingPerception);
	}
}

void ARTSAi_ControllerBase::UpdatePerception()
{
	if(GetPerceptionComponent() && GetPawn())
	{	
		TArray<AActor*> UpdatedHostiles;
		for (auto It = PerceivedEntities.CreateIterator(); It; ++It)
		{
			if(It.Key() != nullptr)
			{
				const float SeenTime = GetWorld()->GetTimeSeconds();

				// Get target eye point (in case behind cover)
				FVector ViewPoint;
				FRotator ViewRotation;
				It.Key()->GetActorEyesViewPoint(ViewPoint, ViewRotation);

				// LOS to eye location
				if(HasLineOfSight(ViewPoint))
				{
					// Update perceived last seen time
					It.Value() = SeenTime;
					
					if(const IRTSCore_Team* TeamInterface = Cast<IRTSCore_Team>(It.Key()))
					{
						if(TeamInterface->GetTeamAttitudeTowards(*It.Key()) == ETeamAttitude::Hostile)
						{
							// Add to hostile list and update last seen hostile time
							UpdatedHostiles.Add(It.Key());
							LastSeenTargetTime = SeenTime;
						}
					}					
				}
				else
				{
					// If can no longer directly see perceived actor check if known time should expire
					if(const URTSAi_DataAsset* AiData = GetAiData())
					{
						if(SeenTime - It.Value() > AiData->SightAge)
						{
							It.RemoveCurrent();
							continue;
						}
					}
				}
				
#if WITH_EDITOR
				DebugTargets(It.Key());
#endif	
			}
		}

		// Report hostiles to group
		ReportHostiles(UpdatedHostiles);
	}

	if(PerceivedEntities.Num() <= 0)
	{
		StopUpdatingPerception();
	}
}

void ARTSAi_ControllerBase::UpdatePerceivedActors(const TArray<AActor*>& UpdatedActors)
{
	if(!GetWorld() || !GetPerceptionComponent())
	{
		return;
	}
	
	// Update perceived entities
	for (int i = 0; i < UpdatedActors.Num(); ++i)
	{
		if(!PerceivedEntities.Contains(UpdatedActors[i]))
		{
			PerceivedEntities.Add(UpdatedActors[i], GetWorld()->GetTimeSeconds());
		}
	}

	// Check all current perceived actors	
	TArray<AActor*> CurrentlyPerceivedActors;
	GetPerceptionComponent()->GetCurrentlyPerceivedActors(SightConfig->GetSenseImplementation(), CurrentlyPerceivedActors);
	for (int j = 0; j < CurrentlyPerceivedActors.Num(); ++j)
	{
		if(!PerceivedEntities.Contains(CurrentlyPerceivedActors[j]))
		{
			PerceivedEntities.Add(CurrentlyPerceivedActors[j], GetWorld()->GetTimeSeconds());
		}
	}
	
	if(PerceivedEntities.Num() > 0)
	{
		if(!GetWorld()->GetTimerManager().TimerExists(Handle_UpdatingPerception))
		{
			// Begin updating perception
			StartUpdatingPerception();
		}		
	}
}

void ARTSAi_ControllerBase::ReportHostiles(const TArray<AActor*>& Hostiles)
{
	// Report targets to group
	if(Group != nullptr)
	{
		if(IRTSCore_GroupInterface* CombatInterface = Cast<IRTSCore_GroupInterface>(Group))
		{
			CombatInterface->ReportHostiles(this, Hostiles);
		}
	}
}

void ARTSAi_ControllerBase::SetCollisionType(ERTSAi_CollisionType& CollisionType, const float DotProduct)
{	
	if(DotProduct >= 0.75f)
	{
		CollisionType = ERTSAi_CollisionType::SameDirection;
	}
	else if(DotProduct < 0.75 && DotProduct > -0.75f)
	{
		CollisionType = ERTSAi_CollisionType::Perpendicular;
	}
	else if(DotProduct <= -0.75f)
	{
		CollisionType = ERTSAi_CollisionType::Incoming;
	}
}

bool ARTSAi_ControllerBase::SweepForCollision(float& HitRange)
{
	if(GetWorld() && GetPawn() && GetPawn()->GetVelocity().Size() > 25.f)
	{
		// Get the collision shape for selected unit
		FCollisionShape CollisionShape = GetCollisionShape();

		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = false;
		QueryParams.bReturnPhysicalMaterial = false;
		QueryParams.AddIgnoredActor(GetPawn());
		
		const FVector Start = GetPawn()->GetActorLocation() + FVector(0, 0, 35);
		const FVector End = (GetPawn()->GetActorLocation() + FVector(0, 0, 35)) + GetPawn()->GetVelocity() * 2.f;
	
		FHitResult HitResult;
		GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, RTS_TRACE_CHANNEL_ENTITIES, CollisionShape, QueryParams);

		FColor BoxColor = HitResult.bBlockingHit ? FColor::Red : FColor::Green; 
		DrawDebugDirectionalArrow(GetWorld(), Start, End, 25.f, BoxColor, false, 0.1f, 0, 3.f);
		DrawDebugBox(GetWorld(), End, CollisionShape.GetExtent(), GetPawn()->GetActorForwardVector().ToOrientationQuat(), BoxColor, false, 0.1f, 0, 3.f);
				
		if(HitResult.bBlockingHit)
		{
			HitRange = (HitResult.Location - GetPawn()->GetActorLocation()).Length();
			return true;
		}
	}
	
	return false; 
}

FCollisionShape ARTSAi_ControllerBase::GetCollisionShape() const
{
	if(GetPawn())
	{
		if(const IRTSCore_EntityInterface* EntityInterface = Cast<IRTSCore_EntityInterface>(GetPawn()->GetController()))
		{
			if(UShapeComponent* CollisionComponent = EntityInterface->GetCollisionBox())
			{
				// Get the bounding box of the mesh
				if (CollisionComponent->IsA<USphereComponent>())
				{
					if (const USphereComponent* SphereComponent = Cast<USphereComponent>(CollisionComponent))
					{
						return FCollisionShape::MakeSphere(SphereComponent->GetUnscaledSphereRadius());
					}
				}
				
				if (CollisionComponent->IsA<UBoxComponent>())
				{
					if (const UBoxComponent* BoxComponent = Cast<UBoxComponent>(CollisionComponent))
					{
						return FCollisionShape::MakeBox(BoxComponent->GetUnscaledBoxExtent() * 0.5f);
					}
				}
			}
			
			/*if(const USceneComponent* EntityMesh = AiInterface->GetEntityMesh())
			{
				// Get the bounding box of the mesh
				const FVector BoxExtent = EntityMesh->CalcBounds(EntityMesh->GetComponentTransform()).BoxExtent * 0.75f;

				// Construct a collision shape from the bounding box
				return FCollisionShape::MakeBox(BoxExtent);
			}#1#
		}
	}

	return FCollisionShape::MakeBox(FVector(100.f,100.f,100.f));
}

int32 ARTSAi_ControllerBase::GetClosestWaypointIndex(const FVector& Location, const int32 StartIndex)
{
	float ClosestDist = MAX_FLT;
	int32 Index = StartIndex;
	for (int i = StartIndex; i < Waypoints.Num(); ++i)
	{
		if(Waypoints.IsValidIndex(i))
		{
			const float Dist = (Waypoints[i].Location - Location).Length();
			if(Dist < ClosestDist)
			{
				ClosestDist = Dist;
				Index = i;
			}
		}
	}

	return Index;
}

void ARTSAi_ControllerBase::AssignTargetData(const FRTSAi_TargetData& TargetData)
{
	if(GetWorld())
	{
		CurrentTarget = TargetData;
		LastAssignedTargetTime = GetWorld()->GetTimeSeconds();
	}
}

FVector ARTSAi_ControllerBase::GetTargetLastKnownLocation() const
{
	if(GetBlackboardComponent())
	{
		return GetBlackboardComponent()->GetValueAsVector(RTS_DATA_AI_BLACKBOARD_VALUE_LASTKNOWNLOC);		
	}

	return FVector::ZeroVector;
}

bool ARTSAi_ControllerBase::HasTarget()
{
	if(GetBlackboardComponent())
	{
		return GetBlackboardComponent()->GetValueAsVector(RTS_DATA_AI_BLACKBOARD_VALUE_LASTKNOWNLOC) != FVector::ZeroVector;
	}			

	return false;
}

bool ARTSAi_ControllerBase::CanUpdateTarget()
{
	if(const URTSAi_DataAsset* AiData = GetAiData())
	{
		if(GetBlackboardComponent())
		{
			const bool bTargetTimeOut = GetWorld() && GetWorld()->GetTimeSeconds() < LastAssignedTargetTime + AiData->SightAge;
			return GetBlackboardComponent()->GetValueAsVector(RTS_DATA_AI_BLACKBOARD_VALUE_LASTKNOWNLOC) == FVector::ZeroVector || bTargetTimeOut;
		}
	}	

	return false;
}

void ARTSAi_ControllerBase::OnEquipmentCreated()
{
	if(IRTSCore_InventoryInterface* EquipmentInterface = Cast<IRTSCore_InventoryInterface>(GetEquipment()))
	{
		if(EquipmentInterface->HasWeaponEquipped() && EquipmentInterface->GetCurrentWeaponMagazineAmmoCount() == 0)
		{
			RequestReload();
		}
	}
}

TArray<FRotator> ARTSAi_ControllerBase::GetTurretRotations() const
{
	if(GetEquipment() != nullptr)
	{
		if(const IRTSCore_InventoryInterface* InventoryInterface = Cast<IRTSCore_InventoryInterface>(GetEquipment()))
		{
			return InventoryInterface->GetTurretRotations();
		}
	}

	return TArray<FRotator>{};
}

TArray<FRotator> ARTSAi_ControllerBase::GetWeaponRotations() const
{
	if(GetEquipment() != nullptr)
	{
		if(const IRTSCore_InventoryInterface* InventoryInterface = Cast<IRTSCore_InventoryInterface>(GetEquipment()))
		{
			return InventoryInterface->GetWeaponRotations();
		}
	}

	return TArray<FRotator>{};
}

void ARTSAi_ControllerBase::ExecuteNavigationCommand(const FRTSAi_MemberNavigationData& EntityNavigationData)
{
	if(HasAuthority())
	{
		NavigationData = EntityNavigationData;
	}
}

bool ARTSAi_ControllerBase::CanEngageTarget()
{
	// Update target data
 	UpdateTargetData();

	/** Check
	 *   - Current target is valid
	 *   - LastKnown location is valid
	 *   - Has line of sight to lastknown location
	 *   - Is within current weapon range *#1#
	return CurrentTarget.IsValid() && CurrentTarget.LastSeenEnemyLocation != FVector::ZeroVector && HasLineOfSight(CurrentTarget.LastSeenEnemyLocation) && TargetInRange(CurrentTarget.LastSeenEnemyLocation);
}

bool ARTSAi_ControllerBase::ValidateCover()
{
	if(GetWorld() && HasTarget())
	{
		FVector TargetLocation = GetTargetLastKnownLocation();
		URTSCore_SystemStatics::GetTerrainPosition(GetWorld(), TargetLocation);
		TargetLocation.Z += 50.f;
		
		FCollisionQueryParams CollisionParams(SCENE_QUERY_STAT(LineOfSight), true, GetPawn());
		CollisionParams.AddIgnoredActor(GetPawn());

		DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), TargetLocation, FColor::Black, false, 5.f);
		
		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, GetPawn()->GetActorLocation(), TargetLocation, ECC_Visibility, CollisionParams))
		{
			if(HitResult.bBlockingHit)
			{
				return HitResult.GetActor() != CurrentTarget.TargetActor;
			}
		}
	}

	return true;
}

void ARTSAi_ControllerBase::FireOnTarget()
{
	if(GetWorld() && !GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_FireWeapon))
	{
		const float FireRate = GetCurrentWeaponFireRate();
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_FireWeapon, this, &ThisClass::FireWeaponTimer, 0.1f, false, FireRate + FMath::FRandRange(-(FireRate * 0.5f), FireRate * 0.5f));		
	}	
}

void ARTSAi_ControllerBase::ReloadWeapon()
{
	if(GetWorld() && !GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_Reload))
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Reload, this, &ThisClass::ReloadTimer, 0.1f, false, GetCurrentWeaponReloadRate());
		
		if(GetEquipment())
		{
			if(IRTSCore_InventoryInterface* EquipmentInterface = Cast<IRTSCore_InventoryInterface>(GetEquipment()))
			{
				EquipmentInterface->ReloadWeapon();
			}
		}
	}
}

void ARTSAi_ControllerBase::SetAmmunitionState(const ERTSAi_AmmoState AmmoState)
{
	// Set Ai state for reloading
	if(GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsEnum(RTS_DATA_AI_BLACKBOARD_VALUE_AMMOSTATE, static_cast<uint8>(AmmoState));
	}
}

bool ARTSAi_ControllerBase::TargetInRange(const FVector& TargetLocation) const
{
	if(GetPawn() && GetEquipment() != nullptr)
	{
		if(IRTSCore_InventoryInterface* EquipmentInterface = Cast<IRTSCore_InventoryInterface>(GetEquipment()))
		{
			// Get range to target and check current weapon is within range
			const float Range = (TargetLocation - GetPawn()->GetActorLocation()).Length();
			const bool bWithinRange = Range < EquipmentInterface->GetCurrentWeaponRange();
			if(!bWithinRange)
			{
				//@TODO Notify target out of range
				return false;
			}
				
			return true;			
		}		
	}

	return false;
}

void ARTSAi_ControllerBase::FireWeaponTimer()
{
	if(GetEquipment() != nullptr)
	{
		if(IRTSCore_InventoryInterface* EquipmentInterface = Cast<IRTSCore_InventoryInterface>(GetEquipment()))
		{
			EquipmentInterface->FireWeapon();
		}
	}
}

void ARTSAi_ControllerBase::ReloadTimer()
{
	if(GetEquipment())
	{
		if(IRTSCore_InventoryInterface* EquipmentInterface = Cast<IRTSCore_InventoryInterface>(GetEquipment()))
		{
			if(EquipmentInterface->GetCurrentWeaponMagazineAmmoCount() > 0)
			{
				SetAmmunitionState(ERTSAi_AmmoState::HasAmmo);
			}
			else
			{
				SetAmmunitionState(ERTSAi_AmmoState::NoAmmo);
			}
		}
	}
}

bool ARTSAi_ControllerBase::HasLineOfSight(const FVector& Location) const
{
	if(GetPawn() && Location != FVector::ZeroVector)
	{
		FVector ViewPoint;
		FRotator ViewRotation;
		GetActorEyesViewPoint(ViewPoint, ViewRotation);
	
		FCollisionQueryParams CollisionParams(SCENE_QUERY_STAT(LineOfSight), true, GetPawn());
		CollisionParams.AddIgnoredActor(GetPawn());
		
		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, ViewPoint, Location, ECC_Visibility, CollisionParams))
		{
			if(HitResult.bBlockingHit)
			{
				return false; //(HitResult.Location - Location).Length() < 100.f;
			}
		}
	}	

	return true;
}

void ARTSAi_ControllerBase::UpdateTargetData()
{
	if(CurrentTarget.IsValid())
	{
		// Check if ai can see target currently, update target location to actual location if so
		if(const APawn* TargetPawn = Cast<APawn>(CurrentTarget.TargetActor))
		{
			if(const AAIController* TargetAiController = Cast<AAIController>(TargetPawn->GetController()))
			{
				FVector ViewPoint;
				FRotator ViewRotation;
				TargetAiController->GetActorEyesViewPoint(ViewPoint, ViewRotation);
				if(HasLineOfSight(ViewPoint))
				{
					CurrentTarget.LastSeenEnemyLocation = ViewPoint;
				}
			}
		}
	}
	else
	{
		CurrentTarget.LastSeenEnemyLocation = FVector::ZeroVector;
	}
	
	UpdateTargetLastKnownLocation();
}

void ARTSAi_ControllerBase::UpdateTargetLastKnownLocation()
{
	if(GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsVector(RTS_DATA_AI_BLACKBOARD_VALUE_LASTKNOWNLOC, CurrentTarget.LastSeenEnemyLocation);				
	}
}

int32 ARTSAi_ControllerBase::GetCurrentWeaponAmmo() const
{
	if(GetEquipment() != nullptr)
	{
		if(IRTSCore_InventoryInterface* InventoryInterface = Cast<IRTSCore_InventoryInterface>(GetEquipment()))
		{
			return InventoryInterface->GetCurrentWeaponMagazineAmmoCount();
		}
	}

	return 0;
}

bool ARTSAi_ControllerBase::HasInventoryAmmoForCurrentWeapon() const
{
	if(GetEquipment() != nullptr)
	{
		if(IRTSCore_InventoryInterface* InventoryInterface = Cast<IRTSCore_InventoryInterface>(GetEquipment()))
		{
			return InventoryInterface->HasInventoryAmmoForCurrentWeapon();
		}
	}

	return false;
}

float ARTSAi_ControllerBase::GetCurrentWeaponFireRate() const
{
	if(GetEquipment() != nullptr)
	{
		if(IRTSCore_InventoryInterface* InventoryInterface = Cast<IRTSCore_InventoryInterface>(GetEquipment()))
		{
			return InventoryInterface->GetCurrentWeaponFireRate();
		}
	}

	return -1.f;
}

float ARTSAi_ControllerBase::GetCurrentWeaponReloadRate() const
{
	if(GetEquipment() != nullptr)
	{
		if(IRTSCore_InventoryInterface* InventoryInterface = Cast<IRTSCore_InventoryInterface>(GetEquipment()))
		{
			return InventoryInterface->GetCurrentWeaponReloadRate();
		}
	}

	return -1.f;
}

float ARTSAi_ControllerBase::GetCurrentWeaponAccuracy() const
{
	if(GetEquipment() != nullptr)
	{
		if(IRTSCore_InventoryInterface* InventoryInterface = Cast<IRTSCore_InventoryInterface>(GetEquipment()))
		{
			return InventoryInterface->GetCurrentWeaponAccuracy();
		}
	}

	return -1.f;
}*/

/*#if WITH_EDITOR

void ARTSAi_ControllerBase::DebugTargets(const AActor* Other) const
{
	if(const URTSCore_DeveloperSettings* DeveloperSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(DeveloperSettings->DebugRTSCore && DeveloperSettings->DebugAiCombat)
		{
			FVector ViewPoint;
			FRotator ViewRotation;
			GetActorEyesViewPoint(ViewPoint, ViewRotation);
			FVector ViewPointTarget;
			FRotator ViewRotationTarget;
			Other->GetActorEyesViewPoint(ViewPointTarget, ViewRotationTarget);
			
			DrawDebugDirectionalArrow(GetWorld(), ViewPoint, ViewPointTarget, 200.f, FColor::Orange, false, 3.f, 0, 2.f);
		}
	}
}

void ARTSAi_ControllerBase::DebugTarget()
{
	if(const URTSCore_DeveloperSettings* DeveloperSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(DeveloperSettings->DebugRTSCore && DeveloperSettings->DebugAiCombat)
		{
			if(HasTarget())
			{
				FVector ViewPoint;
				FRotator ViewRotation;
				GetActorEyesViewPoint(ViewPoint, ViewRotation);
				DrawDebugDirectionalArrow(GetWorld(), ViewPoint, GetTargetLastKnownLocation(), 200.f, FColor::Red, false, 0.1f, 0, 3.f);
			}
		}
	}
}


#endif*/
