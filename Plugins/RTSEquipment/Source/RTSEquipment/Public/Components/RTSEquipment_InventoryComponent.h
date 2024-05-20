// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/RTSEquipment_DataTypes.h"
#include "Framework/Interfaces/RTSCore_InventoryInterface.h"
#include "RTSEquipment_InventoryComponent.generated.h"


class URTSEquipment_ItemDataAsset;

DECLARE_MULTICAST_DELEGATE(FOnInventoryCreatedDelegate);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RTSEQUIPMENT_API URTSEquipment_InventoryComponent : public UActorComponent, public IRTSCore_InventoryInterface
{
	GENERATED_BODY()

public:
	URTSEquipment_InventoryComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void SetEquipmentData(APlayerController* NewOwningPlayer, const TArray<FPrimaryAssetId>& NewEquipmentData) override;
	void EquipItem(const int32 Slot);
	void UnEquipItem();

	void EquipWeapon(const ERTSEquipment_WeaponSlot WeaponSlot);
	int32 GetAmmunitionSlot(const FPrimaryAssetId& AmmoType);

	FOnInventoryCreatedDelegate OnInventoryCreated;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void AssignEntityEquipment(APawn* Pawn, AController* OldController, AController* NewController);
	
	APawn* GetPawn() const;
	void LoadEquipmentData();
	void OnEquipmentDataLoaded();
	void CreateInventorySlot(const FPrimaryAssetId& EquipmentDataId);
	void OnEquippedWeaponFired();
	void OnEquippedWeaponReload();

	void CreateUnEquippedActors(const int32 Slot);
	void CreateUnEquippedActor(FRTSEquipment_Slot& Slot, const FRTSEquipment_ItemActorData& SlotData) const;	
	void CreateEquippedActors(const int32 Slot);
	void CreateEquippedActor(FRTSEquipment_Slot& Slot, const FRTSEquipment_ItemActorData& SlotData) const;
	
	FRTSEquipment_SlotItem CreateEquipment(const FRTSEquipment_ItemActorData& SlotData) const;
	
	virtual void EquipmentUpdate();
	void UpdateTurretRotation(FRTSEquipment_Slot& EquipmentSlot);
	FVector GetTurretAimPoint(const FVector& TurretLocation, const AActor* Target) const;
	bool WeaponBodyCollisionCheck(const FVector& WeaponLocation, const FRotator& WeaponRotation, const bool bIgnoreSelf) const;
	void CalculateBallistics(FVector& SocketToTargetDir) const;

	UPROPERTY(Replicated)
	TArray<FRotator> TurretRotations;

	UPROPERTY(Replicated)
	TArray<FRotator> RawTurretRotations;

	UPROPERTY(Replicated)
	TArray<FRotator> WeaponRotations;

	UPROPERTY(Replicated)
	TArray<FRotator> RawWeaponRotations;

	UPROPERTY(Replicated)
	TArray<FRTSEquipment_Slot> Inventory;
	
	UPROPERTY(Replicated)
	APlayerController* OwningPlayer;
	
	UPROPERTY(Replicated)
	TArray<FPrimaryAssetId> EquipmentData;

	UPROPERTY(Replicated)
	int32 EquippedSlot;

private:
	UPROPERTY()
	FGuid CreateEntityEquipmentTaskId;

	/** URTSCore_InventoryInterface **/
public:
	virtual bool HasWeaponEquipped() const override;
	virtual void FireWeapon() override;
	virtual void ReloadWeapon() override;
	virtual float GetCurrentWeaponRange() override;
	virtual int32 GetCurrentWeaponMagazineAmmoCount() override;
	virtual float GetCurrentWeaponFireRate() override;
	virtual float GetCurrentWeaponReloadRate() override;
	virtual bool HasInventoryAmmoForCurrentWeapon() override;
	virtual float GetCurrentWeaponAccuracy() override;	
	virtual TArray<FRotator> GetTurretRotations() const override { return TurretRotations; }
	virtual TArray<FRotator> GetWeaponRotations() const override { return WeaponRotations; }

protected:
	virtual bool IsWeapon() const { return Inventory.IsValidIndex(EquippedSlot) && Inventory[EquippedSlot].Type == ERTSEquipment_Type::Weapon; }
};
