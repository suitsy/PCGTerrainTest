// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AI/RTSEntities_AiControllerBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/AssetManager.h"
#include "Framework/Data/RTSEntities_AiDataAsset.h"
#include "Framework/Data/RTSEntities_StaticGameData.h"
#include "Framework/Debug/RTSEntities_Debug.h"
#include "Framework/Entities/Components/RTSEntities_Entity.h"
#include "Framework/Entities/Group/RTSEntities_Group.h"
#include "Framework/Interfaces/RTSCore_EntityInterface.h"
#include "Framework/Settings/RTSCore_DeveloperSettings.h"
#include "Framework/Interfaces/RTSCore_TeamInterface.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"


ARTSEntities_AiControllerBase::ARTSEntities_AiControllerBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	BehaviourTreeAsset = nullptr;
	SightConfig = nullptr;
	LastSeenTargetTime = 0.f;
	EntityComponent = nullptr;
}

void ARTSEntities_AiControllerBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, LastSeenTargetTime);
}

void ARTSEntities_AiControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARTSEntities_AiControllerBase::BeginPlay()
{
	Super::BeginPlay();
}

void ARTSEntities_AiControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	SetDefaultState();
	InitAiForGameplay();
}

void ARTSEntities_AiControllerBase::InitAiForGameplay()
{
	if(AiDataAssetId.IsValid())
	{		
		InitialiseAiBehaviour();
		InitialisePerceptionSight();

		if(GetPawn())
		{
			if(IRTSCore_EntityInterface* EntityInterface = Cast<IRTSCore_EntityInterface>(GetPawn()))
			{
				EntityInterface->CreatePerceptionStimuliSourceComponent();
			}
		}

		SetState(ERTSCore_StateCategory::Active, static_cast<int32>(ERTSCore_ActiveState::Active));		
	}
	else
	{
		UE_LOG(LogRTSEntities, Error, TEXT("[%s] - Failed to init Ai [%s] for gameplay!"), *GetClass()->GetName(), *GetPawn()->GetName());
	}
}

bool ARTSEntities_AiControllerBase::HasEntityComponent()
{
	if(GetPawn() && EntityComponent == nullptr)
	{
		EntityComponent = URTSEntities_Entity::FindEntityComponent(GetPawn());
	}

	return EntityComponent != nullptr;
}

ARTSEntities_Group* ARTSEntities_AiControllerBase::GetEntityGroup()
{
	if(GetPawn())
	{
		if(HasEntityComponent())
		{
			return EntityComponent->GetGroup();
		}
	}

	return nullptr;
}

URTSEntities_AiDataAsset* ARTSEntities_AiControllerBase::GetAiData()
{
	if (const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		if(AiDataAssetId.IsValid())
		{
			return Cast<URTSEntities_AiDataAsset>(AssetManager->GetPrimaryAssetObject(AiDataAssetId));
		}
	}

	return nullptr;
}

FCollisionShape ARTSEntities_AiControllerBase::GetCollisionShape() const
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
		}
	}

	return FCollisionShape::MakeBox(FVector(100.f,100.f,100.f));
}

ERTSCore_EntityType ARTSEntities_AiControllerBase::GetEntityType()
{
	if(GetPawn())
	{
		if(HasEntityComponent())
		{
			return EntityComponent->GetType();
		}
	}

	return ERTSCore_EntityType::NonEntity;
}

void ARTSEntities_AiControllerBase::InitialiseAiBehaviour()
{
	if(const URTSEntities_AiDataAsset* AiData = GetAiData())
	{
		// Set the behaviour tree from the behaviour data
		BehaviourTreeAsset = AiData->BehaviourTreeAsset.LoadSynchronous();
	}
	
	// Assign the behaviour tree blackboard as our blackboard
	if(BehaviourTreeAsset != nullptr && BehaviourTreeAsset->BlackboardAsset != nullptr)
	{
		UBlackboardComponent* NewBlackboard;
		UseBlackboard(BehaviourTreeAsset->BlackboardAsset, NewBlackboard);
		Blackboard = NewBlackboard;

		// Start the behaviour tree
		RunBehaviorTree(BehaviourTreeAsset);
	}
}

int32 ARTSEntities_AiControllerBase::GetState(const ERTSCore_StateCategory Category) const 
{
	if(GetBlackboardComponent())
	{
		switch (Category)
		{		
			case ERTSCore_StateCategory::Active:
				return GetBlackboardComponent()->GetValueAsInt(RTS_BLACKBOARD_VALUE_ACTIVESTATE);
			case ERTSCore_StateCategory::Navigation:
				return GetBlackboardComponent()->GetValueAsInt(RTS_BLACKBOARD_VALUE_NAVSTATE);
			case ERTSCore_StateCategory::Behaviour:
				return GetBlackboardComponent()->GetValueAsInt(RTS_BLACKBOARD_VALUE_BEHAVESTATE);
			case ERTSCore_StateCategory::Speed:
				return GetBlackboardComponent()->GetValueAsInt(RTS_BLACKBOARD_VALUE_SPEEDSTATE);
			case ERTSCore_StateCategory::Condition:
				return GetBlackboardComponent()->GetValueAsInt(RTS_BLACKBOARD_VALUE_CONDSTATE);
			case ERTSCore_StateCategory::Posture:
				return GetBlackboardComponent()->GetValueAsInt(RTS_BLACKBOARD_VALUE_POSTURESTATE);
			default: return -1;
		}
	}

	return -1;
}

void ARTSEntities_AiControllerBase::SetState(const ERTSCore_StateCategory Category, const int32 NewState)
{
	if(HasAuthority() && GetBlackboardComponent())
	{
		switch (Category)
		{
			case ERTSCore_StateCategory::Active:
				GetBlackboardComponent()->SetValueAsInt(RTS_BLACKBOARD_VALUE_ACTIVESTATE, NewState);
				break;
			case ERTSCore_StateCategory::Navigation:
				GetBlackboardComponent()->SetValueAsInt(RTS_BLACKBOARD_VALUE_NAVSTATE, NewState);
				break;
			case ERTSCore_StateCategory::Behaviour:
				GetBlackboardComponent()->SetValueAsInt(RTS_BLACKBOARD_VALUE_BEHAVESTATE, NewState);
				break;
			case ERTSCore_StateCategory::Speed:
				GetBlackboardComponent()->SetValueAsInt(RTS_BLACKBOARD_VALUE_SPEEDSTATE, NewState);
				break;
			case ERTSCore_StateCategory::Condition:
				GetBlackboardComponent()->SetValueAsInt(RTS_BLACKBOARD_VALUE_CONDSTATE, NewState);
				break;
			case ERTSCore_StateCategory::Posture:
				GetBlackboardComponent()->SetValueAsInt(RTS_BLACKBOARD_VALUE_POSTURESTATE, NewState);
				break;
			case ERTSCore_StateCategory::Ammunition:
				GetBlackboardComponent()->SetValueAsInt(RTS_BLACKBOARD_VALUE_AMMOSTATE, NewState);
				break;
			default: ;
		}
	}
}

void ARTSEntities_AiControllerBase::SetDefaultState()
{
	if(HasAuthority() && GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsInt(RTS_BLACKBOARD_VALUE_ACTIVESTATE, static_cast<int32>(ERTSCore_ActiveState::InActive));
		GetBlackboardComponent()->SetValueAsInt(RTS_BLACKBOARD_VALUE_NAVSTATE, static_cast<int32>(ERTSCore_NavigationState::Idle));
		GetBlackboardComponent()->SetValueAsInt(RTS_BLACKBOARD_VALUE_BEHAVESTATE, static_cast<int32>(ERTSCore_BehaviourState::Safe));
		GetBlackboardComponent()->SetValueAsInt(RTS_BLACKBOARD_VALUE_SPEEDSTATE, static_cast<int32>(ERTSCore_SpeedState::Walk));
		GetBlackboardComponent()->SetValueAsInt(RTS_BLACKBOARD_VALUE_CONDSTATE, static_cast<int32>(ERTSCore_ConditionState::Normal));
		GetBlackboardComponent()->SetValueAsInt(RTS_BLACKBOARD_VALUE_POSTURESTATE, static_cast<int32>(ERTSCore_PostureState::Defensive));
	}
}

bool ARTSEntities_AiControllerBase::HasLineOfSight(const FVector& Location) const
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

void ARTSEntities_AiControllerBase::InitialisePerceptionSight()
{
	if(const URTSEntities_AiDataAsset* AiData = GetAiData())
	{
		PerceptionComponent = NewObject<UAIPerceptionComponent>(this, TEXT("AIPerceptionComponent"));
		PerceptionComponent->RegisterComponent();
		SightConfig = NewObject<UAISenseConfig_Sight>(this, TEXT("SightConfiguration"));
		
		if(GetPerceptionComponent() && SightConfig)
		{
			SightConfig->SightRadius = AiData->SightRadius;
			SightConfig->LoseSightRadius = SightConfig->SightRadius + AiData->LoseSightRadius;
			SightConfig->PeripheralVisionAngleDegrees = AiData->PeripheralVisionAngleDegrees;
			SightConfig->SetMaxAge(AiData->SightAge);
			SightConfig->AutoSuccessRangeFromLastSeenLocation = AiData->AutoSuccessRangeFromLastSeenLocation;
			SightConfig->DetectionByAffiliation.bDetectEnemies = AiData->DetectEnemies;
			SightConfig->DetectionByAffiliation.bDetectFriendlies = AiData->DetectFriendlies;
			SightConfig->DetectionByAffiliation.bDetectNeutrals = AiData->DetectNeutrals;

			GetPerceptionComponent()->ConfigureSense(*SightConfig);	
			GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
			GetPerceptionComponent()->OnPerceptionUpdated.AddDynamic(this, &ThisClass::UpdatePerceivedActors);		
			GetPerceptionComponent()->RequestStimuliListenerUpdate();
			GetPerceptionComponent()->SetSenseEnabled(UAISense_Sight::StaticClass(), true);
		}
	}
}

void ARTSEntities_AiControllerBase::StartUpdatingPerception()
{
	if(GetWorld() && !GetWorld()->GetTimerManager().TimerExists(Handle_UpdatingPerception))
	{
		GetWorld()->GetTimerManager().SetTimer(Handle_UpdatingPerception, this, &ThisClass::UpdatePerception, 1.f, true);
	}
}

void ARTSEntities_AiControllerBase::StopUpdatingPerception()
{
	if(GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(Handle_UpdatingPerception);
	}
}

void ARTSEntities_AiControllerBase::ReportHostiles(const TArray<AActor*>& Hostiles)
{
	// Report targets to group
	if(ARTSEntities_Group* EntityGroup = GetEntityGroup())
	{
		EntityGroup->ReportHostiles(this, Hostiles);
	}
}

void ARTSEntities_AiControllerBase::UpdatePerceivedActors(const TArray<AActor*>& UpdatedActors)
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

void ARTSEntities_AiControllerBase::UpdatePerception()
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
					
					if(HasEntityComponent())
					{
						if(const IRTSCore_TeamInterface* TeamsInterface = Cast<IRTSCore_TeamInterface>(EntityComponent))
						{
							if(TeamsInterface->GetTeamAttitudeTowards(*It.Key()) == ETeamAttitude::Hostile)
							{
								// Add to hostile list and update last seen hostile time
								UpdatedHostiles.Add(It.Key());
								LastSeenTargetTime = SeenTime;
							}
						}
					}				
				}
				else
				{
					// If can no longer directly see perceived actor check if known time should expire
					if(const URTSEntities_AiDataAsset* AiData = GetAiData())
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

void ARTSEntities_AiControllerBase::DebugTargets(const AActor* Other) const
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


