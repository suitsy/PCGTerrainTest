// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Entities/Group/RTSEntities_Group.h"
#include "CommandSystem/RTSEntities_Command.h"
#include "Framework/Data/RTSEntities_DataTypes.h"
#include "Components/WidgetComponent.h"
#include "Engine/AssetManager.h"
#include "Framework/Entities/Components/RTSEntities_Entity.h"
#include "Framework/AI/RTSEntities_AiControllerCommand.h"
#include "Framework/Data/RTSCore_SystemStatics.h"
#include "Framework/Data/RTSEntities_AiDataAsset.h"
#include "Framework/Data/RTSEntities_EntityDataAsset.h"
#include "Framework/Data/RTSEntities_GroupDataAsset.h"
#include "Framework/Settings/RTSCore_DeveloperSettings.h"
#include "Framework/Settings/RTSEntities_DeveloperSettings.h"
#include "Framework/UI/RTSEntities_GroupDisplay.h"
#include "Net/UnrealNetwork.h"
#include "StateMachine/RTSEntities_GroupStateManager.h"


ARTSEntities_Group::ARTSEntities_Group(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	bReplicates = true;
	bAlwaysRelevant = true;
	NetPriority = 3.0f;
	SetReplicatingMovement(false);

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;
	
	DisplayComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponentA"));
	DisplayComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	bUpdatingKnownTargets = false;
}

void ARTSEntities_Group::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, DataAssetId);
	DOREPLIFETIME(ThisClass, Entities);
	DOREPLIFETIME(ThisClass, GroupState);
	DOREPLIFETIME(ThisClass, KnownTargets);
	DOREPLIFETIME_CONDITION(ThisClass, OwningPlayer, COND_OwnerOnly);
}

void ARTSEntities_Group::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(GetWorld() && CameraManager != nullptr)
	{
		FVector TargetDisplayLocation = GetCenterPosition();
		URTSCore_SystemStatics::GetTerrainPosition(GetWorld(), TargetDisplayLocation);
		TargetDisplayLocation.Z += DisplayWidgetHeight;
		SetActorLocation(FMath::VInterpTo(GetActorLocation(), TargetDisplayLocation, DeltaTime, 2.f));

		// Set distance opacities
		const float CameraDistance = (CameraManager->GetCameraLocation() - GetActorLocation()).Length();
		if(GroupDisplayWidget)
		{
			GroupDisplayWidget->SetRenderOpacity(FMath::GetMappedRangeValueClamped(FVector2D(DisplayWidgetDistance, DisplayWidgetDistance*2.f), FVector2D(1.0f, 0.0f), CameraDistance));
		}
	}
}

void ARTSEntities_Group::BeginPlay()
{
	Super::BeginPlay();
}

void ARTSEntities_Group::ReportHostiles(AAIController* Reporter, TArray<AActor*> Hostiles)
{
	// Update pending hostile target data
	for (int i = 0; i < Hostiles.Num(); ++i)
	{
		// Use target eye point as location
		FVector HostileLocation;
		FRotator ViewRotation;
		Hostiles[i]->GetActorEyesViewPoint(HostileLocation, ViewRotation);

		// Create a Target Data
		FRTSEntities_TargetData TargetData = FRTSEntities_TargetData(Hostiles[i], Reporter, GetWorld()->GetTimeSeconds(), HostileLocation);
		AssignThreatLevel(TargetData);
		PendingHostileTargets.Add(TargetData);
	}

	// Update known targets	
	if(IsKnownTargetsUpdating())
	{
		if(GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::UpdateKnownTargetsData);
		}
	}
	else
	{
		UpdateKnownTargetsData();
	}
}

void ARTSEntities_Group::AssignThreatLevel(FRTSEntities_TargetData& TargetData) const
{
	// If player has overriden threat level ignore self updating threat
	if(TargetData.ThreatLevel == ERTSCore_ThreatLevel::PlayerSet)
	{
		return;
	}
	
	// Assign a threat level
	if(const APawn* TargetPawn = Cast<APawn>(TargetData.TargetActor))
	{
		if(ARTSEntities_AiControllerBase* AiController = Cast<ARTSEntities_AiControllerBase>(TargetPawn->GetController()))
		{
			// Check if the target is incapacitated, put to no threat if they are
			if(AiController->GetState(ERTSCore_StateCategory::Condition) == static_cast<int32>(ERTSCore_ConditionState::Incapacitated))
			{
				TargetData.ThreatLevel = ERTSCore_ThreatLevel::NoThreat;
				return;
			}

			// Assign threat based on distance			
			if(const URTSEntities_AiDataAsset* AiData = AiController->GetAiData())
			{
				const float MediumThreatDistance = AiData->SightRadius * 0.35f; // Example 1500 - 525
				const float LowThreatDistance = MediumThreatDistance * 2.f;		// Example 1500 - 1050
				const float HighThreatDistance = MediumThreatDistance * 0.5f;	// Example 1500 - 262.5
	
				
				const float DistanceToTarget = (TargetPawn->GetActorLocation() - TargetData.LastSeenEnemyLocation).Length();
				if (DistanceToTarget >= LowThreatDistance)
				{
					TargetData.ThreatLevel = ERTSCore_ThreatLevel::Low;
				}
				else if (DistanceToTarget >= MediumThreatDistance)
				{
					TargetData.ThreatLevel = ERTSCore_ThreatLevel::Medium;
				}
				else if (DistanceToTarget >= HighThreatDistance)
				{
					TargetData.ThreatLevel = ERTSCore_ThreatLevel::High;
				}
				else
				{
					TargetData.ThreatLevel = ERTSCore_ThreatLevel::Extreme;
				}				
			}			
		}
	}	
}

void ARTSEntities_Group::UpdateKnownTargetsData()
{
	for (int i = 0; i < PendingHostileTargets.Num(); ++i)
	{
		// Update or add the target data
		const int32 Index = KnownTargets.Find(PendingHostileTargets[i]);
		if(Index != INDEX_NONE && KnownTargets.IsValidIndex(Index))
		{
			// Update data with new target data at existing index
			if(PendingHostileTargets[i].ThreatLevel > KnownTargets[Index].ThreatLevel)
			{
				KnownTargets[Index].Source = PendingHostileTargets[i].Source;
				KnownTargets[Index].ThreatLevel = PendingHostileTargets[i].ThreatLevel;
			}
			
			KnownTargets[Index].LastSeenEnemyTime = PendingHostileTargets[i].LastSeenEnemyTime;
			KnownTargets[Index].LastSeenEnemyLocation = PendingHostileTargets[i].LastSeenEnemyLocation;
		}
		else
		{
			// Add new entry to know targets
			KnownTargets.Add(PendingHostileTargets[i]);
		}
	}

	PendingHostileTargets.Empty();
}

int32 ARTSEntities_Group::GetState(const ERTSCore_StateCategory Category)
{
	return GroupState.GetState(Category);
}

void ARTSEntities_Group::SetState(const ERTSCore_StateCategory Category, const int32 NewState)
{
	GroupState.SetState(Category, NewState);
	OnRep_GroupState();
}

void ARTSEntities_Group::Register_OnStateUpdate(FOnStateUpdateDelegate::FDelegate&& Delegate)
{
	OnStateUpdate.Add(MoveTemp(Delegate));
	OnStateUpdate.Broadcast(GroupState);
}

void ARTSEntities_Group::CreateGroupStateManager()
{
	if(GroupStateManager == nullptr)
	{
		GroupStateManager = NewObject<URTSEntities_GroupStateManager>(this);
		if(GroupStateManager)
		{
			GroupStateManager->Init();
		}
	}
	else
	{
		GroupStateManager->StartStateMachine();
	}
}

void ARTSEntities_Group::StopGroupStateManager() const
{
	if(GroupStateManager)
	{
		GroupStateManager->ExitStateMachine();
	}
}

void ARTSEntities_Group::Initialise(APlayerController* NewOwningPlayer, const FPrimaryAssetId& GroupDataAsset)
{
	if(HasAuthority() && GroupDataAsset.IsValid())
	{		
		OwningPlayer = NewOwningPlayer;
		DataAssetId = GroupDataAsset;
	}	
}

void ARTSEntities_Group::AssignEntities(const TArray<AActor*>& GroupEntities)
{
	if(HasAuthority())
	{
		Entities = GroupEntities;

		for (int i = 0; i < Entities.Num(); ++i)
		{
			if(URTSEntities_Entity* EntityComponent = URTSEntities_Entity::FindEntityComponent(Entities[i]))
			{
				EntityComponent->InitDefaultSpacing();
			}
		}

		CreateGroupStateManager();
	}
}

void ARTSEntities_Group::OnRep_GroupState()
{
	OnStateUpdate.Broadcast(GroupState);
}

void ARTSEntities_Group::CreateGroupDisplay()
{
	if(GroupDisplayWidget == nullptr)
	{
		if(const URTSEntities_GroupDataAsset* GroupData = GetData())
		{
			if(GroupData->DisplayWidgetClass != nullptr && OwningPlayer != nullptr)
			{
				DisplayComponent->SetWidgetSpace(EWidgetSpace::Screen);
				DisplayComponent->SetCastShadow(false);
				DisplayComponent->SetWidgetClass(GroupData->DisplayWidgetClass.LoadSynchronous());
				DisplayComponent->InitWidget();
				GroupDisplayWidget = Cast<URTSEntities_GroupDisplay>(DisplayComponent->GetWidget());
				if(GroupDisplayWidget)
				{
					GroupDisplayWidget->Init(this);		
				}

				DisplayWidgetHeight = GroupData->DisplayWidgetHeight;
				DisplayWidgetDistance = GroupData->DisplayWidgetDistance;
				CameraManager = OwningPlayer->PlayerCameraManager.Get();
			}
		}
	}
}

URTSEntities_GroupDataAsset* ARTSEntities_Group::GetData() const
{
	if (const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		if(DataAssetId.IsValid())
		{
			return Cast<URTSEntities_GroupDataAsset>(AssetManager->GetPrimaryAssetObject(DataAssetId));	
		}
	}

	return nullptr;
}

/*FVector ARTSEntities_Group::GetCurrentFormationOffset() const
{
	if(const URTSEntities_FormationDataAsset* FormationData = GetFormationData())
	{
		return FormationData->Offset;
	}

	return FVector(0.f,1.f,0.f);
}*/

FVector ARTSEntities_Group::GetCenterPosition() const
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

AActor* ARTSEntities_Group::GetLead(const TArray<AActor*>& GroupMembers)
{
	AActor* NewLeader = nullptr;
	int32 LeaderIndex = 99;		
	for (int i = 0; i < GroupMembers.Num(); ++i)
	{
		if(const URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(GroupMembers[i]))
		{
			if(Entity->GetIndex() < LeaderIndex)
			{
				NewLeader = GroupMembers[i];
				LeaderIndex = Entity->GetIndex();
			}
		}		
	}
	

	return NewLeader;
}

void ARTSEntities_Group::SetInitialFormation()
{
	if(const URTSEntities_GroupDataAsset* GroupData = GetData())
	{
		SetAllEntitiesFormation(GroupData->DefaultFormation);
	}
}

void ARTSEntities_Group::SetAllEntitiesFormation(const FPrimaryAssetId& NewFormation)
{
	if(HasAuthority())
	{
		for (int i = 0; i < Entities.Num(); ++i)
		{
			if(URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(Entities[i]))
			{
				Entity->SetFormation(NewFormation);			
			}
		}
	}
}

void ARTSEntities_Group::SetEntitiesFormation(const TArray<AActor*>& InEntities, const FPrimaryAssetId& NewFormation) const
{
	if(HasAuthority())
	{
		for (int i = 0; i < InEntities.Num(); ++i)
		{
			if(URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(InEntities[i]))
			{
				Entity->SetFormation(NewFormation);			
			}
		}
	}
}

TArray<FPrimaryAssetId> ARTSEntities_Group::GetFormationData() const
{
	if(URTSEntities_GroupDataAsset* GroupData = GetData())
	{
		return GroupData->Formations;
	}

	return TArray<FPrimaryAssetId>();
}

float ARTSEntities_Group::GetDefaultEntitySpacing() const
{
	if(const URTSEntities_GroupDataAsset* GroupData = GetData())
	{
		return GroupData->DefaultEntitySpacing;
	}

	return 250.f;
}

float ARTSEntities_Group::GetDefaultGroupSpacing() const
{
	if(const URTSEntities_GroupDataAsset* GroupData = GetData())
	{
		return GroupData->DefaultGroupSpacing;
	}

	return 100.f;
}

void ARTSEntities_Group::ExecuteNavigationCommand(URTSEntities_Command* Command)
{
	if(!HasAuthority())
	{
		return;
	}
	
	if(Command != nullptr)
	{		
		// Add command to active commands list
		Commands.Add(Command);

		// Assign the command to selection entities
		AssignCommandToGroupEntities(Command);
	}
}

bool ARTSEntities_Group::HasActiveCommandFor(const TArray<AActor*>& CommandMembers) const
{
	// Check for active commands dependent on command members
	for (int i = 0; i < CommandMembers.Num(); ++i)
	{		
		if(const APawn* Pawn = Cast<APawn>(CommandMembers[i]))
		{
			if(const ARTSEntities_AiControllerCommand* AiController = Cast<ARTSEntities_AiControllerCommand>(Pawn->GetController()))
			{
				if(AiController->HasActiveCommand())
				{
					return true;
				}
			}
		}		
	}			

	return false;
}

bool ARTSEntities_Group::AssignedCommand(const FGuid Id)
{
	for (int i = 0; i < Commands.Num(); ++i)
	{
		if(Commands[i]->GetId() == Id)
		{
			return true;
		}
	}

	return false;
}

bool ARTSEntities_Group::HasCompletedCommand(const FGuid Id)
{
	for (int i = 0; i < CommandHistory.Num(); ++i)
	{
		if(CommandHistory[i]->GetId() == Id)
		{
			return true;
		}
	}

	return false;
}

void ARTSEntities_Group::OnEntityCommandComplete(const FGuid Id, const ERTSEntities_CommandStatus EntityStatus, const FVector& Location)
{
	// Check command status
	if(EntityStatus == ERTSEntities_CommandStatus::Completed || EntityStatus == ERTSEntities_CommandStatus::Aborted)
	{
		// Remove callback for this entity
		//AiController->OnEntityCommandComplete.RemoveAll(this);
	
		const ERTSEntities_CommandStatus GroupStatus = CheckCommandStatus(Id);
		if(GroupStatus == ERTSEntities_CommandStatus::Completed || GroupStatus == ERTSEntities_CommandStatus::Aborted)
		{			
			CommandComplete(Id, GroupStatus, Location);
		}
	}	
}

float ARTSEntities_Group::GetMaxFormUpDistance() const
{
	if(const URTSEntities_GroupDataAsset* GroupData = GetData())
	{
		return GroupData->MaxFormUpDistance;
	}

	return MAX_FLT;
}

float ARTSEntities_Group::GetRequiredSpacing() const
{
	float SingleSpace = 0.f;
	float TotalSpacing = 0.f;
	for (int i = 0; i < Entities.Num(); ++i)
	{
		if(const APawn* Pawn = Cast<APawn>(Entities[i]))
		{
			if(const URTSEntities_Entity* EntityComponent = URTSEntities_Entity::FindEntityComponent(Pawn))
			{
				TotalSpacing += EntityComponent->GetMinSpacing();
				if(SingleSpace <= 0.f)
				{
					SingleSpace = TotalSpacing;
				}
			}
		}
	}

	return TotalSpacing + SingleSpace;
}

void ARTSEntities_Group::AssignCommandToGroupEntities(URTSEntities_Command* Command) const
{
	if(!HasAuthority())
	{
		return;
	}
	
	const FRTSEntities_PlayerSelection& GroupSelection = GetGroupSelection(Command);	
	if(GroupSelection.IsValid())
	{			
		for (int j = 0; j < GroupSelection.Entities.Num(); ++j)
		{
			if(const APawn* Pawn = Cast<APawn>(GroupSelection.Entities[j]))
			{
				if(ARTSEntities_AiControllerCommand* AiController = Cast<ARTSEntities_AiControllerCommand>(Pawn->GetController()))
				{						
					// Assign entity
					AiController->ExecuteCommand(Command);
				}
			}
		}
	}	
}

FRTSEntities_PlayerSelection ARTSEntities_Group::GetGroupSelection(URTSEntities_Command* Command) const
{
	const TArray<FRTSEntities_PlayerSelection>& CommandSelections = Command->GetSelections();
	for (int i = 0; i < CommandSelections.Num(); ++i)
	{
		if(CommandSelections[i].Group == this)
		{
			return CommandSelections[i];
		}
	}

	return FRTSEntities_PlayerSelection();
}

void ARTSEntities_Group::RemoveCommand(const FGuid Id)
{
	// Find the completed command and remove from the groups command list
	for (int i = Commands.Num() - 1; i >= 0; --i)
	{
		if(Commands[i]->GetId() == Id)
		{
			CommandHistory.Add(Commands[i]);
			Commands.RemoveAt(i);
		}
	}

	// Check the size of the command history, trim oldest command if greater than command history size
	if(const URTSEntities_DeveloperSettings* EntitiesSettings = GetDefault<URTSEntities_DeveloperSettings>())
	{
		if(CommandHistory.Num() > EntitiesSettings->CommandHistorySize)
		{
			CommandHistory.RemoveAt(0);
		}
	}
}

URTSEntities_Command* ARTSEntities_Group::GetCommand(const FGuid Id)
{
	for (int i = 0; i < Commands.Num(); ++i)
	{
		if(Commands[i]->GetId() == Id)
		{
			return Commands[i];
		}
	}

	return nullptr;
}

void ARTSEntities_Group::CommandComplete(const FGuid Id, const ERTSEntities_CommandStatus Status, const FVector& Location)
{
	if(URTSEntities_Command* Command = GetCommand(Id))
	{
		// Notify command this group has completed the command
		Command->Complete(Id, this, Status, Location);
		
		// Remove completed command from group
		RemoveCommand(Id);	
	}
}

ERTSEntities_CommandStatus ARTSEntities_Group::CheckCommandStatus(const FGuid Id) const
{
	// Find command for status check
	for (int i = 0; i < Commands.Num(); ++i)
	{
		if(Commands[i]->GetId() == Id)
		{
			// Check if the command is active on any of the selection members
			TArray<AActor*> CommandEntities = Commands[i]->GetGroupsSelectedEntities(this);
			if(IsActiveCommand(Commands[i]->GetId(), CommandEntities))
			{
				return ERTSEntities_CommandStatus::Active;
			}
		}
	}

	return ERTSEntities_CommandStatus::Completed;
}

bool ARTSEntities_Group::IsActiveCommand(const FGuid Id, const TArray<AActor*>& CommandMembers) const
{
	// Check for active commands dependent on command members
	for (int i = 0; i < CommandMembers.Num(); ++i)
	{		
		if(const APawn* Pawn = Cast<APawn>(CommandMembers[i]))
		{
			if(const ARTSEntities_AiControllerCommand* AiController = Cast<ARTSEntities_AiControllerCommand>(Pawn->GetController()))
			{
				if(AiController->IsActiveCommand(Id))
				{
					return true;
				}
			}
		}		
	}			

	return false;
}

