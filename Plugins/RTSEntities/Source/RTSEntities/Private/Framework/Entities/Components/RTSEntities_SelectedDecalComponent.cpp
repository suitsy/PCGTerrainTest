// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Entities/Components/RTSEntities_SelectedDecalComponent.h"
#include "Framework/Entities/Components/RTSEntities_Entity.h"


URTSEntities_SelectedDecalComponent::URTSEntities_SelectedDecalComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	EntityComponent = nullptr;
}

void URTSEntities_SelectedDecalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(IsVisible())
	{
		FVector DecalLocation = GetOwner()->GetActorLocation();
		if(EntityComponent)
		{
			DecalLocation.Z -= EntityComponent->GetHalfHeight();
			SetWorldLocation(DecalLocation);
		}
	}
}

void URTSEntities_SelectedDecalComponent::OnRegister()
{
	Super::OnRegister();

	if(!GetOwner())
	{
		return;
	}

	// Assign entity component
	EntityComponent = URTSEntities_Entity::FindEntityComponent(GetOwner());
	if(EntityComponent)
	{
		if(UMaterialInstance* SelectedMaterial = EntityComponent->GetSelectMaterial())
		{			
			EntityComponent->OnSelectedChange.AddUObject(this, &ThisClass::UpdateSelected);

			const FVector EntityExtent = EntityComponent->GetExtent();
			const float MaxRadius = FMath::Max(EntityExtent.X, EntityExtent.Y) * EntityComponent->GetSelectionDecalSize();
			DecalSize = FVector(EntityExtent.Z, MaxRadius, MaxRadius);
			SetRelativeRotation(FRotator(90.f, 0.f,0.f));

			FVector DecalLocation = GetOwner()->GetActorLocation();
			DecalLocation.Z = 0.f;
			SetRelativeLocation(DecalLocation);
			
			DecalMaterial = SelectedMaterial;
			MarkRenderStateDirty();

			SetVisibility(false);
		}
	}
}

void URTSEntities_SelectedDecalComponent::UpdateSelected(const bool bSelected)
{
	if(DecalMaterial == nullptr)
	{
		return;
	}
	
	SetVisibility(bSelected);
}

