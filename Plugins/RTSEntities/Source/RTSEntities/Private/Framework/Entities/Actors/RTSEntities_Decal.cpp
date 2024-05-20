// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Entities/Actors/RTSEntities_Decal.h"
#include "Components/DecalComponent.h"
#include "Framework/Data/RTSEntities_StaticGameData.h"


ARTSEntities_Decal::ARTSEntities_Decal()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(false);

	Decal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
	RootComponent = Decal;
}

void ARTSEntities_Decal::BeginPlay()
{
	Super::BeginPlay();

	if(Decal)
	{
		Decal->SetWorldLocation(GetActorLocation());
		Decal->SetRelativeRotation(FRotator(90.f, 0.f,0.f));
	}
}

void ARTSEntities_Decal::InitDestinationMarker(const FVector& InDecalSize, UMaterialInstanceDynamic* DecalMaterial)
{
	if(Decal)
	{
		Decal->DecalSize = InDecalSize;
	
		if(DecalMaterial != nullptr)
		{
			DynamicMaterialInstance = DecalMaterial;	
			Decal->SetMaterial(0, DynamicMaterialInstance);
			Decal->SetVisibility(true);
			Decal->MarkRenderStateDirty();
		}
	}
}

void ARTSEntities_Decal::UpdateDestinationMarker(const FVector& NewLocation, const FRotator& NewRotation, const ERTSEntities_CommandStatus Status) const
{
	if(DynamicMaterialInstance != nullptr && Decal != nullptr)
	{
		Decal->SetWorldLocation(NewLocation);
		Decal->SetRelativeRotation(FRotator(90.f, NewRotation.Yaw, 0.f));
		SetDecalStatus(Status);
		Decal->SetVisibility(true);
		Decal->MarkRenderStateDirty();
	}
}

void ARTSEntities_Decal::Disable() const
{
	if(Decal)
	{
		Decal->SetVisibility(false);
		Decal->MarkRenderStateDirty();
	}
}

void ARTSEntities_Decal::Enable() const
{
	if(Decal)
	{
		Decal->SetVisibility(true);
		Decal->MarkRenderStateDirty();
	}
}

void ARTSEntities_Decal::SetDecalStatus(const ERTSEntities_CommandStatus Status) const
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

