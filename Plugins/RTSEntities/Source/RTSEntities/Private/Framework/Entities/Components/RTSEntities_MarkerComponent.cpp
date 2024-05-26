// Fill out your copyright notice in the Description page of Project Settings.


// Sets default values for this component's properties

#include "Framework/Entities/Components/RTSEntities_MarkerComponent.h"
#include "Framework/Data/RTSCore_SystemStatics.h"
#include "Framework/Entities/Components/RTSEntities_Entity.h"

URTSEntities_MarkerComponent::URTSEntities_MarkerComponent(const FObjectInitializer& ObjectInitializer):
	Super(ObjectInitializer),
	Visibility(0), Type(ERTSEntities_MarkerType::NoMarker), EntityComponent(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = false;
}

void URTSEntities_MarkerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

URTSEntities_MarkerComponent* URTSEntities_MarkerComponent::FindMarkerComponent(const AActor* Entity, const ERTSEntities_MarkerType& Type)
{
	if(Entity != nullptr)
	{
		TArray<URTSEntities_MarkerComponent*> MarkerComponents;
		Entity->GetComponents<URTSEntities_MarkerComponent>(MarkerComponents);

		for (URTSEntities_MarkerComponent* MarkerComponent : MarkerComponents)
		{
			if (MarkerComponent != nullptr && MarkerComponent->GetMarkerType() == Type)
			{
				return MarkerComponent;
			}
		}
	}

	return nullptr;
}

void URTSEntities_MarkerComponent::SetMarkerType(const ERTSEntities_MarkerType NewType)
{
	Type = NewType;

	SetSelectedDelegate();
}

void URTSEntities_MarkerComponent::SetMarkerVisibility(const bool NewVisibility)
{
	// Ensure visibility is disabled for preview markers if preview is off
	Visibility = Type == ERTSEntities_MarkerType::Preview ? Preview : NewVisibility;
	SetVisibility(Visibility);
	
	if(Visibility)
	{
		StartPositionTimer();
		Activate(true);
	}
	else
	{
		StopPositionTimer();
		Deactivate();
	}
}

void URTSEntities_MarkerComponent::UpdatePreview(const uint8 PreviewState)
{
	const uint8 HDest = EntityComponent->HasPreviewData();
	const uint8 Sel = EntityComponent->IsEntitySelected();
	Preview = PreviewState;
	SetMarkerVisibility(Preview && HDest && Sel); 
}

void URTSEntities_MarkerComponent::DestinationUpdate()
{
	const uint8 HDest = EntityComponent->HasDestination();
	const uint8 Sel = EntityComponent->IsEntitySelected();
	SetMarkerVisibility(HDest && Sel);
}

void URTSEntities_MarkerComponent::OnRegister()
{
	Super::OnRegister();

	// Assign entity component
	EntityComponent = URTSEntities_Entity::FindEntityComponent(GetOwner());
	
	if(EntityComponent)
	{
		SetAsset(EntityComponent->GetSelectionNiagaraSystem());

		// Apply type parameters
		switch (Type)
		{
			case ERTSEntities_MarkerType::Selection:
				SetVariableMaterial(FName(TEXT("Material")), EntityComponent->GetSelectionMaterial());
				break;
			case ERTSEntities_MarkerType::Destination:				
				SetVariableMaterial(FName(TEXT("Material")), EntityComponent->GetDestinationMaterial());
				break;
			case ERTSEntities_MarkerType::Preview:
				SetVariableMaterial(FName(TEXT("Material")), EntityComponent->GetDestinationMaterial());
				break;
			default: ;
		}		

		SetFloatParameter(FName(TEXT("Size")), EntityComponent->GetSelectionMarkerSize());
		SetFloatParameter(FName(TEXT("Height")), 5.f);	
	}

	SetVisibility(false);
}

void URTSEntities_MarkerComponent::SetSelectedDelegate()
{
	if(EntityComponent != nullptr)
	{
		if(DelegateHandleSelected.IsValid())
		{
			EntityComponent->OnSelectedChange.Remove(DelegateHandleSelected);
		}
		DelegateHandleSelected = EntityComponent->OnSelectedChange.AddUObject(this, &ThisClass::SetMarkerVisibility);
	}
}

void URTSEntities_MarkerComponent::StartPositionTimer()
{
	if(!GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_Position))
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Position, this, &ThisClass::PositionTimer, 0.01f, true);
	}
}

void URTSEntities_MarkerComponent::StopPositionTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Position);
}

void URTSEntities_MarkerComponent::PositionTimer()
{
	if(EntityComponent != nullptr)
	{
		// Set marker location to entities location by default
		FVector MarkerLocation = GetOwner()->GetActorLocation();

		// Update location based on marker type
		switch (Type)
		{
			case ERTSEntities_MarkerType::Destination:
				if(EntityComponent->HasDestination() && EntityComponent->IsEntitySelected())
				{
					MarkerLocation = EntityComponent->GetDestination();
				}
				else
				{
					Deactivate();
					return;
				}				
				break;
			case ERTSEntities_MarkerType::Preview:
				if(EntityComponent->HasPreviewData() && EntityComponent->IsEntitySelected())
				{
					MarkerLocation = EntityComponent->GetPreviewLocation();
				}
				else
				{
					Deactivate();
					return;
				}					
				break;
			default: ;
		}
		
		FVector MarkerNormal = FVector::ZeroVector;
		URTSCore_SystemStatics::GetTerrainLocationAndNormal(GetWorld(), MarkerLocation, MarkerNormal);
		SetVectorParameter(FName(TEXT("Position")), MarkerLocation);
		SetVectorParameter(FName(TEXT("Facing")), MarkerNormal);
	}
}

