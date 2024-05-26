// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Entities/Components/RTSEntities_Entity.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/AssetManager.h"
#include "Framework/Entities/Group/RTSEntities_Group.h"
#include "Framework/AI/RTSEntities_AiControllerCommand.h"
#include "Framework/Data/RTSEntities_EntityDataAsset.h"
#include "Framework/Data/RTSEntities_FormationDataAsset.h"
#include "Framework/Data/RTSEntities_GroupDataAsset.h"
#include "Framework/Debug/RTSEntities_Debug.h"
#include "Framework/Entities/Actors/RTSEntities_MarkerActor.h"
#include "Framework/Entities/Components/RTSEntities_MarkerComponent.h"
#include "Framework/Entities/Components/RTSEntities_SelectedDecalComponent.h"
#include "Framework/Interfaces/RTSCore_TeamManager.h"
#include "Framework/Player/RTSEntities_PlayerController.h"
#include "Framework/Settings/RTSEntities_DeveloperSettings.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "NiagaraSystem.h"
#include "Framework/Data/RTSCore_SystemStatics.h"
#include "Framework/Interfaces/RTSCore_EntityInterface.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"


// Sets default values for this component's properties
URTSEntities_Entity::URTSEntities_Entity(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void URTSEntities_Entity::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_CONDITION(ThisClass, OwningPlayer, COND_OwnerOnly);
	DOREPLIFETIME(ThisClass, EntityDataAssetId);
	DOREPLIFETIME(ThisClass, Index);
	DOREPLIFETIME(ThisClass, SubGroupId);
	DOREPLIFETIME(ThisClass, Group);
	DOREPLIFETIME(ThisClass, Spacing);
	DOREPLIFETIME(ThisClass, EntitySpacing);
	DOREPLIFETIME(ThisClass, GroupSpacing);
	DOREPLIFETIME(ThisClass, MaxSpeed);
	DOREPLIFETIME(ThisClass, Formation);
	DOREPLIFETIME(ThisClass, NavigationDestination);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, TeamId, SharedParams);
}

void URTSEntities_Entity::BeginPlay()
{
	Super::BeginPlay();	
}

void URTSEntities_Entity::OnRep_EntityDataAssetId()
{	
	if(!HasAuthority())
	{
		return;
	}

	// Set any attributes from the entity data when replicated
	if(const URTSEntities_EntityDataAsset* EntityData = GetEntityData())
	{
		MaxSpeed = EntityData->DefaultMaxSpeed;
	}
}

void URTSEntities_Entity::Initialise(APlayerController* NewOwningPlayer, const FRTSEntities_InitialisationData& EntityInitData)
{	
	if(!HasAuthority())
	{
		return;
	}

	OwningPlayer = NewOwningPlayer;
	Index = EntityInitData.EntityConfigData.Index;
	Group = EntityInitData.Group;
	EntityDataAssetId = EntityInitData.EntityConfigData.EntityData;
	SubGroupId = EntityInitData.EntityConfigData.SubGroupId;
	Formation = EntityInitData.DefaultFormation;
	
	OnRep_EntityDataAssetId();	
}

void URTSEntities_Entity::InitDefaultSpacing()
{
	if(Group)
	{
		EntitySpacing = Group->GetDefaultEntitySpacing();
		GroupSpacing = Group->GetDefaultGroupSpacing();
	}
}

ARTSEntities_AiControllerCommand* URTSEntities_Entity::GetAiController() const
{
	if(GetOwner())
	{
		if(const APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			return Cast<ARTSEntities_AiControllerCommand>(Pawn->GetController());
		}
	}

	return nullptr;
}

float URTSEntities_Entity::GetHalfHeight() const
{
	switch (GetType())
	{
		case ERTSCore_EntityType::Character:
			{				
				if(const ACharacter* OwningCharacter = Cast<ACharacter>(GetActor()))
				{
					if(const UCapsuleComponent* CapsuleComponent = OwningCharacter->GetCapsuleComponent())
					{
						return CapsuleComponent->GetScaledCapsuleHalfHeight();
					}
				}
			}
			break;
		case ERTSCore_EntityType::Vehicle:
			{				
				if(const UBoxComponent* BoxComponent = GetActor()->FindComponentByClass<UBoxComponent>())
				{
					return BoxComponent->GetScaledBoxExtent().Z * 0.5f;
				}
			}
			break;
		default: ;
	}
	
	return 0.f;
}

URTSEntities_GroupDataAsset* URTSEntities_Entity::GetGroupData() const
{
	if(Group)
	{
		return Group->GetData();
	}

	return nullptr;
}

AActor* URTSEntities_Entity::GetGroupLead(const TArray<AActor*>& Members)
{
	AActor* NewLeader = nullptr;
	
	if(Group)
	{
		int32 LeaderIndex = 99;		
		for (int i = 0; i < Members.Num(); ++i)
		{
			if(const URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(Members[i]))
			{
				if(Entity->GetIndex() < LeaderIndex)
				{
					NewLeader = Members[i];
					LeaderIndex = Entity->GetIndex();
				}
			}		
		}
	}

	return NewLeader;
}

void URTSEntities_Entity::GetSubGroupMembers(TArray<AActor*>& ReturnMembers) const
{
	GetGroup()->GetEntities(ReturnMembers);
	for (int i = ReturnMembers.Num() - 1; i >= 0; --i)
	{
		if(const URTSEntities_Entity* OtherEntity = URTSEntities_Entity::FindEntityComponent(ReturnMembers[i]))
		{
			if(OtherEntity->GetSubGroupId() != SubGroupId)
			{
				ReturnMembers.RemoveAt(i);
			}
		}
	}
}

float URTSEntities_Entity::GetMaxWaypointLength() const
{
	if(const URTSEntities_GroupDataAsset* GroupData = GetGroupData())
	{
		return GroupData->MaxWaypointLength;
	}

	return 500.f;
}

uint8 URTSEntities_Entity::IsEntitySelected() const
{
	if(GetOwner())
	{
		if(ARTSEntities_PlayerController* PC = Cast<ARTSEntities_PlayerController>(OwningPlayer))
		{
			return PC->IsEntitySelected(GetOwner());
		}
	}
	
	return false;
}

void URTSEntities_Entity::Highlight(APlayerController* RequestingPlayer, const bool bHighlight)
{
	if(!GetOwner() || !RequestingPlayer || RequestingPlayer != OwningPlayer)
	{
		return;
	}
	
	TArray<UMeshComponent*> Components;
	GetOwner()->GetComponents<UMeshComponent>(Components);
	for(int32 i = 0; i < Components.Num(); i++)
	{
		if(bHighlight)
		{
			if(DynamicHighlightMaterial == nullptr)
			{
				UMaterialInterface* CurrentMaterial = GetHighlightMaterial();
	
				// If we already set a MID, then we need to create based on its parent.
				if (const UMaterialInstanceDynamic* CurrentMaterialMID = Cast<UMaterialInstanceDynamic>(CurrentMaterial))
				{
					CurrentMaterial = CurrentMaterialMID->Parent;
				}

				// Create the MID
				DynamicHighlightMaterial = UMaterialInstanceDynamic::Create(CurrentMaterial, Components[i]);
				SetMaterialTeam(DynamicHighlightMaterial);
			}

			if(DynamicHighlightMaterial != nullptr)
			{						
				Components[i]->SetOverlayMaterial(DynamicHighlightMaterial);
			}
		}
		else
		{
			Components[i]->SetOverlayMaterial(nullptr);
		}
	}
}

void URTSEntities_Entity::Select(APlayerController* RequestingPlayer, const bool bSelect)
{
	if(!RequestingPlayer || RequestingPlayer != OwningPlayer && !RequestingPlayer->IsLocalController())
	{
		return;
	}
	
	OnSelectedChange.Broadcast(bSelect);
	
	

	/*TArray<USkeletalMeshComponent*> Components;
	GetOwner()->GetComponents<USkeletalMeshComponent>(Components);
	for(int32 i = 0; i < Components.Num(); i++)
	{
		if(bSelect)
		{
			if(UMaterialInstance* SelectMaterial = GetSelectMaterial())
			{
				Components[i]->SetOverlayMaterial(SelectMaterial);
			}
		}
		else
		{
			Components[i]->SetOverlayMaterial(nullptr);
		}
	}*/
}

FRTSEntities_PlayerSelection URTSEntities_Entity::CreatePlayerSelection(const TArray<AActor*>& SelectionMembers)
{
	if(Group)
	{		
		return FRTSEntities_PlayerSelection(
			GetGroupLead(SelectionMembers),
			Group,
			SelectionMembers
		);
	}

	return FRTSEntities_PlayerSelection();
}

UMaterialInstance* URTSEntities_Entity::GetHighlightMaterial() const
{
	if(const URTSEntities_EntityDataAsset* EntityData = GetEntityData())
	{
		return EntityData->HighlightMaterial.LoadSynchronous();		
	}

	return nullptr;
}

UMaterialInstance* URTSEntities_Entity::GetSelectMaterial() const
{
	if(const URTSEntities_EntityDataAsset* EntityData = GetEntityData())
	{
		return EntityData->SelectedMarkerMaterial.LoadSynchronous();		
	}

	return nullptr;
}

UMaterialInstanceDynamic* URTSEntities_Entity::GetCommandDestinationMaterial()
{
	if(DynamicDestinationMaterial == nullptr)
	{
		UMaterialInterface* MaterialInterface = nullptr;
		if(const URTSEntities_EntityDataAsset* EntityData = GetEntityData())
		{
			MaterialInterface = EntityData->CommandDestinationMaterial.LoadSynchronous();		
		}

		if(MaterialInterface != nullptr)
		{
			// If we already set a MID, then we need to create based on its parent.
			if (const UMaterialInstanceDynamic* CurrentMaterialMID = Cast<UMaterialInstanceDynamic>(MaterialInterface))
			{
				MaterialInterface = CurrentMaterialMID->Parent;
			}
		}

		// Create the MID
		DynamicDestinationMaterial = UMaterialInstanceDynamic::Create(MaterialInterface, GetOwner());
		SetMaterialTeam(DynamicDestinationMaterial);
	}	

	return DynamicDestinationMaterial;
}

ERTSEntities_SelectionMarkerType URTSEntities_Entity::GetMarkerType()
{
	if(const URTSEntities_EntityDataAsset* Data = GetEntityData())
	{
		return Data->SelectionMarkerType;
	}

	return ERTSEntities_SelectionMarkerType::NoSelectionMarker;
}

void URTSEntities_Entity::CreateMarkerComponents()
{
	Client_CreateDecalComponent();
}

float URTSEntities_Entity::GetSelectionMarkerSize() const
{
	if(const URTSEntities_EntityDataAsset* Data = GetEntityData())
	{
		return Data->SelectionMarkerRadius;
	}

	return 150.f;
}

UMaterialInstance* URTSEntities_Entity::GetSelectionMaterial() const
{
	if(const URTSEntities_EntityDataAsset* Data = GetEntityData())
	{
		return Data->SelectedMarkerMaterial.LoadSynchronous();
	}

	return nullptr;
}

UMaterialInstance* URTSEntities_Entity::GetDestinationMaterial() const
{
	if(const URTSEntities_EntityDataAsset* Data = GetEntityData())
	{
		return Data->DestinationMarkerMaterial.LoadSynchronous();
	}

	return nullptr;
}

UNiagaraSystem* URTSEntities_Entity::GetSelectionNiagaraSystem() const
{
	if(const URTSEntities_EntityDataAsset* Data = GetEntityData())
	{
		return Data->NiagaraSystemSelection.LoadSynchronous();
	}

	return nullptr;
}

UNiagaraSystem* URTSEntities_Entity::GetDestinationNiagaraSystem() const
{
	if(const URTSEntities_EntityDataAsset* Data = GetEntityData())
	{
		return Data->NiagaraSystemDestination.LoadSynchronous();
	}

	return nullptr;
}

void URTSEntities_Entity::HandleDestinationMarker(uint8 Display, const FRTSEntities_EntityPosition& EntityPosition, const ERTSEntities_CommandStatus& Status)
{
	Client_HandleDestinationMarker(Display, EntityPosition, Status);
}

void URTSEntities_Entity::Client_HandleDestinationMarker_Implementation(uint8 Display, const FRTSEntities_EntityPosition& EntityPosition, const ERTSEntities_CommandStatus& Status)
{
	
	/*switch (GetMarkerType())
	{
		case ERTSEntities_SelectionMarkerType::Decal:
			if(DestinationMarkerDecal == nullptr)
			{
				DestinationMarkerDecal = CreateMarkerActor();
			}

			if(DestinationMarkerDecal != nullptr)
			{
				Display ? UpdateDestinationMarker(EntityPosition, Status) : DestinationMarkerDecal->Disable();
			}			
			break;
		case ERTSEntities_SelectionMarkerType::NiagaraSystem:
			CreateDestinationEffect(Display, EntityPosition, Status);
			break;
		default: ;
	}*/
	
}

ARTSEntities_MarkerActor* URTSEntities_Entity::CreateMarkerActor()
{
	FActorSpawnParameters SpawnParameters = FActorSpawnParameters();
	SpawnParameters.Owner = GetOwner();
	
	ARTSEntities_MarkerActor* MarkerActor = GetWorld()->SpawnActor<ARTSEntities_MarkerActor>(ARTSEntities_MarkerActor::StaticClass(), FTransform::Identity, SpawnParameters);
	if(MarkerActor != nullptr)
	{
		if(UMaterialInstanceDynamic* MaterialInstance = GetCommandDestinationMaterial())
		{
			const float Radius = GetSelectionMarkerSize();
			const FVector DecalSize = FVector(Radius, Radius, Radius);
			MarkerActor->InitDestinationDecal(DecalSize, MaterialInstance);
		}		
	}

	return MarkerActor;
}

void URTSEntities_Entity::UpdateDestinationMarker(const FRTSEntities_EntityPosition& EntityPosition, const ERTSEntities_CommandStatus Status)
{
	if(DestinationMarkerDecal != nullptr)
	{
		switch (GetMarkerType())
		{
			case ERTSEntities_SelectionMarkerType::Decal:
				DestinationMarkerDecal->UpdateDestinationDecal(EntityPosition, Status);
				break;
			default: ;
		}
	}
}

void URTSEntities_Entity::CreateDestinationEffect(const uint8 Display, const FRTSEntities_EntityPosition& EntityPosition, const ERTSEntities_CommandStatus& Status)
{
	if(Display)
	{
		if(DestinationMarkerEffect == nullptr)
		{
			const TSoftObjectPtr<UNiagaraSystem> SystemTemplate = GetDestinationNiagaraSystem();
			if(SystemTemplate.IsValid())
			{
				if (UNiagaraSystem* NiagaraSystem = SystemTemplate.LoadSynchronous())
				{
					DestinationMarkerEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraSystem, EntityPosition.Destination, EntityPosition.Rotation);
					if(DestinationMarkerEffect != nullptr)
					{
						DestinationMarkerEffect->SetVariableMaterial(FName(TEXT("Material")), GetDestinationMaterial());
						DestinationMarkerEffect->SetFloatParameter(FName(TEXT("Size")), GetSelectionMarkerSize());
						DestinationMarkerEffect->SetFloatParameter(FName(TEXT("Height")), 5.f);
					}
				}
			}
		}
		
		if(DestinationMarkerEffect != nullptr)
		{
			FVector MarkerLocation = EntityPosition.Destination;
			FVector MarkerNormal = FVector::ZeroVector;
			URTSCore_SystemStatics::GetTerrainLocationAndNormal(GetWorld(), MarkerLocation, MarkerNormal);
			DestinationMarkerEffect->SetVectorParameter(FName(TEXT("Position")), MarkerLocation);
			DestinationMarkerEffect->SetVectorParameter(FName(TEXT("Facing")), MarkerNormal);

			return;
		}		
	}

	if(DestinationMarkerEffect != nullptr)
	{
		DestinationMarkerEffect->Deactivate();
		DestinationMarkerEffect->UnregisterComponent();
		DestinationMarkerEffect->DestroyInstance();
		DestinationMarkerEffect->DestroyComponent();
		DestinationMarkerEffect = nullptr;
	}	
}

void URTSEntities_Entity::Client_CreateDecalComponent_Implementation()
{
	if(GetOwner())
	{
		// Get all marker types
		TArray<ERTSEntities_MarkerType> MarkerTypes = GetEnumValues<ERTSEntities_MarkerType>();
		
		for (int i = 0; i < MarkerTypes.Num(); ++i)
		{
			// Ensure we dont add a component for the no marker type
			if(!MarkerTypes.IsValidIndex(i) || MarkerTypes[i] == ERTSEntities_MarkerType::NoMarker)
			{
				continue;
			}

			// Create the marker component for this type with unique name
			FString EnumName = StaticEnum<ERTSEntities_MarkerType>()->GetNameStringByValue((int64)MarkerTypes[i]);

			// Create a unique component name by appending the enum name
			FString ComponentName = FString::Printf(TEXT("MarkerComponent_%s"), *EnumName);
			
			if(URTSEntities_MarkerComponent* MarkerComponent = NewObject<URTSEntities_MarkerComponent>(GetOwner(), *ComponentName))
			{
				// Attach and register
				MarkerComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
				MarkerComponent->RegisterComponent();		
				GetOwner()->AddInstanceComponent(MarkerComponent);

				// Assign marker type
				MarkerComponent->SetMarkerType(MarkerTypes[i]);
			}
		}
		
		/*switch (SelectionMarkerType)
		{
			case ERTSEntities_SelectionMarkerType::Decal:
				if(URTSEntities_SelectedDecalComponent* DecalComponent = NewObject<URTSEntities_SelectedDecalComponent>(GetOwner(), TEXT("SelectedDecalComponent")))
				{
					DecalComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
					DecalComponent->RegisterComponent();
					GetOwner()->AddInstanceComponent(DecalComponent);
				}
				break;
			case ERTSEntities_SelectionMarkerType::NiagaraSystem:
						
				break;
			default: ;
		}*/
		
		
	}
}

URTSEntities_EntityDataAsset* URTSEntities_Entity::GetEntityData() const
{
	if (const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		if(EntityDataAssetId.IsValid())
		{
			return Cast<URTSEntities_EntityDataAsset>(AssetManager->GetPrimaryAssetObject(EntityDataAssetId));			
		}
	}

	return nullptr;
}

ERTSCore_EntityType URTSEntities_Entity::GetType() const
{
	if(const URTSEntities_EntityDataAsset* EntityData = GetEntityData())
	{
		return EntityData->Type;
	}

	return ERTSCore_EntityType::NonEntity;
}

URTSEntities_GroupDataAsset* URTSEntities_Entity::GetEntityGroupData() const
{
	if(Group != nullptr)
	{
		return Group->GetData();						
	}
	
	return nullptr;
}

TSubclassOf<APawn> URTSEntities_Entity::GetEntityClass() const
{
	if(const URTSEntities_DeveloperSettings* EntitiesSettings = GetDefault<URTSEntities_DeveloperSettings>())
	{
		if(const URTSEntities_EntityDataAsset* EntityData = GetEntityData())
		{
			if(const TSoftClassPtr<APawn>* EntityClassPtr = EntitiesSettings->EntitiesMap.Find(EntityData->EntityClass))
			{
				return EntityClassPtr->LoadSynchronous();
			}
		}
	}

	return nullptr;
}

void URTSEntities_Entity::SetGenericTeamId(const FGenericTeamId& NewTeamId)
{
	if (HasAuthority())
	{
		const FGenericTeamId OldTeamId = TeamId;		
		TeamId = NewTeamId;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, TeamId, this);
		OnRep_TeamId(OldTeamId);
		ConditionalBroadcastTeamChanged(this, OldTeamId, NewTeamId);
	}
	else
	{
		UE_LOG(LogRTSEntities, Error, TEXT("Cannot set team for %s on non-authority"), *GetPathName(this));
	}
}

void URTSEntities_Entity::OnRep_TeamId(FGenericTeamId OldTeamId)
{
	ConditionalBroadcastTeamChanged(this, OldTeamId, TeamId);
	
	SetDecalTeam();
	DynamicHighlightMaterial = nullptr;	
}

void URTSEntities_Entity::SetDecalTeam()
{
	if(URTSEntities_SelectedDecalComponent* DecalComponent = URTSEntities_SelectedDecalComponent::FindSelectedDecalComponent(GetOwner()))
	{
		if(HasTeamsManagerInterface())
		{			
			TeamsManagerInterface.GetInterface()->ApplyTeamDataToDecalComponent(GetGenericTeamId().GetId(), DecalComponent);			
		}			
	}
}

void URTSEntities_Entity::SetMaterialTeam(UMaterialInstanceDynamic* DynamicMaterial)
{
	if(HasTeamsManagerInterface() && DynamicMaterial != nullptr)
	{
		TeamsManagerInterface.GetInterface()->ApplyTeamDataToMaterial(GetGenericTeamId().GetId(), DynamicMaterial);
	}
}

bool URTSEntities_Entity::HasTeamsManagerInterface()
{
	if(TeamsManagerInterface.GetInterface() == nullptr)
	{
		if(const UWorld* WorldContext = GetWorld())
		{
			if(const AGameStateBase* GameState = WorldContext->GetGameState<AGameStateBase>())	
			{
				TArray<UActorComponent*> Components = GameState->GetComponentsByInterface(URTSCore_TeamManagerInterface::StaticClass());
				if(Components.IsValidIndex(0))
				{
					TeamsManagerInterface.SetObject(Components[0]); // Set the object implementing the interface
					TeamsManagerInterface.SetInterface(Cast<IRTSCore_TeamManagerInterface>(Components[0]));
				}
			}
		}
	}

	return TeamsManagerInterface.GetInterface() != nullptr;
}

bool URTSEntities_Entity::IsNavigating() const
{
	if(const ARTSEntities_AiControllerCommand* AiController = GetAiController())
	{
		return AiController->IsNavigating();
	}

	return false;
}

float URTSEntities_Entity::GetMinSpacing() const
{
	if(const URTSEntities_EntityDataAsset* EntityData = GetEntityData())
	{
		return EntityData->MinimumSpacing;
	}

	return 150.f;
}

float URTSEntities_Entity::GetMaxSpeed() const
{
	switch (GetType())
	{
		case ERTSCore_EntityType::Character:
		{
			if(const ACharacter* OwningCharacter = Cast<ACharacter>(GetActor()))
			{
				if(const UCharacterMovementComponent* MovementComponent = OwningCharacter->GetCharacterMovement())
				{
					return MovementComponent->MaxWalkSpeed;
				}
			}
		}
		break;
		case ERTSCore_EntityType::Vehicle:
		{
			/*if(const URTSEntities_EntityDataAsset* EntityData = GetEntityData())
			{
				
			}*/
		}
		break;
		default: ;
	}
	
	return 0;
}

float URTSEntities_Entity::GetDefaultMaxSpeed() const
{
	if(const URTSEntities_EntityDataAsset* EntityData = GetEntityData())
	{
		return EntityData->DefaultMaxSpeed;
	}

	return 400.f;
}

FVector URTSEntities_Entity::GetExtent() const
{
	FCollisionShape CollisionShape = FCollisionShape();
	if(GetOwner())
	{
		switch (GetType())
		{
			case ERTSCore_EntityType::Character:
			{
				if(const UCapsuleComponent* CapsuleComponent = Cast<ACharacter>(GetOwner())->GetCapsuleComponent())
				{
					float Radius, Height;
					CapsuleComponent->GetScaledCapsuleSize( Radius, Height);
					CollisionShape = FCollisionShape::MakeCapsule(Radius, Height);					
				}
				else
				{
					CollisionShape = FCollisionShape::MakeCapsule(35.f, 90.f);
				}
			}
			break;
			case ERTSCore_EntityType::Vehicle:
			{
				if(const UBoxComponent* BoxComponent = GetOwner()->GetComponentByClass<UBoxComponent>())
				{
					CollisionShape = FCollisionShape::MakeBox(BoxComponent->GetScaledBoxExtent());					
				}
				else
				{
					CollisionShape = FCollisionShape::MakeBox(FVector(500.f,400.f,200.f));
				}
			}
			break;
			default:;
		}
	}

	return CollisionShape.GetExtent();
}

float URTSEntities_Entity::GetStandOffRadius()
{
	float StandoffRadius = 150.f;
	
	if(!GetActor())
	{
		return StandoffRadius;
	}
	
	switch (GetType())
	{
		case ERTSCore_EntityType::Character:
		{
			if(const UCapsuleComponent* CapsuleComponent = Cast<ACharacter>(GetOwner())->GetCapsuleComponent())
			{
				return CapsuleComponent->GetScaledCapsuleRadius() * 2.f;				
			}
		}
		break;
		case ERTSCore_EntityType::Vehicle:
		{
			if(const UBoxComponent* BoxComponent = GetOwner()->GetComponentByClass<UBoxComponent>())
			{
				return BoxComponent->GetScaledBoxExtent().X;					
			}
			else
			{
				StandoffRadius = 500.f;
			}
		}
		break;
		default:;
	}	

	return StandoffRadius;
}

bool URTSEntities_Entity::IsApproachingWaypoint(const float DistanceToWaypoint)
{
	return DistanceToWaypoint <= (GetAcceptanceRadius() * 2.f) + (FMath::Abs(GetActor()->GetVelocity().Size()) * 0.25f);
}

bool URTSEntities_Entity::HasReachedDestination(const float DistanceToWaypoint)
{
	return DistanceToWaypoint < GetAcceptanceRadius() && GetSpeed() < GetAcceptanceSpeed();
}

float URTSEntities_Entity::GetArrivalDistance() const
{
	if(const URTSEntities_EntityDataAsset* EntityData = GetEntityData())
	{
		return EntityData->ArriveDistance;
	}

	return 200.f;
}

float URTSEntities_Entity::GetAcceptanceRadius() const
{
	if(const URTSEntities_EntityDataAsset* EntityData = GetEntityData())
	{
		return EntityData->AcceptanceRadius;
	}

	return 50.f;
}

float URTSEntities_Entity::GetAcceptanceSpeed() const
{
	if(const URTSEntities_EntityDataAsset* EntityData = GetEntityData())
	{
		return EntityData->AcceptanceSpeed;
	}

	return 4.f;
}

float URTSEntities_Entity::GetMaxTurnRate() const
{
	if(const URTSEntities_EntityDataAsset* EntityData = GetEntityData())
	{
		return EntityData->MaxTurnRate;
	}

	return 10.f;
}

float URTSEntities_Entity::GetMaxDestinationBrake() const
{
	if(const URTSEntities_EntityDataAsset* EntityData = GetEntityData())
	{
		return EntityData->MaxDestinationBrake;
	}

	return 10.f;
}

float URTSEntities_Entity::GetSpeedChangeFactor(const float Value) const
{
	if(const URTSEntities_GroupDataAsset* GroupData = GetEntityGroupData())
	{
		return GroupData->MemberSpeedReductionCurve->GetFloatValue(FMath::Clamp(Value, 0.0f, 1.0f));
	}

	return 0.5f;
}

void URTSEntities_Entity::SetMaxSpeed(const float NewMaxSpeed)
{
	switch (GetType())
	{
		case ERTSCore_EntityType::Character:
		{
			if(const ACharacter* OwningCharacter = Cast<ACharacter>(GetActor()))
			{
				if(UCharacterMovementComponent* MovementComponent = OwningCharacter->GetCharacterMovement())
				{
					MovementComponent->MaxWalkSpeed = NewMaxSpeed;
				}
			}
		}
		case ERTSCore_EntityType::Vehicle:
		{
			
		}
		default: ;
	}
}

void URTSEntities_Entity::ResetMaxSpeed()
{
	switch (GetType())
	{
		case ERTSCore_EntityType::Character:
			{
				if(const ACharacter* OwningCharacter = Cast<ACharacter>(GetActor()))
				{
					if(UCharacterMovementComponent* MovementComponent = OwningCharacter->GetCharacterMovement())
					{
						MovementComponent->MaxWalkSpeed = GetDefaultMaxSpeed();
					}
				}
			}
		case ERTSCore_EntityType::Vehicle:
			{
				
			}
		default: ;
	}
}

void URTSEntities_Entity::RequestSpeedChange(const float SpeedChangeFactor)
{
	RequestedSpeedChangeFactor = SpeedChangeFactor;
}

void URTSEntities_Entity::HandleSpeedChange()
{
	SetMaxSpeed(GetDefaultMaxSpeed() * FMath::Min(RequestedSpeedChangeFactor, GetDefaultMaxSpeed()));
}

float URTSEntities_Entity::GetFormationThreshold() const
{
	if(Group != nullptr)
	{
		if(const URTSEntities_GroupDataAsset* GroupData = Group->GetData())
		{
			return GroupData->NavigationFormationThreshold;
		}
	}

	return 99999;
}

void URTSEntities_Entity::SetNavigationDestination(const FVector& Location)
{
	NavigationDestination = Location;
	if(HasAuthority())
	{
		OnRep_NavigationDestination();
	}
}

void URTSEntities_Entity::PreviewNavigation(const FRTSEntities_EntityPosition& EntityPosition, const uint8 ShowPreview)
{
	/** CLIENT ONLY FUNCTION **/
	
	Client_PreviewEntityPosition = ShowPreview ? EntityPosition : FRTSEntities_EntityPosition();

	if(URTSEntities_MarkerComponent* Marker = URTSEntities_MarkerComponent::FindMarkerComponent(GetOwner(), ERTSEntities_MarkerType::Preview))
	{
		Marker->UpdatePreview(ShowPreview);
	}	
}

void URTSEntities_Entity::SetSpeedState(const ERTSCore_SpeedState& NewSpeedState) const
{
	if(const URTSEntities_EntityDataAsset* EntityData = GetEntityData())
	{
		switch (GetType())
		{
		case ERTSCore_EntityType::Character:
			{
			
				if(const ACharacter* OwningCharacter = Cast<ACharacter>(GetActor()))
				{
					if(UCharacterMovementComponent* MovementComponent = OwningCharacter->GetCharacterMovement())
					{
						MovementComponent->MaxWalkSpeed = EntityData->GetStateSpeed(NewSpeedState);
					}
				}
			}
		case ERTSCore_EntityType::Vehicle:
			{
			
			}
		default: ;
		}
	}
}

void URTSEntities_Entity::SetStanceState(const ERTSCore_StanceState& NewStanceState) const
{
	switch (GetType())
	{
		case ERTSCore_EntityType::Character:
		{			
			if(IRTSCore_EntityInterface* EntityInterface = Cast<IRTSCore_EntityInterface>(GetActor()))
			{				
				switch (NewStanceState)
				{
					case ERTSCore_StanceState::Crouch:
						EntityInterface->Crouch();
						break;
					case ERTSCore_StanceState::Prone:
						EntityInterface->Crouch();
						break;
					default: EntityInterface->Stand();
				}				
			}
		}
		case ERTSCore_EntityType::Vehicle:
		{
			
		}
		default: ;
	}
}

void URTSEntities_Entity::OnRep_NavigationDestination()
{
	if(IsLocalController())
	{
		if(URTSEntities_MarkerComponent* Marker = URTSEntities_MarkerComponent::FindMarkerComponent(GetOwner(), ERTSEntities_MarkerType::Destination))
		{
			Marker->DestinationUpdate();
		}
	}
}

void URTSEntities_Entity::SetFormation(const FPrimaryAssetId NewFormation)
{
	if(HasAuthority())
	{
		Formation = NewFormation;
		OnRep_Formation();
	}
}

URTSEntities_FormationDataAsset* URTSEntities_Entity::GetFormationData() const
{
	if (const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		if(Formation.IsValid())
		{
			return Cast<URTSEntities_FormationDataAsset>(AssetManager->GetPrimaryAssetObject(Formation));	
		}
	}

	return nullptr;
}

float URTSEntities_Entity::GetFormationSpacingTolerance() const
{
	if(const URTSEntities_GroupDataAsset* GroupData = GetEntityGroupData())
	{
		return GroupData->SpacingTolerance;
	}

	return 0.5f;
}

FVector2D URTSEntities_Entity::GetFormationSubordinateRearAngle() const
{
	if(const URTSEntities_FormationDataAsset* FormationData = GetFormationData())
	{
		return FormationData->SubordinateRearAngle;
	}

	return FVector2D(-0.5f, -1.f);
}

EFormationType URTSEntities_Entity::GetFormationType() const
{
	if(const URTSEntities_FormationDataAsset* FormationData = GetFormationData())
	{
		return FormationData->Type;
	}

	return EFormationType::Wedge;
}

FVector URTSEntities_Entity::GetFormationOffset() const
{
	if(const URTSEntities_FormationDataAsset* FormationData = GetFormationData())
	{
		return FormationData->Offset;
	}

	return FVector(0.f, 1.f, 0.f);
}



void URTSEntities_Entity::OnBehaviourStateChange(const ERTSCore_BehaviourState& NewState)
{
	switch (NewState)
	{
	case ERTSCore_BehaviourState::Stealth:
			SetStanceState(ERTSCore_StanceState::Crouch);
			SetSpeedState(ERTSCore_SpeedState::Crouch);
			break;
		case ERTSCore_BehaviourState::Safe:	
			SetStanceState(ERTSCore_StanceState::Standing);
			SetSpeedState(ERTSCore_SpeedState::Run);	
			break;
		case ERTSCore_BehaviourState::Cautious:	
			SetStanceState(ERTSCore_StanceState::Standing);
			SetSpeedState(ERTSCore_SpeedState::Walk);	
			break;
		case ERTSCore_BehaviourState::Combat:		
			SetStanceState(ERTSCore_StanceState::Standing);
			SetSpeedState(ERTSCore_SpeedState::Sprint);
			break;
		default: ;
	}
}



bool URTSEntities_Entity::HasAuthority() const
{
	if(const AActor* OwningActor = GetOwner())
	{
		return OwningActor->HasAuthority();
	}
	
	return false;
}

bool URTSEntities_Entity::IsLocalController() const
{
	if(OwningPlayer != nullptr)
	{
		return OwningPlayer->IsLocalController();		
	}

	return false;
}

void URTSEntities_Entity::OnRep_Formation()
{
	OnFormationChange.Broadcast();	
}

