// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSEntities_AiControllerCommand.h"
#include "Framework/Interfaces/RTSCore_EntityEquipmentInterface.h"
#include "RTSEntities_AiControllerEquipment.generated.h"

UCLASS()
class RTSENTITIES_API ARTSEntities_AiControllerEquipment : public ARTSEntities_AiControllerCommand, public IRTSCore_EntityEquipmentInterface
{
	GENERATED_BODY()

public:
	ARTSEntities_AiControllerEquipment(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void InitAiForGameplay() override;
	
	/** Equipment System **/
public:		
	// IRTSCore_EntityEquipmentInterface
	virtual void AssignEquipment(UActorComponent* NewEquipment) override { Equipment = NewEquipment; }
	virtual UActorComponent* GetEquipment() const override { return Equipment; }
	virtual void OnEquipmentCreated() override;	
	virtual TArray<FRotator> GetTurretRotations() const override;
	virtual TArray<FRotator> GetWeaponRotations() const override;
	virtual void RequestReload() override;
	virtual void FireWeapon() override;
	virtual void ReloadWeapon() override;
	virtual int32 GetCurrentWeaponAmmo() const override;	
	virtual bool HasInventoryAmmoForCurrentWeapon() const override;	
	virtual float GetCurrentWeaponFireRate() const override;	
	virtual float GetCurrentWeaponReloadRate() const override;	
	virtual float GetCurrentWeaponAccuracy() const override;
	virtual bool CanEngageTarget() override;
	virtual bool CanUpdateTarget() override;
	virtual FVector GetTargetLastKnownLocation() const override;
	// End IRTSCore_EntityEquipmentInterface
	
	virtual void AssignTargetData(const FRTSEntities_TargetData& TargetData);

protected:
	void UpdateTargetData();
	void UpdateTargetLastKnownLocation();
	bool TargetInRange(const FVector& TargetLocation) const;
	
	UPROPERTY()
	UActorComponent* Equipment;
	
	UPROPERTY()
	FRTSEntities_TargetData CurrentTarget;
	
	UPROPERTY()
	float LastAssignedTargetTime;	
	
	/** End Equipment System **/
};