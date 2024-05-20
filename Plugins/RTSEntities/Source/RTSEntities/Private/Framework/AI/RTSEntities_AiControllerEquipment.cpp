// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AI/RTSEntities_AiControllerEquipment.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Framework/Data/RTSEntities_AiDataAsset.h"
#include "Framework/Data/RTSEntities_StaticGameData.h"
#include "Framework/Interfaces/RTSCore_InventoryInterface.h"


ARTSEntities_AiControllerEquipment::ARTSEntities_AiControllerEquipment(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	Equipment = nullptr;
	LastAssignedTargetTime = 0.f;
}

void ARTSEntities_AiControllerEquipment::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARTSEntities_AiControllerEquipment::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARTSEntities_AiControllerEquipment::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ARTSEntities_AiControllerEquipment::InitAiForGameplay()
{
	if(GetEquipment())
	{
		OnEquipmentCreated();
	}
	
	Super::InitAiForGameplay();
}

int32 ARTSEntities_AiControllerEquipment::GetCurrentWeaponAmmo() const
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

bool ARTSEntities_AiControllerEquipment::HasInventoryAmmoForCurrentWeapon() const
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

float ARTSEntities_AiControllerEquipment::GetCurrentWeaponFireRate() const
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

float ARTSEntities_AiControllerEquipment::GetCurrentWeaponReloadRate() const
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

float ARTSEntities_AiControllerEquipment::GetCurrentWeaponAccuracy() const
{
	if(GetEquipment() != nullptr)
	{
		if(IRTSCore_InventoryInterface* InventoryInterface = Cast<IRTSCore_InventoryInterface>(GetEquipment()))
		{
			return InventoryInterface->GetCurrentWeaponAccuracy();
		}
	}

	return -1.f;
}

bool ARTSEntities_AiControllerEquipment::CanEngageTarget()
{
	// Update target data
	UpdateTargetData();

	/** Check
	 *   - Current target is valid
	 *   - LastKnown location is valid
	 *   - Has line of sight to lastknown location
	 *   - Is within current weapon range **/
	return CurrentTarget.IsValid() && CurrentTarget.LastSeenEnemyLocation != FVector::ZeroVector && HasLineOfSight(CurrentTarget.LastSeenEnemyLocation) && TargetInRange(CurrentTarget.LastSeenEnemyLocation);
}

bool ARTSEntities_AiControllerEquipment::CanUpdateTarget()
{
	if(const URTSEntities_AiDataAsset* AiData = GetAiData())
	{
		if(GetBlackboardComponent())
		{
			const bool bTargetTimeOut = GetWorld() && GetWorld()->GetTimeSeconds() < LastAssignedTargetTime + AiData->SightAge;
			return GetBlackboardComponent()->GetValueAsVector(RTS_BLACKBOARD_VALUE_LASTKNOWNLOC) == FVector::ZeroVector || bTargetTimeOut;
		}
	}	

	return false;
}

FVector ARTSEntities_AiControllerEquipment::GetTargetLastKnownLocation() const
{
	if(GetBlackboardComponent())
	{
		return GetBlackboardComponent()->GetValueAsVector(RTS_BLACKBOARD_VALUE_LASTKNOWNLOC);		
	}

	return FVector::ZeroVector;
}

void ARTSEntities_AiControllerEquipment::AssignTargetData(const FRTSEntities_TargetData& TargetData)
{
	if(GetWorld())
	{
		CurrentTarget = TargetData;
		LastAssignedTargetTime = GetWorld()->GetTimeSeconds();
	}
}

void ARTSEntities_AiControllerEquipment::UpdateTargetData()
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

void ARTSEntities_AiControllerEquipment::UpdateTargetLastKnownLocation()
{
	if(GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsVector(RTS_BLACKBOARD_VALUE_LASTKNOWNLOC, CurrentTarget.LastSeenEnemyLocation);				
	}
}

bool ARTSEntities_AiControllerEquipment::TargetInRange(const FVector& TargetLocation) const
{
	if(GetPawn() && GetEquipment() != nullptr)
	{
		if(IRTSCore_InventoryInterface* EquipmentInterface = Cast<IRTSCore_InventoryInterface>(GetEquipment()))
		{
			// Get range to target and check current weapon is within range
			const float Range = (TargetLocation - GetPawn()->GetActorLocation()).Length();
			return Range < EquipmentInterface->GetCurrentWeaponRange();			
		}		
	}

	return false;
}

void ARTSEntities_AiControllerEquipment::OnEquipmentCreated()
{
	if(IRTSCore_InventoryInterface* EquipmentInterface = Cast<IRTSCore_InventoryInterface>(GetEquipment()))
	{
		if(EquipmentInterface->HasWeaponEquipped() && EquipmentInterface->GetCurrentWeaponMagazineAmmoCount() == 0)
		{
			RequestReload();
		}
	}
}

TArray<FRotator> ARTSEntities_AiControllerEquipment::GetTurretRotations() const
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

TArray<FRotator> ARTSEntities_AiControllerEquipment::GetWeaponRotations() const
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

void ARTSEntities_AiControllerEquipment::RequestReload()
{
	if(GetEquipment() != nullptr)
	{
		if(IRTSCore_InventoryInterface* EquipmentInterface = Cast<IRTSCore_InventoryInterface>(GetEquipment()))
		{
			if(EquipmentInterface->HasInventoryAmmoForCurrentWeapon())
			{
				SetState(ERTSCore_StateCategory::Ammunition, static_cast<int32>(ERTSCore_AmmoState::Reloading));
			}
			else
			{
				// Implement switch weapon
			}
		}
	}
}

void ARTSEntities_AiControllerEquipment::FireWeapon()
{
	
}

void ARTSEntities_AiControllerEquipment::ReloadWeapon()
{
	
}
