// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Entities/Actors/RTSEntities_MarkerActor.h"
#include "Components/DecalComponent.h"
#include "Framework/Data/RTSEntities_StaticGameData.h"
#include "Framework/Entities/Components/RTSEntities_MarkerComponent.h"


ARTSEntities_MarkerActor::ARTSEntities_MarkerActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(false);

	Decal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
	RootComponent = Decal;
}

void ARTSEntities_MarkerActor::BeginPlay()
{
	Super::BeginPlay();

	if(Decal)
	{
		Decal->SetWorldLocation(GetActorLocation());
		Decal->SetRelativeRotation(FRotator(90.f, 0.f,0.f));
	}
}

void ARTSEntities_MarkerActor::InitDestinationDecal(const FVector& InDecalSize, UMaterialInstanceDynamic* InMaterial)
{
	if(Decal)
	{
		Decal->DecalSize = InDecalSize;
	
		if(InMaterial != nullptr)
		{
			DynamicMaterialInstance = InMaterial;	
			Decal->SetMaterial(0, DynamicMaterialInstance);
			Decal->SetVisibility(true);
			Decal->MarkRenderStateDirty();
		}
	}
}

void ARTSEntities_MarkerActor::UpdateDestinationDecal(const FRTSEntities_EntityPosition& EntityPosition, const ERTSEntities_CommandStatus Status) const
{
	if(DynamicMaterialInstance != nullptr && Decal != nullptr)
	{
		Decal->SetWorldLocation(EntityPosition.Destination);
		Decal->SetRelativeRotation(FRotator(90.f, EntityPosition.Rotation.Yaw, 0.f));
		SetDecalStatus(Status);
		Decal->SetVisibility(true);
		Decal->MarkRenderStateDirty();
	}
}

void ARTSEntities_MarkerActor::InitNiagaraSystem()
{
	MarkerComponent = NewObject<URTSEntities_MarkerComponent>(GetOwner(), TEXT("MarkerComponent"));
	if(MarkerComponent != nullptr)
	{		
		MarkerComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		MarkerComponent->RegisterComponent();		
		AddInstanceComponent(MarkerComponent);
		MarkerComponent->SetMarkerType(ERTSEntities_MarkerType::Destination);		
	}
}

void ARTSEntities_MarkerActor::Disable() const
{
	if(Decal)
	{
		Decal->SetVisibility(false);
		Decal->MarkRenderStateDirty();
	}

	if(MarkerComponent != nullptr)
	{
		MarkerComponent->Deactivate();
	}	
}

void ARTSEntities_MarkerActor::Enable() const
{
	if(Decal)
	{
		Decal->SetVisibility(true);
		Decal->MarkRenderStateDirty();
	}

	if(MarkerComponent != nullptr)
	{
		MarkerComponent->Activate(true);
	}
}

void ARTSEntities_MarkerActor::SetDecalStatus(const ERTSEntities_CommandStatus Status) const
{
	if(DynamicMaterialInstance != nullptr && Decal != nullptr)
	{
		switch (Status)
		{
			case ERTSEntities_CommandStatus::Preview:
			case ERTSEntities_CommandStatus::Queued:
				DynamicMaterialInstance->SetScalarParameterValue(RTS_MATERIAL_DECAL_DOTTEDLINE, 1);
				break;
			case ERTSEntities_CommandStatus::Active:
				DynamicMaterialInstance->SetScalarParameterValue(RTS_MATERIAL_DECAL_DOTTEDLINE, 0);
				break;
			case ERTSEntities_CommandStatus::Completed:
				DynamicMaterialInstance->SetScalarParameterValue(RTS_MATERIAL_DECAL_DOTTEDLINE, 0);
				break;
			default: DynamicMaterialInstance->SetScalarParameterValue(RTS_MATERIAL_DECAL_DOTTEDLINE, 1);
		}
	}
}

